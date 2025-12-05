/**
 * FileName: SymbolDownloader.h
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
#pragma once
#include "../framework.h"
#include <string>
#include <functional>
#include <atomic>
#include <ShObjIdl.h>

namespace MDWMBlurGlass
{
	// 符号下载进度信息
	struct SymbolDownloadProgress
	{
		std::wstring fileName;		// 当前下载的文件名
		int currentFile = 0;		// 当前文件索引 (1-based)
		int totalFiles = 0;			// 总文件数
		ULONGLONG bytesReceived = 0;// 已接收字节数
		ULONGLONG totalBytes = 0;	// 总字节数
		int progressPercentage = 0;	// 进度百分比 (0-100)
	};

	// 进度回调函数类型
	using ProgressCallback = std::function<void(const SymbolDownloadProgress&)>;

	// PDB调试信息结构
	struct PEDebugInfo
	{
		GUID guid;
		DWORD age;
	};

	class SymbolDownloader
	{
	public:
		SymbolDownloader();
		~SymbolDownloader();

		// 检查符号文件是否有效
		bool CheckSymbolsValid();

		// 下载符号文件 (带进度回调)
		bool DownloadSymbols(ProgressCallback progressCallback = nullptr);

		// 取消下载
		void CancelDownload();

		// 获取最后的错误信息
		std::wstring GetLastError() const { return m_lastError; }

	private:
		// 检查单个DLL的符号是否有效
		bool CheckSingleSymbolValid(std::wstring_view dllName);

		// 下载单个符号文件
		bool DownloadSingleSymbol(std::wstring_view dllName, ProgressCallback progressCallback);

		// 从PE文件获取调试信息
		bool GetPEDebugInfo(std::wstring_view filePath, PEDebugInfo& info);

		// HTTP下载文件
		bool HttpDownloadFile(std::wstring_view url, std::wstring_view localPath,
			std::wstring_view fileName, int currentFile, int totalFiles,
			const ProgressCallback& progressCallback);

		std::wstring m_symbolsPath;
		std::wstring m_lastError;
		std::atomic<bool> m_cancelRequested{ false };

		static constexpr const wchar_t* TargetDlls[] = { L"dwmcore.dll", L"uDwm.dll" };
		static constexpr const wchar_t* SymbolServerUrl = L"https://msdl.microsoft.com/download/symbols";
	};

	// 符号下载进度对话框
	class SymbolDownloadDialog
	{
	public:
		// 显示下载对话框 (模态)
		// 返回: true = 下载成功, false = 下载失败或被取消
		static bool Show(HWND hParent = nullptr);

	private:
		static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		static DWORD WINAPI DownloadThreadProc(LPVOID lpParam);

		struct DialogData
		{
			SymbolDownloader* downloader = nullptr;
			HWND hDlg = nullptr;
			bool success = false;
			bool cancelled = false;
		};
	};
}
