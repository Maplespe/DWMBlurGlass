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
#include "Helper/Helper.h"
#include "UIManager.h"
#include "Common.h"
#include "../DWMBlurGlassExt/Common/DefFunctionList.h"

#pragma data_seg(".DWMBlurGlassShared")

DWORD64 g_hookFunOffsetList[MDWMBlurGlassExt::g_hookFunList.size()] = { 0 };

size_t g_dwmcoreFunCount = 0;
size_t g_udwmFunCount = 0;

#pragma data_seg()
#pragma comment(linker,"/SECTION:.DWMBlurGlassShared,RWS")

namespace MDWMBlurGlass
{
    SymbolResolver g_symResolver;

	void ParsingSymbol(PSYMBOL_INFO symInfo, std::string& funName, DWORD64& offset)
	{
		auto functionName{ reinterpret_cast<const CHAR*>(symInfo->Name) };
		CHAR unDecoratedFunctionName[MAX_PATH + 1]{};
		UnDecorateSymbolName(
			functionName, unDecoratedFunctionName, MAX_PATH,
			UNDNAME_COMPLETE | UNDNAME_NO_ACCESS_SPECIFIERS | UNDNAME_NO_THROW_SIGNATURES
		);
		CHAR fullyUnDecoratedFunctionName[MAX_PATH + 1]{};
		UnDecorateSymbolName(
			functionName, fullyUnDecoratedFunctionName, MAX_PATH,
			UNDNAME_NAME_ONLY
		);

		funName = fullyUnDecoratedFunctionName;
		offset = symInfo->Address - symInfo->ModBase;
	}

	bool SymCallback_Dwmcore(PSYMBOL_INFO symInfo, ULONG symbolSize)
	{
		DWORD64 offset = 0;
		std::string funName;
		ParsingSymbol(symInfo, funName, offset);

		size_t funCount = 0;
		for (size_t i = 0; i < MDWMBlurGlassExt::g_hookFunList.size(); ++i)
		{
			auto& [type, _funName] = MDWMBlurGlassExt::g_hookFunList[i];
			if (type == dwmcore && funName == _funName)
			{
				g_hookFunOffsetList[i] = offset;
				funCount++;
			}
		}

		return g_dwmcoreFunCount != funCount;
	}

	bool SymCallback_uDwm(PSYMBOL_INFO symInfo, ULONG symbolSize)
	{
		DWORD64 offset = 0;
		std::string funName;
		ParsingSymbol(symInfo, funName, offset);

		size_t funCount = 0;
		for (size_t i = 0; i < MDWMBlurGlassExt::g_hookFunList.size(); ++i)
		{
			auto& [type, _funName] = MDWMBlurGlassExt::g_hookFunList[i];
			if (type == udwm && funName == _funName)
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

	bool MHostDownloadSymbol()
	{
		auto callback = [](PSYMBOL_INFO, ULONG) { return false; };
		HRESULT hr = g_symResolver.Walk(L"dwmcore.dll", "*!*", callback);
		if (FAILED(hr))
			return false;
		hr = g_symResolver.Walk(L"uDwm.dll", "*!*", callback);
		return SUCCEEDED(hr);
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

	bool LoadDWMExtensionBase(std::wstring& err)
	{
		if(!LoadSymbolOffset())
		{
			err = GetBaseLanguageString(L"symloadfail");
			return false;
		}
		bool success = true;
		for (DWORD index = 0;; index++) {
			DWORD pid = GetProcessId(L"dwm.exe", index);
			if (pid == 0)
			{
				break
			}
			if(Inject(pid, Utils::GetCurrentDir() + L"\\DWMBlurGlassExt.dll", err))
			{
				PostMessageW(FindWindowW(L"Dwm", nullptr), WM_THEMECHANGED, 0, 0);
			} else {
				success = false;
			}
		}
		return ret;
	}

	bool LoadDWMExtension(std::wstring& err, Mui::XML::MuiXML* ui)
	{
		if (!LoadSymbolOffset())
		{
			err = ui->GetStringValue(L"symloadfail");
			return false;
		}
		bool success = true;
		for (DWORD index = 0;; index++) {
			DWORD pid = GetProcessId(L"dwm.exe", index);
			if (pid == 0)
			{
				break
			}
			if (!Inject(pid, Utils::GetCurrentDir() + L"\\DWMBlurGlassExt.dll", err)) {
				success = false;
			}
		}
		return sucesss;
	}

	bool ShutdownDWMExtension(std::wstring& err)
	{
		MHostNotify(MHostNotifyType::Shutdown);
		bool success = true;
		for (DWORD index = 0;; index++) {
			DWORD pid = GetProcessId(L"dwm.exe", index);
			if (pid == 0)
			{
				break
			}
			if (!UnInject(pid, Utils::GetCurrentDir() + L"\\DWMBlurGlassExt.dll", err)) {
				success = false;
			}
		}
		return sucesss;
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

	void MHostNotify(MHostNotifyType type)
	{
		for (DWORD index = 0;; index++) {
			DWORD pid = GetProcessId(L"dwm.exe", index);
			if (pid == 0)
			{
				break
			}
			HWND msgwnd = FindMessageWnd(pid);
			if (!IsWindow(msgwnd)) continue;

			SendMessageW(msgwnd, WM_APP + 20, (WPARAM)type, 0);
		}
	}
}

extern "C" __declspec(dllexport) DWORD64 __stdcall GetModuleOffset(size_t index)
{
	using namespace MDWMBlurGlass;
	if (index < sizeof g_hookFunOffsetList)
		return g_hookFunOffsetList[index];
	return 0;
}
