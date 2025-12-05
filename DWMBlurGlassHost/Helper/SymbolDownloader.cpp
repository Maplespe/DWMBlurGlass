/**
 * FileName: SymbolDownloader.cpp
 *
 * Copyright (C) 2024 Maplespe、ALTaleX531
 *
 * This file is part of MToolBox and DWMBlurGlass.
 * DWMBlurGlass is free software: you can redistribute it and/or modify it under the terms of the
 * GNU Lesser General Public License as published by the Free Software Foundation, either version 3
 * of the License, or any later version.
 *
 * DWMBlurGlass is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with Foobar.
 * If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/
#include "SymbolDownloader.h"
#include "Helper.h"
#include "../UIManager.h"
#include <winhttp.h>
#include <Shlwapi.h>
#include <shlobj.h>
#include <CommCtrl.h>
#include <iomanip>
#include <sstream>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comctl32.lib")

namespace MDWMBlurGlass
{
	SymbolDownloader::SymbolDownloader()
	{
		m_symbolsPath = Utils::GetCurrentDir() + L"\\data\\symbols";
	}

	SymbolDownloader::~SymbolDownloader()
	{
		CancelDownload();
	}

	bool SymbolDownloader::CheckSymbolsValid()
	{
		for (const auto& dllName : TargetDlls)
		{
			if (!CheckSingleSymbolValid(dllName))
				return false;
		}
		return true;
	}

	bool SymbolDownloader::CheckSingleSymbolValid(std::wstring_view dllName)
	{
		// 获取系统目录中的DLL路径
		wchar_t systemPath[MAX_PATH];
		GetSystemDirectoryW(systemPath, MAX_PATH);
		std::wstring dllPath = std::wstring(systemPath) + L"\\" + dllName.data();

		if (!PathFileExistsW(dllPath.c_str()))
			return false;

		// 获取PE调试信息
		PEDebugInfo debugInfo{};
		if (!GetPEDebugInfo(dllPath, debugInfo))
			return false;

		// 构建PDB路径
		std::wstring pdbName = dllName.data();
		pdbName = pdbName.substr(0, pdbName.rfind(L'.')) + L".pdb";
		
		// GUID转字符串 (大写，无连字符)
		std::wstringstream ss;
		ss << std::uppercase << std::hex << std::setfill(L'0')
			<< std::setw(8) << debugInfo.guid.Data1
			<< std::setw(4) << debugInfo.guid.Data2
			<< std::setw(4) << debugInfo.guid.Data3;
		for (int i = 0; i < 8; i++)
			ss << std::setw(2) << debugInfo.guid.Data4[i];
		ss << std::dec << debugInfo.age;

		std::wstring pdbPath = m_symbolsPath + L"\\" + pdbName + L"\\" + ss.str() + L"\\" + pdbName;

		return PathFileExistsW(pdbPath.c_str());
	}

	bool SymbolDownloader::GetPEDebugInfo(std::wstring_view filePath, PEDebugInfo& info)
	{
		HANDLE hFile = CreateFileW(filePath.data(), GENERIC_READ, FILE_SHARE_READ, 
			nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		auto cleanup = RAIIHelper::scope_exit([&] { CloseHandle(hFile); });

		// 读取DOS头
		IMAGE_DOS_HEADER dosHeader;
		DWORD bytesRead;
		if (!ReadFile(hFile, &dosHeader, sizeof(dosHeader), &bytesRead, nullptr))
			return false;

		if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) // MZ
			return false;

		// 读取PE签名
		SetFilePointer(hFile, dosHeader.e_lfanew, nullptr, FILE_BEGIN);
		DWORD peSignature;
		if (!ReadFile(hFile, &peSignature, sizeof(peSignature), &bytesRead, nullptr))
			return false;

		if (peSignature != IMAGE_NT_SIGNATURE) // PE\0\0
			return false;

		// 读取COFF头
		IMAGE_FILE_HEADER fileHeader;
		if (!ReadFile(hFile, &fileHeader, sizeof(fileHeader), &bytesRead, nullptr))
			return false;

		// 读取可选头魔数判断32/64位
		WORD magic;
		if (!ReadFile(hFile, &magic, sizeof(magic), &bytesRead, nullptr))
			return false;

		// 回退到可选头开始
		SetFilePointer(hFile, -(LONG)sizeof(magic), nullptr, FILE_CURRENT);

		// 获取调试目录RVA和大小
		DWORD debugDirRVA = 0, debugDirSize = 0;
		DWORD sectionTableOffset = dosHeader.e_lfanew + 4 + sizeof(IMAGE_FILE_HEADER);

		if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		{
			IMAGE_OPTIONAL_HEADER64 optHeader;
			if (!ReadFile(hFile, &optHeader, sizeof(optHeader), &bytesRead, nullptr))
				return false;

			if (optHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_DEBUG)
			{
				debugDirRVA = optHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
				debugDirSize = optHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
			}
			sectionTableOffset += sizeof(IMAGE_OPTIONAL_HEADER64);
		}
		else
		{
			IMAGE_OPTIONAL_HEADER32 optHeader;
			if (!ReadFile(hFile, &optHeader, sizeof(optHeader), &bytesRead, nullptr))
				return false;

			if (optHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_DEBUG)
			{
				debugDirRVA = optHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
				debugDirSize = optHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
			}
			sectionTableOffset += sizeof(IMAGE_OPTIONAL_HEADER32);
		}

		if (debugDirRVA == 0 || debugDirSize == 0)
			return false;

		// 读取节表，找到调试目录的文件偏移
		SetFilePointer(hFile, sectionTableOffset, nullptr, FILE_BEGIN);
		DWORD debugDirFileOffset = 0;

		for (int i = 0; i < fileHeader.NumberOfSections; i++)
		{
			IMAGE_SECTION_HEADER sectionHeader;
			if (!ReadFile(hFile, &sectionHeader, sizeof(sectionHeader), &bytesRead, nullptr))
				return false;

			if (debugDirRVA >= sectionHeader.VirtualAddress &&
				debugDirRVA < sectionHeader.VirtualAddress + sectionHeader.SizeOfRawData)
			{
				debugDirFileOffset = sectionHeader.PointerToRawData + 
					(debugDirRVA - sectionHeader.VirtualAddress);
				break;
			}
		}

		if (debugDirFileOffset == 0)
			return false;

		// 读取调试目录条目
		SetFilePointer(hFile, debugDirFileOffset, nullptr, FILE_BEGIN);
		int numEntries = debugDirSize / sizeof(IMAGE_DEBUG_DIRECTORY);

		for (int i = 0; i < numEntries; i++)
		{
			IMAGE_DEBUG_DIRECTORY debugDir;
			if (!ReadFile(hFile, &debugDir, sizeof(debugDir), &bytesRead, nullptr))
				return false;

			if (debugDir.Type == IMAGE_DEBUG_TYPE_CODEVIEW)
			{
				DWORD currentPos = SetFilePointer(hFile, 0, nullptr, FILE_CURRENT);
				SetFilePointer(hFile, debugDir.PointerToRawData, nullptr, FILE_BEGIN);

				// 读取CodeView签名
				DWORD signature;
				if (!ReadFile(hFile, &signature, sizeof(signature), &bytesRead, nullptr))
					return false;

				if (signature == 0x53445352) // RSDS
				{
					// 读取GUID
					if (!ReadFile(hFile, &info.guid, sizeof(GUID), &bytesRead, nullptr))
						return false;

					// 读取Age
					if (!ReadFile(hFile, &info.age, sizeof(DWORD), &bytesRead, nullptr))
						return false;

					return true;
				}

				SetFilePointer(hFile, currentPos, nullptr, FILE_BEGIN);
			}
		}

		return false;
	}

	bool SymbolDownloader::DownloadSymbols(ProgressCallback progressCallback)
	{
		m_cancelRequested = false;
		m_lastError.clear();

		// 创建符号目录
		SHCreateDirectoryExW(nullptr, m_symbolsPath.c_str(), nullptr);

		int totalFiles = std::size(TargetDlls);
		int currentFile = 0;

		for (const auto& dllName : TargetDlls)
		{
			if (m_cancelRequested)
			{
				m_lastError = L"Download cancelled";
				return false;
			}

			currentFile++;
			
			// 报告开始下载
			if (progressCallback)
			{
				SymbolDownloadProgress progress;
				progress.fileName = dllName;
				progress.currentFile = currentFile;
				progress.totalFiles = totalFiles;
				progress.bytesReceived = 0;
				progress.totalBytes = 0;
				progress.progressPercentage = 0;
				progressCallback(progress);
			}

			if (!DownloadSingleSymbol(dllName, progressCallback))
				return false;
		}

		return true;
	}

	bool SymbolDownloader::DownloadSingleSymbol(std::wstring_view dllName, ProgressCallback progressCallback)
	{
		// 获取系统目录中的DLL路径
		wchar_t systemPath[MAX_PATH];
		GetSystemDirectoryW(systemPath, MAX_PATH);
		std::wstring dllPath = std::wstring(systemPath) + L"\\" + dllName.data();

		if (!PathFileExistsW(dllPath.c_str()))
		{
			m_lastError = L"DLL not found: ";
			m_lastError += dllName.data();
			return false;
		}

		// 获取PE调试信息
		PEDebugInfo debugInfo{};
		if (!GetPEDebugInfo(dllPath, debugInfo))
		{
			m_lastError = L"Failed to get debug info from: ";
			m_lastError += dllName.data();
			return false;
		}

		// 构建下载URL和本地路径
		std::wstring pdbName = dllName.data();
		pdbName = pdbName.substr(0, pdbName.rfind(L'.')) + L".pdb";
		
		// GUID转字符串
		std::wstringstream ss;
		ss << std::uppercase << std::hex << std::setfill(L'0')
			<< std::setw(8) << debugInfo.guid.Data1
			<< std::setw(4) << debugInfo.guid.Data2
			<< std::setw(4) << debugInfo.guid.Data3;
		for (int i = 0; i < 8; i++)
			ss << std::setw(2) << debugInfo.guid.Data4[i];
		ss << std::dec << debugInfo.age;
		std::wstring guidAge = ss.str();

		std::wstring downloadUrl = std::wstring(SymbolServerUrl) + L"/" + pdbName + L"/" + guidAge + L"/" + pdbName;
		std::wstring localDir = m_symbolsPath + L"\\" + pdbName + L"\\" + guidAge;
		std::wstring localPath = localDir + L"\\" + pdbName;

		if (PathFileExistsW(localPath.c_str()))
			return true;

		SHCreateDirectoryExW(nullptr, localDir.c_str(), nullptr);

		// 计算当前文件索引
		int currentFile = 0;
		int totalFiles = std::size(TargetDlls);
		for (int i = 0; i < totalFiles; i++)
		{
			if (dllName == TargetDlls[i])
			{
				currentFile = i + 1;
				break;
			}
		}

		return HttpDownloadFile(downloadUrl, localPath, dllName, currentFile, totalFiles, progressCallback);
	}

	bool SymbolDownloader::HttpDownloadFile(std::wstring_view url, std::wstring_view localPath,
		std::wstring_view fileName, int currentFile, int totalFiles,
		const ProgressCallback& progressCallback)
	{
		HINTERNET hSession = nullptr, hConnect = nullptr, hRequest = nullptr;
		HANDLE hFile = INVALID_HANDLE_VALUE;

		auto cleanup = RAIIHelper::scope_exit([&] 
		{
			if (hRequest) WinHttpCloseHandle(hRequest);
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
			if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
		});

		// 解析URL
		URL_COMPONENTS urlComp = { sizeof(URL_COMPONENTS) };
		wchar_t hostName[256] = {}, urlPath[1024] = {};
		urlComp.lpszHostName = hostName;
		urlComp.dwHostNameLength = 256;
		urlComp.lpszUrlPath = urlPath;
		urlComp.dwUrlPathLength = 1024;

		if (!WinHttpCrackUrl(url.data(), 0, 0, &urlComp))
		{
			m_lastError = L"Failed to parse URL";
			return false;
		}

		// 创建会话
		hSession = WinHttpOpen(L"DWMBlurGlass/1.0",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS, 0);
		if (!hSession)
		{
			m_lastError = L"Failed to create HTTP session";
			return false;
		}

		// 设置超时
		DWORD timeout = 60000; // 60秒
		WinHttpSetOption(hSession, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
		WinHttpSetOption(hSession, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

		// 连接服务器
		hConnect = WinHttpConnect(hSession, hostName, urlComp.nPort, 0);
		if (!hConnect)
		{
			m_lastError = L"Failed to connect to server";
			return false;
		}

		// 创建请求
		DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
		hRequest = WinHttpOpenRequest(hConnect, L"GET", urlPath,
			nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
		if (!hRequest)
		{
			m_lastError = L"Failed to create request";
			return false;
		}

		// 发送请求
		if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
			WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
		{
			m_lastError = L"Failed to send request";
			return false;
		}

		// 接收响应
		if (!WinHttpReceiveResponse(hRequest, nullptr))
		{
			m_lastError = L"Failed to receive response";
			return false;
		}

		// 检查状态码
		DWORD statusCode = 0, statusCodeSize = sizeof(statusCode);
		WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			nullptr, &statusCode, &statusCodeSize, nullptr);

		if (statusCode != 200)
		{
			m_lastError = L"HTTP error: " + std::to_wstring(statusCode);
			return false;
		}

		// 获取内容长度
		ULONGLONG totalBytes = 0;
		wchar_t contentLength[32] = {};
		DWORD contentLengthSize = sizeof(contentLength);
		if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH,
			nullptr, contentLength, &contentLengthSize, nullptr))
		{
			totalBytes = _wtoll(contentLength);
		}

		// 创建本地文件
		hFile = CreateFileW(localPath.data(), GENERIC_WRITE, 0, nullptr,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			m_lastError = L"Failed to create local file";
			return false;
		}

		// 下载数据
		ULONGLONG bytesReceived = 0;
		BYTE buffer[8192];
		DWORD bytesRead, bytesWritten;

		while (true)
		{
			if (m_cancelRequested)
			{
				CloseHandle(hFile);
				hFile = INVALID_HANDLE_VALUE;
				DeleteFileW(localPath.data());
				m_lastError = L"Download cancelled";
				return false;
			}

			if (!WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead))
			{
				CloseHandle(hFile);
				hFile = INVALID_HANDLE_VALUE;
				DeleteFileW(localPath.data());
				m_lastError = L"Failed to read data";
				return false;
			}

			if (bytesRead == 0)
				break; // 下载完成

			if (!WriteFile(hFile, buffer, bytesRead, &bytesWritten, nullptr))
			{
				CloseHandle(hFile);
				hFile = INVALID_HANDLE_VALUE;
				DeleteFileW(localPath.data());
				m_lastError = L"Failed to write file";
				return false;
			}

			bytesReceived += bytesRead;

			// 报告进度
			if (progressCallback)
			{
				SymbolDownloadProgress progress;
				progress.fileName = fileName;
				progress.currentFile = currentFile;
				progress.totalFiles = totalFiles;
				progress.bytesReceived = bytesReceived;
				progress.totalBytes = totalBytes;
				progress.progressPercentage = (totalBytes > 0) ? 
					static_cast<int>(bytesReceived * 100 / totalBytes) : 0;
				progressCallback(progress);
			}
		}

		return true;
	}

	void SymbolDownloader::CancelDownload()
	{
		m_cancelRequested = true;
	}

	//========================================
	// 进度对话框实现
	//========================================

	// 对话框控件ID
	#define IDC_PROGRESS_BAR		1001
	#define IDC_STATUS_TEXT			1002
	#define IDC_CANCEL_BUTTON		1003
	#define IDC_INFO_TEXT			1004

	// 自定义消息
	#define WM_UPDATE_PROGRESS		(WM_USER + 100)
	#define WM_DOWNLOAD_COMPLETE	(WM_USER + 101)

	bool SymbolDownloadDialog::Show(HWND hParent)
	{
		// 初始化通用控件
		INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX), ICC_PROGRESS_CLASS };
		InitCommonControlsEx(&icc);

		DialogData data;
		data.downloader = new SymbolDownloader();

		// 创建对话框
		// 使用CreateDialogIndirect动态创建对话框
		struct DialogTemplate
		{
			DLGTEMPLATE dlg;
			WORD menu;
			WORD windowClass;
			WCHAR title[64];
		};

		DialogTemplate dlgTemplate = {};
		dlgTemplate.dlg.style = DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		dlgTemplate.dlg.dwExtendedStyle = 0;
		dlgTemplate.dlg.cdit = 0;
		dlgTemplate.dlg.x = 0;
		dlgTemplate.dlg.y = 0;
		dlgTemplate.dlg.cx = 280;
		dlgTemplate.dlg.cy = 100;
		dlgTemplate.menu = 0;
		dlgTemplate.windowClass = 0;
		
		// 获取本地化标题
		std::wstring title = GetLanguageString(L"symdowntitle");
		if (title.empty()) title = L"Download Symbol Files";
		wcsncpy_s(dlgTemplate.title, title.c_str(), 63);

		HWND hDlg = CreateDialogIndirectParamW(
			GetModuleHandle(nullptr),
			&dlgTemplate.dlg,
			hParent,
			DialogProc,
			reinterpret_cast<LPARAM>(&data)
		);

		if (!hDlg)
		{
			delete data.downloader;
			return false;
		}

		data.hDlg = hDlg;

		// 启动下载线程
		HANDLE hThread = CreateThread(nullptr, 0, DownloadThreadProc, &data, 0, nullptr);
		if (!hThread)
		{
			DestroyWindow(hDlg);
			delete data.downloader;
			return false;
		}

		// 显示对话框
		ShowWindow(hDlg, SW_SHOW);
		UpdateWindow(hDlg);

		// 消息循环
		MSG msg;
		while (GetMessageW(&msg, nullptr, 0, 0))
		{
			if (!IsWindow(hDlg))
				break;

			if (!IsDialogMessageW(hDlg, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}

		// 等待线程结束
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);

		bool result = data.success;
		delete data.downloader;

		return result;
	}

	INT_PTR CALLBACK SymbolDownloadDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		static DialogData* s_data = nullptr;
		static HFONT s_hFont = nullptr;
		static ITaskbarList3* s_pTaskbar = nullptr;

		switch (message)
		{
		case WM_INITDIALOG:
		{
			s_data = reinterpret_cast<DialogData*>(lParam);

			// 初始化COM和任务栏进度接口
			CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
			if (SUCCEEDED(CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, 
				IID_ITaskbarList3, reinterpret_cast<void**>(&s_pTaskbar))))
			{
				s_pTaskbar->HrInit();
				s_pTaskbar->SetProgressState(hDlg, TBPF_NORMAL);
			}

			// 获取DPI缩放比例
			int dpi = GetDpiForWindow(hDlg);
			float dpiScale = static_cast<float>(dpi) / 96.0f;
			
			// 基础尺寸（96 DPI下的像素值）
			int baseWidth = 480;
			int baseHeight = 180;
			int baseMargin = 20;
			int baseButtonWidth = 100;
			int baseButtonHeight = 30;
			int baseProgressHeight = 24;
			int baseTextHeight = 20;
			int baseSpacing = 12;

			// 应用DPI缩放
			int width = static_cast<int>(baseWidth * dpiScale);
			int height = static_cast<int>(baseHeight * dpiScale);
			int margin = static_cast<int>(baseMargin * dpiScale);
			int buttonWidth = static_cast<int>(baseButtonWidth * dpiScale);
			int buttonHeight = static_cast<int>(baseButtonHeight * dpiScale);
			int progressHeight = static_cast<int>(baseProgressHeight * dpiScale);
			int textHeight = static_cast<int>(baseTextHeight * dpiScale);
			int spacing = static_cast<int>(baseSpacing * dpiScale);

			// 设置对话框标题
			std::wstring title = L"DWMBlurGlass - ";
			std::wstring localTitle = GetLanguageString(L"symdowntitle");
			if (localTitle.empty()) localTitle = L"Download Symbol Files";
			title += localTitle;
			SetWindowTextW(hDlg, title.c_str());

			// 调整对话框大小（包括非客户区）
			RECT rcWindow = { 0, 0, width, height };
			AdjustWindowRectEx(&rcWindow, GetWindowLong(hDlg, GWL_STYLE), FALSE, GetWindowLong(hDlg, GWL_EXSTYLE));
			int windowWidth = rcWindow.right - rcWindow.left;
			int windowHeight = rcWindow.bottom - rcWindow.top;

			// 居中显示
			HWND hOwner = GetParent(hDlg);
			if (!hOwner) hOwner = GetDesktopWindow();
			RECT rcOwner;
			GetWindowRect(hOwner, &rcOwner);
			int x = rcOwner.left + (rcOwner.right - rcOwner.left - windowWidth) / 2;
			int y = rcOwner.top + (rcOwner.bottom - rcOwner.top - windowHeight) / 2;
			SetWindowPos(hDlg, nullptr, x, y, windowWidth, windowHeight, SWP_NOZORDER);

			// 获取客户区大小
			RECT rcClient;
			GetClientRect(hDlg, &rcClient);
			int clientWidth = rcClient.right - rcClient.left;
			int clientHeight = rcClient.bottom - rcClient.top;

			// 计算控件位置
			int currentY = margin;
			int contentWidth = clientWidth - margin * 2;
			NONCLIENTMETRICSW ncm = { sizeof(NONCLIENTMETRICSW) };
			SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
			// 根据DPI调整字体大小
			ncm.lfMessageFont.lfHeight = static_cast<LONG>(ncm.lfMessageFont.lfHeight * dpiScale / 1.0f);
			s_hFont = CreateFontIndirectW(&ncm.lfMessageFont);

			// 创建信息文本
			std::wstring infoText = GetLanguageString(L"symdowninfo");
			if (infoText.empty()) infoText = L"Downloading symbol files from Microsoft servers...";
			HWND hInfo = CreateWindowExW(0, L"STATIC", infoText.c_str(),
				WS_CHILD | WS_VISIBLE | SS_LEFT,
				margin, currentY, contentWidth, textHeight,
				hDlg, (HMENU)IDC_INFO_TEXT, GetModuleHandle(nullptr), nullptr);
			SendMessage(hInfo, WM_SETFONT, (WPARAM)s_hFont, TRUE);
			currentY += textHeight + spacing;

			// 创建进度条
			HWND hProgress = CreateWindowExW(0, PROGRESS_CLASSW, nullptr,
				WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
				margin, currentY, contentWidth, progressHeight,
				hDlg, (HMENU)IDC_PROGRESS_BAR, GetModuleHandle(nullptr), nullptr);
			SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
			currentY += progressHeight + spacing;

			// 创建状态文本
			HWND hStatus = CreateWindowExW(0, L"STATIC", L"",
				WS_CHILD | WS_VISIBLE | SS_LEFT | SS_ENDELLIPSIS,
				margin, currentY, contentWidth, textHeight,
				hDlg, (HMENU)IDC_STATUS_TEXT, GetModuleHandle(nullptr), nullptr);
			SendMessage(hStatus, WM_SETFONT, (WPARAM)s_hFont, TRUE);

			// 创建取消按钮（放在右下角）
			int buttonY = clientHeight - margin - buttonHeight;
			int buttonX = clientWidth - margin - buttonWidth;
			std::wstring cancelText = GetLanguageString(L"cancel");
			if (cancelText.empty()) cancelText = L"Cancel";
			HWND hCancel = CreateWindowExW(0, L"BUTTON", cancelText.c_str(),
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				buttonX, buttonY,
				buttonWidth, buttonHeight,
				hDlg, (HMENU)IDC_CANCEL_BUTTON, GetModuleHandle(nullptr), nullptr);
			SendMessage(hCancel, WM_SETFONT, (WPARAM)s_hFont, TRUE);

			return TRUE;
		}

		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_CANCEL_BUTTON || LOWORD(wParam) == IDCANCEL)
			{
				if (s_data && s_data->downloader)
				{
					s_data->cancelled = true;
					s_data->downloader->CancelDownload();
				}
				return TRUE;
			}
			break;

		case WM_UPDATE_PROGRESS:
		{
			auto* progress = reinterpret_cast<SymbolDownloadProgress*>(lParam);
			if (progress)
			{
				// 更新进度条
				HWND hProgress = GetDlgItem(hDlg, IDC_PROGRESS_BAR);
				SendMessage(hProgress, PBM_SETPOS, progress->progressPercentage, 0);

				// 更新任务栏进度
				if (s_pTaskbar)
				{
					s_pTaskbar->SetProgressValue(hDlg, progress->progressPercentage, 100);
				}

				// 更新状态文本
				std::wstring statusText;
				if (progress->totalBytes > 0)
				{
					std::wstring downloading = GetLanguageString(L"symdowning");
					if (downloading.empty()) downloading = L"Downloading...";
					
					statusText = downloading + L" " + progress->fileName + 
						L" (" + std::to_wstring(progress->currentFile) + L"/" + 
						std::to_wstring(progress->totalFiles) + L"): " +
						std::to_wstring(progress->bytesReceived / 1024) + L"KB / " +
						std::to_wstring(progress->totalBytes / 1024) + L"KB";
				}
				else
				{
					std::wstring downloading = GetLanguageString(L"symdowning");
					if (downloading.empty()) downloading = L"Downloading...";
					
					statusText = downloading + L" " + progress->fileName + 
						L" (" + std::to_wstring(progress->currentFile) + L"/" + 
						std::to_wstring(progress->totalFiles) + L")...";
				}
				SetDlgItemTextW(hDlg, IDC_STATUS_TEXT, statusText.c_str());

				delete progress;
			}
			return TRUE;
		}

		case WM_DOWNLOAD_COMPLETE:
		{
			if (s_data)
			{
				s_data->success = (wParam == TRUE);
			}
			// 设置任务栏进度为完成或错误状态
			if (s_pTaskbar)
			{
				s_pTaskbar->SetProgressState(hDlg, wParam ? TBPF_NOPROGRESS : TBPF_ERROR);
			}
			DestroyWindow(hDlg);
			return TRUE;
		}

		case WM_CLOSE:
			if (s_data && s_data->downloader)
			{
				s_data->cancelled = true;
				s_data->downloader->CancelDownload();
			}
			return TRUE;

		case WM_DESTROY:
			// 清理任务栏进度
			if (s_pTaskbar)
			{
				s_pTaskbar->SetProgressState(hDlg, TBPF_NOPROGRESS);
				s_pTaskbar->Release();
				s_pTaskbar = nullptr;
			}
			CoUninitialize();

			if (s_hFont)
			{
				DeleteObject(s_hFont);
				s_hFont = nullptr;
			}
			PostQuitMessage(0);
			return TRUE;
		}

		return FALSE;
	}

	DWORD WINAPI SymbolDownloadDialog::DownloadThreadProc(LPVOID lpParam)
	{
		auto* data = reinterpret_cast<DialogData*>(lpParam);
		if (!data || !data->downloader || !data->hDlg)
			return 1;

		bool success = data->downloader->DownloadSymbols([data](const SymbolDownloadProgress& progress)
		{
			if (IsWindow(data->hDlg))
			{
				// 复制进度信息到堆上
				auto* progressCopy = new SymbolDownloadProgress(progress);
				PostMessage(data->hDlg, WM_UPDATE_PROGRESS, 0, reinterpret_cast<LPARAM>(progressCopy));
			}
		});

		if (IsWindow(data->hDlg))
		{
			PostMessage(data->hDlg, WM_DOWNLOAD_COMPLETE, success ? TRUE : FALSE, 0);
		}

		return 0;
	}
}
