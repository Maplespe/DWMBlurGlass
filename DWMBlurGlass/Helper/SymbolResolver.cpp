/**
 * FileName: SymbolResolver.cpp
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
#include "winrt.h"
#include "wil.h"
#include "SymbolResolver.h"
#include "Helper.h"
#include <Shlwapi.h>
#include <iostream>
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "shlwapi.lib")

using namespace std;

namespace MDWMBlurGlass
{
	BOOL CALLBACK SymbolResolver::SymCallback(
		HANDLE hProcess,
		ULONG ActionCode,
		ULONG64 CallbackData,
		ULONG64 UserContext
	)
	{
		if (ActionCode == CBA_EVENT)
		{
			//if (UserContext)
			//{
			//	auto& symbolResolver{ *reinterpret_cast<SymbolResolver*>(UserContext) };
			//	auto event{ reinterpret_cast<PIMAGEHLP_CBA_EVENTW>(CallbackData) };

			//	if (wcsstr(event->desc, L"from http://"))
			//	{
			//		symbolResolver.m_printInfo = true;
			//		if (GetConsoleWindow() || AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole())
			//		{
			//			FILE* fpstdin{ stdin }, * fpstdout{ stdout }, * fpstderr{ stderr };
			//			freopen_s(&fpstdin, "CONIN$", "r", stdin);
			//			freopen_s(&fpstdout, "CONOUT$", "w", stdout);
			//			freopen_s(&fpstderr, "CONOUT$", "w", stderr);
			//			_wsetlocale(LC_ALL, L"chs");
			//		}
			//	}
			//	if (wcsstr(event->desc, L"SYMSRV:  HTTPGET: /download/symbols/index2.txt"))
			//	{
			//		//
			//		std::wcout << event->desc << std::endl;
			//	}
			//	if (symbolResolver.m_printInfo)
			//	{
			//		//
			//		std::wcout << event->desc << std::endl;
			//	}
			//	if (wcsstr(event->desc, L"copied"))
			//	{
			//		symbolResolver.m_printInfo = false;
			//		FreeConsole();
			//	}
			//}

			return TRUE;
		}
		return FALSE;
	}

	SymbolResolver::SymbolResolver()
	{
		try
		{
			THROW_IF_WIN32_BOOL_FALSE(SymInitialize(GetCurrentProcess(), nullptr, FALSE));

			SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_ALLOW_ABSOLUTE_SYMBOLS | SYMOPT_OMAP_FIND_NEAREST);
			THROW_IF_WIN32_BOOL_FALSE(SymRegisterCallbackW64(GetCurrentProcess(), SymCallback, reinterpret_cast<ULONG64>(this)));

			wstring symPath = L"SRV*" + Utils::GetCurrentDir() + L"\\data\\symbols";
			THROW_IF_WIN32_BOOL_FALSE(SymSetSearchPathW(GetCurrentProcess(), symPath.c_str()));
		}
		catch (...)
		{
			SymbolResolver::~SymbolResolver();
			throw;
		}
	}

	SymbolResolver::~SymbolResolver() noexcept
	{
		SymCleanup(GetCurrentProcess());
	}

	HRESULT SymbolResolver::Walk(std::wstring_view dllName, string_view mask, function<bool(PSYMBOL_INFO, ULONG)> callback) try
	{
		wstring symPath = L"SRV*" + Utils::GetCurrentDir() + L"\\data\\symbols*http://msdl.microsoft.com/download/symbols";
		return Walk(dllName, mask, symPath, callback);
	}
	CATCH_LOG_RETURN_HR(wil::ResultFromCaughtException())

	HRESULT SymbolResolver::Walk(std::wstring_view dllName, std::string_view mask, std::wstring_view srv, std::function<bool(PSYMBOL_INFO, ULONG)> callback) try
	{
		DWORD64 dllBase{ 0 };
		WCHAR filePath[MAX_PATH + 1]{}, symFile[MAX_PATH + 1]{};
		MODULEINFO modInfo{};

		auto cleanUp = wil::scope_exit([&]
		{
			if (dllBase != 0)
			{
				SymUnloadModule64(GetCurrentProcess(), dllBase);
				dllBase = 0;
			}
		});

		THROW_HR_IF(E_INVALIDARG, dllName.empty());

		wil::unique_hmodule moduleHandle{ LoadLibraryExW(dllName.data(), nullptr, DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_SEARCH_SYSTEM32) };
		THROW_LAST_ERROR_IF_NULL(moduleHandle);
		THROW_LAST_ERROR_IF(GetModuleFileNameW(moduleHandle.get(), filePath, MAX_PATH) == 0);
		THROW_IF_WIN32_BOOL_FALSE(GetModuleInformation(GetCurrentProcess(), moduleHandle.get(), &modInfo, sizeof(modInfo)));

		if (SymGetSymbolFileW(GetCurrentProcess(), nullptr, filePath, sfPdb, symFile, MAX_PATH, symFile, MAX_PATH) == FALSE)
		{
			m_requireInternet = true;
			DWORD lastError{ GetLastError() };
			THROW_WIN32_IF(lastError, lastError != ERROR_FILE_NOT_FOUND);

			WCHAR curDir[MAX_PATH + 1]{};
			THROW_LAST_ERROR_IF(GetModuleFileName(HINST_THISCOMPONENT, curDir, MAX_PATH) == 0);
			PathRemoveFileSpec(curDir);

			DWORD options = SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_DEBUG | SYMOPT_ALLOW_ABSOLUTE_SYMBOLS | SYMOPT_OMAP_FIND_NEAREST);

			auto cleanUp = wil::scope_exit([&]
			{
				SymSetOptions(options);
			});


			THROW_IF_WIN32_BOOL_FALSE(SymGetSymbolFileW(GetCurrentProcess(), srv.data(), filePath, sfPdb, symFile, MAX_PATH, symFile, MAX_PATH));

		}
		m_symbolsOK = true;

		dllBase = SymLoadModuleExW(GetCurrentProcess(), nullptr, filePath, nullptr, reinterpret_cast<DWORD64>(modInfo.lpBaseOfDll), modInfo.SizeOfImage, nullptr, 0);
		THROW_LAST_ERROR_IF(dllBase == 0);
		THROW_IF_WIN32_BOOL_FALSE(SymEnumSymbols(GetCurrentProcess(), dllBase, mask.empty() ? nullptr : mask.data(), SymbolResolver::EnumSymbolsCallback, &callback));

		return S_OK;
	}
	CATCH_LOG_RETURN_HR(wil::ResultFromCaughtException())

	bool SymbolResolver::GetSymbolStatus()
	{
		return m_symbolsOK;
	}

	bool SymbolResolver::GetSymbolSource()
	{
		return m_requireInternet;
	}

	BOOL SymbolResolver::EnumSymbolsCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
	{
		auto& callback{ *reinterpret_cast<function<bool(PSYMBOL_INFO symInfo, ULONG symbolSize)>*>(UserContext) };

		if (callback)
		{
			return static_cast<BOOL>(callback(pSymInfo, SymbolSize));
		}

		return TRUE;
	}
}