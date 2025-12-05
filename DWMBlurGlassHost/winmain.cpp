/**
 * FileName: winmain.cpp
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
#include "framework.h"
#include "UIManager.h"
#include "Helper/Helper.h"
#include "MHostHelper.h"
#include <Knownfolders.h>
#include <Shlobj.h>
#include <minidumpapiset.h>
#include "Helper/wil.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

#ifndef _DEBUG
	if (!MDWMBlurGlass::IsRunasAdmin())
		return 0;
#endif

	auto [locale, parentLocale] = MDWMBlurGlass::GetSystemLocaleAndParent();

	if (!MDWMBlurGlass::LoadLanguageFileList() 
		|| (!MDWMBlurGlass::LoadLanguageString(locale) && !MDWMBlurGlass::LoadLanguageString(parentLocale)
		&& !MDWMBlurGlass::LoadLanguageString(L"en-US")))
	{
		MessageBoxW(nullptr, L"初始化失败: 没有有效的语言文件 (Initialization failed: No valid language file).", L"error", MB_ICONERROR);
		return 0;
	}

	if(lpCmdLine && _wcsicmp(lpCmdLine, L"runhost") == 0)
	{
		HANDLE hObject = CreateMutexW(nullptr, FALSE, L"_DWMBlurGlass_host_");
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			CloseHandle(hObject);
			return 0;
		}
		if (hObject)
			ReleaseMutex(hObject);

		MDWMBlurGlass::MHostStartProcess();

		return 0;
	}
	return 0;
}
