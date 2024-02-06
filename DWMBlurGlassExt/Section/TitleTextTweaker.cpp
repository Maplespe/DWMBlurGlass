/**
 * FileName: TitleTextTweaker.cpp
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
#include "TitleTextTweaker.h"
#include "CommonDef.h"
#include "AccentBlurEffect.h"

namespace MDWMBlurGlassExt::TitleTextTweaker
{
	using namespace MDWMBlurGlass;
	using namespace CommonDef;
	using namespace DWM;

	PVOID g_pCreateBitmapFromHBITMAP = nullptr;
	decltype(DrawTextW)* g_funDrawTextW = nullptr;
	decltype(FillRect)* g_funFillRect = nullptr;
	decltype(CreateBitmap)* g_funCreateBitmap = nullptr;
	decltype(Vtbl::IWICImagingFactory2Vtbl::CreateBitmapFromHBITMAP) g_funCreateBitmapFromHBITMAP = nullptr;

	//Win10 TextColor
	MinHook g_funCTopLevelWindow_UpdateWindowVisuals
	{
		"CTopLevelWindow::UpdateWindowVisuals", CTopLevelWindow_UpdateWindowVisuals
	};

	MinHook g_funCTopLevelWindow_UpdateText
	{
		"CTopLevelWindow::UpdateText", CTopLevelWindow_UpdateText
	};

	void Attach()
	{
		if (auto wicFactory = CDesktopManager::s_pDesktopManagerInstance->GetWICFactory())
		{
			if (((Vtbl::IWICImagingFactory2*)wicFactory)->lpVtbl)
			{
				g_pCreateBitmapFromHBITMAP = &((Vtbl::IWICImagingFactory2*)wicFactory)->lpVtbl->CreateBitmapFromHBITMAP;
				g_funCreateBitmapFromHBITMAP = ((Vtbl::IWICImagingFactory2*)wicFactory)->lpVtbl->CreateBitmapFromHBITMAP;
			}

			if (g_pCreateBitmapFromHBITMAP)
				WriteMemory(g_pCreateBitmapFromHBITMAP, [&] { *(PVOID*)g_pCreateBitmapFromHBITMAP = MyCreateBitmapFromHBITMAP; });
		}

		if (os::buildNumber < 22621)
		{
			g_funCTopLevelWindow_UpdateWindowVisuals.Attach();
			g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<1, true>();
		}
		else if (os::buildNumber >= 22621)
		{
			g_funCTopLevelWindow_UpdateText.Attach();
		}

		HMODULE udwmModule = GetModuleHandleW(L"udwm.dll");
		if (os::buildNumber < 22621)
		{
			HMODULE hModule = GetModuleHandleW(L"user32.dll");
			g_funFillRect = (decltype(g_funFillRect))GetProcAddress(hModule, "FillRect");
			g_funDrawTextW = (decltype(g_funDrawTextW))GetProcAddress(hModule, "DrawTextW");
			WriteIAT(udwmModule, "User32.dll", { { "FillRect", MyFillRect }, { "DrawTextW", MyDrawTextW } });
		}
		if (os::buildNumber < 22621)
		{
			HMODULE hModule = GetModuleHandleW(L"gdi32.dll");
			g_funCreateBitmap = (decltype(g_funCreateBitmap))GetProcAddress(hModule, "CreateBitmap");
			WriteIAT(udwmModule, "gdi32.dll", { { "CreateBitmap", MyCreateBitmap } });
		}
	}

	void Detach()
	{
		if (os::buildNumber >= 22621)
		{
			g_funCTopLevelWindow_UpdateText.Detach();
		}
		else if (os::buildNumber < 22621)
		{
			g_funCTopLevelWindow_UpdateWindowVisuals.Detach();
			g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<1, false>();
		}

		if (g_pCreateBitmapFromHBITMAP)
			WriteMemory(g_pCreateBitmapFromHBITMAP, [&] {*(PVOID*)g_pCreateBitmapFromHBITMAP = g_funCreateBitmapFromHBITMAP; });

		HMODULE udwmModule = GetModuleHandleW(L"udwm.dll");
		if (os::buildNumber < 22621)
		{
			WriteIAT(udwmModule, "User32.dll", { { "DrawTextW", g_funDrawTextW } , { "FillRect", g_funFillRect } });
		}
		if (os::buildNumber < 22621)
			WriteIAT(udwmModule, "gdi32.dll", { { "CreateBitmap", g_funCreateBitmap } });
	}

	int MyFillRect(HDC hdc, LPRECT rect, HBRUSH hbrush)
	{
		return g_funFillRect(hdc, rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}

	int MyDrawTextW(HDC hdc, LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format)
	{
		SetBkMode(hdc, TRANSPARENT);
		int result = 0;

		if ((format & DT_CALCRECT) || (format & DT_INTERNAL) || (format & DT_NOCLIP))
		{
			if (os::buildNumber < 22621)
			{
				//兼容第三方主题绘制发光字需要预留一些区域
				if (format & DT_CALCRECT)
				{
					auto ret = g_funDrawTextW(hdc, lpchText, cchText, lprc, format);
					lprc->right += 30;
					lprc->bottom += 20;
					lprc->top = 0;
					lprc->left = -10;
					return ret + 20;
				}
			}
			return g_funDrawTextW(hdc, lpchText, cchText, lprc, format);
		}
		auto drawTextCallback = [](HDC hdc, LPWSTR pszText, int cchText, LPRECT prc, UINT dwFlags, LPARAM lParam) -> int
			{
				return *reinterpret_cast<int*>(lParam) = DrawTextW(hdc, pszText, cchText, prc, dwFlags);
			};
		DTTOPTS Options =
		{
			sizeof(DTTOPTS),
			DTT_TEXTCOLOR | DTT_CALLBACK | DTT_COMPOSITED | DTT_GLOWSIZE,
			GetTextColor(hdc),
			0,
			0,
			0,
			{},
			0,
			0,
			0,
			0,
			FALSE,
			15,
			drawTextCallback,
			(LPARAM)&result
		};
		HTHEME hTheme = OpenThemeData(nullptr, L"Composited::Window");

		auto clean = RAIIHelper::scope_exit([&] { CloseThemeData(hTheme); });

		if (!hTheme)
			return g_funDrawTextW(hdc, lpchText, cchText, lprc, format);

		RECT offset = *lprc;
		if (os::buildNumber < 22621)
		{
			offset.left += 20;
			offset.top += 10;
		}
		if (FAILED(DrawThemeTextEx(hTheme, hdc, 0, 0, lpchText, cchText, DT_LEFT | DT_TOP, &offset, &Options)))
			return g_funDrawTextW(hdc, lpchText, cchText, lprc, format);
		return 1;
	}

	HRESULT MyCreateBitmapFromHBITMAP(IWICImagingFactory2* This, HBITMAP hBitmap, HPALETTE hPalette,
		WICBitmapAlphaChannelOption options, IWICBitmap** ppIBitmap)
	{
		options = WICBitmapUsePremultipliedAlpha;
		return g_funCreateBitmapFromHBITMAP(This, hBitmap, hPalette, options, ppIBitmap);
	}

	HBITMAP MyCreateBitmap(int nWidth, int nHeight, UINT nPlanes, UINT nBitCount, const void* lpBits)
	{
		if (nWidth == 1 && nHeight == 1)
			return g_funCreateBitmap(nWidth, nHeight, nPlanes, nBitCount, lpBits);

		return CreateAlphaBitmap(nWidth, nHeight);
	}

	HRESULT CTopLevelWindow_UpdateWindowVisuals(CTopLevelWindow* This)
	{
		CTopLevelWindow::WindowFrame* frame{nullptr};
		if (os::buildNumber < 22000)
		{
			frame = CTopLevelWindow::s_ChooseWindowFrameFromStyle
			(
				(char)*((DWORD*)This + 148),
				false,
				(char)(*(BYTE*)(*((ULONG64*)This + 91) + 611) & 0x20) != 0
			);
		}
		else
		{
			frame = CTopLevelWindow::s_ChooseWindowFrameFromStyle
			(
				(char)*((DWORD*)This + 152),
				false,
				(char)(*(BYTE*)(*((ULONG64*)This + 94) + 667) & 0x20) != 0
			);
		}

		//对于Ribbon或扩展到NC区域的窗口 我们无法处理标题文本颜色 需要过滤掉 它应该由应用程序处理
		if (frame && (*((BYTE*)This + 592) & 8) != 0)
		{
			auto ret = g_funCTopLevelWindow_UpdateWindowVisuals.call_org(This);

			auto isDark = This->GetData()->IsUsingDarkMode();

			if (const auto text = This->GetCText(); This->TreatAsActiveWindow())
				text->SetColor(isDark ? g_configData.activeTextColorDark : g_configData.activeTextColor);
			else
				text->SetColor(isDark ? g_configData.inactiveTextColorDark : g_configData.inactiveTextColor);

			return ret;
		}

		return g_funCTopLevelWindow_UpdateWindowVisuals.call_org(This);
	}

	HRESULT CTopLevelWindow_UpdateText(CTopLevelWindow* This, CTopLevelWindow::WindowFrame* a2, double a3)
	{
		if (a2 && (*((BYTE*)This + 624) & 8) != 0)
		{
			auto ret = g_funCTopLevelWindow_UpdateText.call_org(This, a2, a3);

			auto isDark = This->GetData()->IsUsingDarkMode();

			if (This->TreatAsActiveWindow())
				This->CDWriteTextSetColor(isDark ? g_configData.activeTextColorDark : g_configData.activeTextColor);//Active
			else
				This->CDWriteTextSetColor(isDark ? g_configData.inactiveTextColorDark : g_configData.inactiveTextColor);//Inactive

			return ret;
		}

		return g_funCTopLevelWindow_UpdateText.call_org(This, a2, a3);
	}

	HRESULT CTopLevelWindow_ValidateVisual(CTopLevelWindow* This)
	{
		const auto windowData = This->GetData();
		if (!windowData)
			return S_OK;

		const HWND hWnd = windowData->GetHWND();
		const auto accentPolicy = windowData->GetAccentPolicy();

		g_window = hWnd;

		if (os::buildNumber < 22621 && This->HasNonClientBackground())
		{
			//DrawText处给发光字预留了空间 需要刷新文本布局
			if (CText* textThis = This->GetCText())
			{
				if (auto title = textThis->GetText())
				{
					textThis->SetText(title);
					/*SIZE size = *((SIZE*)textThis + 15);
					size.cy += 10;
					CText_SetSize(textThis, &size);*/
				}
			}
		}
		return S_OK;
	}
}
