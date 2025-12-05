/**
 * FileName: MHostHelper.cpp
 *
 * Copyright (C) 2024 Maplespe
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
#include "MHostHelper.h"
#include "Helper/SymbolResolver.h"
#include "Helper/SymbolDownloader.h"
#include "Helper/Helper.h"
#include "UIManager.h"
#include "Common.h"
#include "../DWMBlurGlassExt/Common/DefFunctionList.h"
#include <TlHelp32.h>
#include "Helper/wil.h"

#pragma data_seg(".DWMBlurGlassShared")

DWORD64 g_hookFunOffsetList[MDWMBlurGlassExt::g_hookFunList.size()] = { 0 };

size_t g_dwmcoreFunCount = 0;
size_t g_udwmFunCount = 0;

HWND g_hostMsgWnd = nullptr;

#pragma data_seg()
#pragma comment(linker,"/SECTION:.DWMBlurGlassShared,RWS")

namespace MDWMBlurGlass
{
    SymbolResolver g_symResolver;

	std::vector<DWORD> g_proclist;
	UINT_PTR g_timerID = 0;

	extern void OnProcessStart(DWORD id);

	void RefreshProcessList()
	{
		PROCESSENTRY32W pe;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		pe.dwSize = sizeof(PROCESSENTRY32W);
		if (!Process32FirstW(hSnapshot, &pe))
			return;

		std::vector<DWORD> list;

		while (Process32NextW(hSnapshot, &pe) != FALSE)
		{
			std::wstring exe = pe.szExeFile;
			std::ranges::transform(exe, exe.begin(), ::tolower);

			if(exe == L"dwm.exe")
				list.push_back(pe.th32ProcessID);
		}
		CloseHandle(hSnapshot);

		for (const auto& id : list)
		{
			if (auto iter = std::ranges::find(g_proclist, id); iter != g_proclist.end())
				continue;

			OnProcessStart(id);
		}
		g_proclist = list;
	}

	bool IsEnableTransparency()
	{
		HKEY hKey = nullptr;
		DWORD dwValue = 1;
		if (RegOpenKeyExW(HKEY_CURRENT_USER, LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\themes\personalize)", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			DWORD dwType = REG_DWORD;
			DWORD dwDataSize = sizeof(DWORD);

			RegQueryValueExW(hKey, L"EnableTransparency", nullptr, &dwType, (LPBYTE)&dwValue, &dwDataSize);

			RegCloseKey(hKey);
		}
		return dwValue;
	}

	HWND FindMessageWnd(DWORD pid)
	{
		if (!pid)
			return nullptr;

		HWND hwnd = nullptr;
		do
		{
			hwnd = FindWindowExW(HWND_MESSAGE, hwnd, DWMBlurGlassNotifyClassName, nullptr);
			if (hwnd != nullptr)
			{
				DWORD ProcessId = NULL;
				GetWindowThreadProcessId(hwnd, &ProcessId);
				if (ProcessId == pid)
					return hwnd;
			}
		} while (hwnd != nullptr);
		return nullptr;
	}

	void ParsingSymbol(PSYMBOL_INFO symInfo, std::string& funName, std::string& fullName, DWORD64& offset)
	{
		std::string_view functionName{ symInfo->Name, symInfo->NameLen };

		fullName = functionName;

		CHAR fullyUnDecoratedFunctionName[MAX_PATH + 1]{};
		UnDecorateSymbolName(
			functionName.data(), fullyUnDecoratedFunctionName, MAX_PATH,
			UNDNAME_NAME_ONLY
		);

		funName = fullyUnDecoratedFunctionName;
		offset = symInfo->Address - symInfo->ModBase;
	}

	bool SymCallback_Dwmcore(PSYMBOL_INFO symInfo, ULONG symbolSize)
	{
		DWORD64 offset = 0;
		std::string funName, fullName;
		ParsingSymbol(symInfo, funName, fullName, offset);

		size_t funCount = 0;
		for (size_t i = 0; i < MDWMBlurGlassExt::g_hookFunList.size(); ++i)
		{
			auto& [type, _funName] = MDWMBlurGlassExt::g_hookFunList[i];
			if (type == dwmcore && (funName == _funName || fullName == _funName))
			{
				g_hookFunOffsetList[i] = offset;
				funCount++;
			}
		}

		return 1;
	}

	bool SymCallback_uDwm(PSYMBOL_INFO symInfo, ULONG symbolSize)
	{
		DWORD64 offset = 0;
		std::string funName, fullName;
		ParsingSymbol(symInfo, funName, fullName, offset);

		size_t funCount = 0;
		for (size_t i = 0; i < MDWMBlurGlassExt::g_hookFunList.size(); ++i)
		{
			auto& [type, _funName] = MDWMBlurGlassExt::g_hookFunList[i];
			if (type == udwm && (funName == _funName || fullName == _funName))
			{
				g_hookFunOffsetList[i] = offset;
				funCount++;
			}
		}

		return g_udwmFunCount != funCount;
	}

    bool MHostGetSymbolState()
    {
		std::wstring symPath = L"SRV*" + Utils::GetCurrentDir() + L"\\data\\symbols";
		auto callback = [](PSYMBOL_INFO, ULONG) { return false; };
        HRESULT hr = g_symResolver.Walk(L"dwmcore.dll", "*!*", symPath, callback);
		if (FAILED(hr))
			return false;
		hr = g_symResolver.Walk(L"uDwm.dll", "*!*", symPath, callback);
        return SUCCEEDED(hr);
    }

	void OnProcessStart(DWORD id)
	{
		//单独记录每个用户dwm进程的启动间隔状态
		using namespace std::chrono;
		struct sessionData
		{
			steady_clock::duration times = steady_clock::now().time_since_epoch();
			bool init = true;
		};
		static std::unordered_map<DWORD, sessionData> sessionList;

		DWORD sessionId = 0;
		ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);

		if(auto iter = sessionList.find(sessionId); iter != sessionList.end())
		{
			bool retry = false;
			//dwm进程不应启动这么频繁 这有可能是频繁崩溃 停止自动加载
			if (!iter->second.init && steady_clock::now().time_since_epoch() - iter->second.times < 10s)
			{
				KillTimer(g_hostMsgWnd, g_timerID);
				auto ret = MessageBoxW(nullptr, L"Checked that the process is suspected to have crashed abnormally and has stopped autoloading.",
					L"DWMBlurGlass Error", MB_ICONERROR | MB_RETRYCANCEL);

				iter->second.times = steady_clock::now().time_since_epoch();

				if (ret != IDRETRY)
					return;
				else
					retry = true;
			}
			iter->second.times = steady_clock::now().time_since_epoch();
			if(!retry)
				iter->second.init = false;
		}
		else
			sessionList.insert(std::make_pair(sessionId, sessionData()));

		std::wstring err;
		if(!LoadDWMExtensionBase(err, id))
		{
			MessageBoxW(0, err.c_str(), L"DWMBlurGlass Error", MB_ICONERROR);
		}
	}

	LRESULT CALLBACK MsgWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if(message == WM_APP + 20 && MClientNotifyType::Shutdown == (MClientNotifyType)wParam)
		{
			for(auto& id : g_proclist)
			{
				HWND msgwnd = FindMessageWnd(GetProcessId(L"dwm.exe"));
				if (IsWindow(msgwnd))
					SendMessageW(msgwnd, WM_APP + 20, (WPARAM)MHostNotifyType::Shutdown, 0);

				std::wstring err;
				UnInject(id, Utils::GetCurrentDir() + L"\\DWMBlurGlassExt.dll", err);
			}
		}
		else if(message == WM_APP + 20 && MClientNotifyType::QueryTransparency == (MClientNotifyType)wParam)
		{
			MHostNotify(MHostNotifyType::EnableTransparency, IsEnableTransparency());
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

	// 检查并自动下载符号文件
	bool CheckAndDownloadSymbols()
	{
		// 加载配置文件
		std::wstring configPath = Utils::GetCurrentDir() + L"\\data\\config.ini";
		ConfigData config = ConfigData::LoadFromFile(configPath);

		// 如果禁用了自动下载，直接返回
		if (!config.autoDownloadSymbols)
			return true;

		// 检查符号是否有效
		SymbolDownloader downloader;
		if (downloader.CheckSymbolsValid())
			return true;

		// 符号无效，显示下载对话框
		return SymbolDownloadDialog::Show(nullptr);
	}

	void MHostStartProcess()
	{
		// 检查并自动下载符号文件
		if (!CheckAndDownloadSymbols())
		{
			// 用户取消或下载失败时，显示错误信息
			std::wstring errMsg = GetLanguageString(L"symdownfail");
			if (errMsg.empty()) 
				errMsg = L"Failed to download symbol files. The program may not work properly.";
			MessageBoxW(nullptr, errMsg.c_str(), L"DWMBlurGlass", MB_ICONWARNING);
		}

		WNDCLASSEXW wx = {};
		wx.cbSize = sizeof(WNDCLASSEX);
		wx.lpfnWndProc = MsgWndProc;
		wx.hInstance = GetModuleHandleW(nullptr);
		wx.lpszClassName = DWMBlurGlassHostNotifyClassName;
		if (!RegisterClassExW(&wx))
			return;

		g_hostMsgWnd = CreateWindowExW(0, wx.lpszClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);

		g_timerID = SetTimer(g_hostMsgWnd, 1000, 500, [](HWND, UINT, UINT_PTR, DWORD)
		{
			RefreshProcessList();
		});

		MSG msg;
		while (GetMessageW(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		KillTimer(g_hostMsgWnd, g_timerID);
		UnregisterClassW(wx.lpszClassName, wx.hInstance);
	}

	bool LoadSymbolOffset()
	{
		g_dwmcoreFunCount = g_udwmFunCount = 0;
		for (auto& list : MDWMBlurGlassExt::g_hookFunList)
		{
			if (list.first == dwmcore)
				g_dwmcoreFunCount++;
			else if (list.first == udwm)
				g_udwmFunCount++;
		}

		std::wstring symPath = L"SRV*" + Utils::GetCurrentDir() + L"\\data\\symbols";
		HRESULT hr = g_symResolver.Walk(L"dwmcore.dll", "*!*", symPath, SymCallback_Dwmcore);
		if (FAILED(hr))
			return false;
		hr = g_symResolver.Walk(L"uDwm.dll", "*!*", symPath, SymCallback_uDwm);
		return SUCCEEDED(hr);
	}

	bool LoadDWMExtensionBase(std::wstring& err, DWORD pid)
	{
		if(!LoadSymbolOffset())
		{
			err = GetLanguageString(L"symloadfail");
			return false;
		}
		const bool ret = Inject(GetProcessId(L"dwm.exe"), Utils::GetCurrentDir() + L"\\DWMBlurGlassExt.dll", err);
		if(ret)
		{
			auto enumproc = [](HWND hWnd, LPARAM _pid) -> BOOL
			{
				DWORD dwPid = 0;
				GetWindowThreadProcessId(hWnd, &dwPid);
				if(_pid == dwPid)
				{
					PostMessageW(hWnd, WM_THEMECHANGED, 0, 0);
					return FALSE;
				}
				return TRUE;
			};
			EnumWindows(enumproc, pid);

			//PostMessageW(FindWindowW(L"Dwm", nullptr), WM_THEMECHANGED, 0, 0);
		}
		return ret;
	}

	void MHostNotify(MHostNotifyType type, LPARAM lParam)
	{
		HWND msgwnd = FindMessageWnd(GetProcessId(L"dwm.exe"));
		if (!IsWindow(msgwnd)) return;

		SendMessageW(msgwnd, WM_APP + 20, (WPARAM)type, lParam);
	}
}

extern "C" __declspec(dllexport) DWORD64 __stdcall GetModuleOffset(size_t index)
{
	using namespace MDWMBlurGlass;
	if (index < sizeof g_hookFunOffsetList)
		return g_hookFunOffsetList[index];
	return 0;
}