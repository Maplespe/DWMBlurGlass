/**
 * FileName: MainWindow.cpp
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
#include "MainWindow.h"
#include "UIManager.h"
#include "Helper/Helper.h"
#include "VersionHelper.h"
#include "Extend/ColorPickerDlg.h"
#include "Page/MainPage.h"
#include <User/Mui_Engine.h>
#include <dwmapi.h>
#include "resource.h"
#pragma comment(lib, "dwmapi.lib")

namespace MDWMBlurGlass
{
	using namespace Mui;

	ColorPickerDlg* g_colorDialog = nullptr;
	MainWindowPage* g_mainPage = nullptr;

	bool MainWindow_InitWindow(const MWindowCtx* ctx, UIControl* root, XML::MuiXML* ui)
	{
		if (!LoadDefualtUIStyle(ui))
			return false;

		auto [locale, parentLocale] = GetSystemLocaleAndParent();
		if (!LoadLanguageString(ui, locale, true))
		{
			if (!LoadLanguageString(ui, parentLocale, true))
			{
				if (!LoadLanguageString(ui, L"en-US", true))
					return false;
			}
		}
		else if (locale != L"en-US")
		{
			LoadLanguageString(ui, L"en-US", true, false);
		}

		const HWND hWnd = (HWND)ctx->Base()->GetWindowHandle();
		//SetWindowLongW(hWnd, GWL_STYLE, GetWindowLongW(hWnd, GWL_STYLE) & ~(WS_MAXIMIZEBOX | WS_SIZEBOX));
		HICON hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCE(IDI_ICON1));
		SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

		UIBkgndStyle bgStyle;
		bgStyle.bkgndColor = Color::M_RGBA(245, 241, 249, 255);
		root->SetBackground(bgStyle);

		try
		{
			g_mainPage = new MainWindowPage(root, ui);
			g_colorDialog = new ColorPickerDlg(root, ui);
			g_mainPage->CreateTitleBar(ui);
		}
		catch(...)
		{
			return false;
		}
		return true;
	}

	void MainWindow_Destroy()
	{
		delete g_colorDialog;
		delete g_mainPage;
	}

	void ShowColorPicker(_m_color showColor, bool alpha, std::function<void(bool, _m_color)> callback)
	{
		if (g_colorDialog)
			g_colorDialog->ShowColorPicker(showColor, alpha, std::move(callback));
	}

	_m_result MainWindow_SrcEventProc(MWindowCtx* ctx, const MWndDefEventSource& defcallback, MEventCodeEnum code, _m_param param)
	{
		if (g_mainPage && g_mainPage->SrcEventProc(ctx, defcallback, code, param))
			return 0;
		return defcallback(code, param);
	}

	_m_result MainWindow_EventProc(MWindowCtx*, UINotifyEvent event, UIControl* control, _m_param param)
	{
		if (g_colorDialog && g_colorDialog->EventProc(event, control, param))
			return true;
		if (g_mainPage && g_mainPage->EventProc(event, control, param))
			return true;
		return false;
	}

}