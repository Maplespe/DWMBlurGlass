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
#include "MainWindow.h"
#include "MDcompRender.h"
#include "Helper/Helper.h"
#include "MHostHelper.h"
#include "Extend/ColorDisplay.h"
#include <Knownfolders.h>
#include <Shlobj.h>

using namespace Mui;

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

	if (!MDWMBlurGlass::LoadLanguageFileList() 
		|| (!MDWMBlurGlass::LoadBaseLanguageString(MDWMBlurGlass::GetSystemLocalName())
		&& !MDWMBlurGlass::LoadBaseLanguageString(L"en-US")))
	{
		MessageBoxW(nullptr, L"初始化失败: 没有有效的语言文件 (Initialization failed: No valid language file).", L"error", MB_ICONERROR);
		return 0;
	}

	PWSTR shfolder = nullptr;
	if (SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &shfolder) == S_OK)
	{
		std::wstring path = shfolder;
		CoTaskMemFree(shfolder);
		std::wstring curpath = MDWMBlurGlass::Utils::GetCurrentDir();
		if (curpath.length() >= path.length() && curpath.substr(0, path.length()) == path)
		{
			MessageBoxW(nullptr,
				Helper::M_ReplaceString(MDWMBlurGlass::GetBaseLanguageString(L"initfail0"), L"{path}", curpath).c_str(),
				L"waring", MB_ICONWARNING | MB_TOPMOST);
			return false;
		}
	}

	if(lpCmdLine && _wcsicmp(lpCmdLine, L"loaddll") == 0)
	{
		std::wstring err;
		if (!MDWMBlurGlass::LoadDWMExtensionBase(err))
			MessageBoxW(nullptr, err.c_str(), L"DWMBlurGlass Error", MB_ICONERROR);
		return 0;
	}

	HANDLE hObject = CreateMutexW(nullptr, FALSE, L"_DWMBlurGlass_");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hObject);
		return 0;
	}
	if (hObject)
		ReleaseMutex(hObject);

	//初始化界面库 全局仅有一个MiaoUI类
	MiaoUI engine;
	std::wstring err;
	auto render = new MDWMBlurGlass::MRender_DComp();

	if (!Render::InitDirect2D(err, -1) || !engine.InitEngine(err, MiaoUI::Render::Custom, _m_ptrv(render)))
	{
		MessageBoxW(nullptr, (MDWMBlurGlass::GetBaseLanguageString(L"inituifail0") + err).c_str(), L"error", MB_ICONERROR);
		delete render;
		return 0;
	}

	auto context = engine.CreateWindowCtx({ 0,0,500,680 }, MWindowType::NoTitleBar,
		L"DWMBlurGlass " + MDWMBlurGlass::g_vernum, true, true, 0, WS_EX_NOREDIRECTIONBITMAP);
	if(!context)
	{
		MessageBoxW(nullptr, MDWMBlurGlass::GetBaseLanguageString(L"inituifail1").c_str(), L"error", MB_ICONERROR);
		delete render;
		return 0;
	}

	ColorDisplay::Register();

	context->SetEventCallback(MDWMBlurGlass::MainWindow_EventProc);
	context->SetEventSourceCallback(MDWMBlurGlass::MainWindow_SrcEventProc);
	if(!context->InitWindow(MDWMBlurGlass::MainWindow_InitWindow, false))
	{
		delete context;
		delete render;
		MessageBoxW(nullptr, MDWMBlurGlass::GetBaseLanguageString(L"inituifail2").c_str(), L"error", MB_ICONERROR);
		return 0;
	}

	MDWMBlurGlass::ClearBaseLanguage();

	render->InitBackdrop();

	context->Base()->SetResMode(true);
	//context->Base()->ShowDebugRect(true);
	context->Base()->CenterWindow();
	context->Base()->ShowWindow(true);


	context->EventLoop();

	MDWMBlurGlass::MainWindow_Destroy();

	Render::UninitDirect2D();

	engine.UnInitEngine();

	return 0;
}