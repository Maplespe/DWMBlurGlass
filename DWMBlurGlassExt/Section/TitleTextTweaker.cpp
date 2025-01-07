/**
 * FileName: TitleTextTweaker.cpp
 *
 * Copyright (C) 2024 ALTaleX531, Maplespe
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
#include "wil.h"
#include <shellscalingapi.h>
#pragma comment(lib, "shcore.lib")

namespace MDWMBlurGlassExt::TitleTextTweaker
{
	using namespace MDWMBlurGlass;
	using namespace CommonDef;
	using namespace DWM;

	std::atomic_bool g_startup = false;

	SIZE g_textRcSize{ 0 };
	CText* g_textVisual{ nullptr };
	constexpr int g_textGlowSize{ 16 };

	PVOID g_pCreateBitmapFromHBITMAP = nullptr;
	decltype(DrawTextW)* g_funDrawTextW = nullptr;
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

	MinHook g_funCText_ValidateResources
	{
		"CText::ValidateResources", CText_ValidateResources
	};

	MinHook g_funCMatrixTransformProxy_Update
	{
		"CMatrixTransformProxy::Update", CMatrixTransformProxy_Update
	};

	MinHook g_funCChannel_MatrixTransformUpdate
	{
		"CChannel::MatrixTransformUpdate", CChannel_MatrixTransformUpdate
	};

	void Attach()
	{
		if(g_startup)
			return;
		g_startup = true;
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
			g_funCText_ValidateResources.Attach();
			//g_funCChannel_MatrixTransformUpdate.Attach();
			g_funCMatrixTransformProxy_Update.Attach();
		}
		else if (os::buildNumber >= 22621)
		{
			g_funCTopLevelWindow_UpdateText.Attach();
		}

		HMODULE udwmModule = GetModuleHandleW(L"udwm.dll");
		if (os::buildNumber < 22621)
		{
			HMODULE hModule = GetModuleHandleW(L"user32.dll");
			g_funDrawTextW = (decltype(g_funDrawTextW))GetProcAddress(hModule, "DrawTextW");
			WriteIAT(udwmModule, "User32.dll", { { "DrawTextW", MyDrawTextW } });
			
			hModule = GetModuleHandleW(L"gdi32.dll");
			g_funCreateBitmap = (decltype(g_funCreateBitmap))GetProcAddress(hModule, "CreateBitmap");
			WriteIAT(udwmModule, "gdi32.dll", { { "CreateBitmap", MyCreateBitmap } });
		}
	}

	void Detach()
	{
		if (!g_startup)
			return;
		if (os::buildNumber >= 22621)
		{
			g_funCTopLevelWindow_UpdateText.Detach();
		}
		else if (os::buildNumber < 22621)
		{
			g_funCMatrixTransformProxy_Update.Detach();
			//g_funCChannel_MatrixTransformUpdate.Detach();
			g_funCText_ValidateResources.Detach();
			g_funCTopLevelWindow_UpdateWindowVisuals.Detach();
		}

		if (g_pCreateBitmapFromHBITMAP)
			WriteMemory(g_pCreateBitmapFromHBITMAP, [&] {*(PVOID*)g_pCreateBitmapFromHBITMAP = g_funCreateBitmapFromHBITMAP; });

		HMODULE udwmModule = GetModuleHandleW(L"udwm.dll");
		if (os::buildNumber < 22621)
			WriteIAT(udwmModule, "User32.dll", { { "DrawTextW", g_funDrawTextW } });

		if (os::buildNumber < 22621)
			WriteIAT(udwmModule, "gdi32.dll", { { "CreateBitmap", g_funCreateBitmap } });

		g_startup = false;
	}

	int MyDrawTextW(HDC hdc, LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format)
	{
		int result{ 0 };
		auto drawTextCallback = [](HDC hdc, LPWSTR pszText, int cchText, LPRECT prc, UINT dwFlags, LPARAM lParam) -> int
		{
			return *reinterpret_cast<int*>(lParam) = g_funDrawTextW(hdc, pszText, cchText, prc, dwFlags);
		};

		if ((format & DT_CALCRECT))
		{
			result = g_funDrawTextW(hdc, lpchText, cchText, lprc, format);

			g_textRcSize.cx = wil::rect_width(*lprc);
			// reservice space for the glow
			lprc->right += g_textGlowSize * 2;
			lprc->bottom += g_textGlowSize * 2;
			// if the text height exceed the height of the text visual
			// then later it will be set to the height of the visual
			g_textRcSize.cy = wil::rect_height(*lprc);

			return result;
		}
		// clear the background, so the text can be shown transparent
		// with this hack, we don't need to hook FillRect any more
		BITMAP bmp{};
		if (GetObjectW(GetCurrentObject(hdc, OBJ_BITMAP), sizeof(bmp), &bmp) && bmp.bmBits)
		{
			memset(bmp.bmBits, 0, 4 * bmp.bmWidth * bmp.bmHeight);
		}

		// override that so we can use the correct param in CDrawImageInstruction::Create
		lprc->bottom = lprc->top + g_textRcSize.cy;

		lprc->left += g_textGlowSize;
		lprc->top += g_textGlowSize;
		lprc->right -= g_textGlowSize;
		lprc->bottom -= g_textGlowSize;

		DTTOPTS options
		{
			sizeof(DTTOPTS),
			DTT_TEXTCOLOR | DTT_COMPOSITED | DTT_CALLBACK | DTT_GLOWSIZE,
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
			g_textGlowSize,
			drawTextCallback,
			(LPARAM)&result
		};
		wil::unique_htheme hTheme{ OpenThemeData(nullptr, L"Composited::Window") };

		if (hTheme)
		{
			LOG_IF_FAILED(DrawThemeTextEx(hTheme.get(), hdc, 0, 0, lpchText, cchText, format, lprc, &options));
		}
		else
		{
			LOG_HR_IF_NULL(E_FAIL, hTheme);
			result = g_funDrawTextW(hdc, lpchText, cchText, lprc, format);
		}

		lprc->left -= g_textGlowSize;
		lprc->top -= g_textGlowSize;
		lprc->right += g_textGlowSize;
		lprc->bottom += g_textGlowSize;

		return result;
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

		nHeight = g_textRcSize.cy;
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
		BYTE flag = 0;
		if(os::buildNumber < 26100)
			flag = (*((BYTE*)This + 624) & 8);
		else
			flag = (*((BYTE*)This + 584) & 8);

		if (a2 && flag != 0)
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

	HRESULT CText_ValidateResources(CText* This)
	{
		g_textVisual = This;
		HRESULT hr = g_funCText_ValidateResources.call_org(This);
		g_textVisual = nullptr;

		return hr;
	}

	HRESULT CMatrixTransformProxy_Update(CMatrixTransformProxy* This, MilMatrix3x2D* matrix)
	{
		if (g_textVisual)
		{
			matrix->DX -= static_cast<DOUBLE>(g_textGlowSize) * (g_textVisual->IsRTL() ? -1.f : 1.f);
			matrix->DY = static_cast<DOUBLE>(static_cast<LONG>(static_cast<DOUBLE>(g_textVisual->GetHeight() - g_textRcSize.cy) / 2. - 0.5));
		}

		return g_funCMatrixTransformProxy_Update.call_org(This, matrix);
	}

	HRESULT CChannel_MatrixTransformUpdate(Core::CChannel* This, UINT handleIndex, MilMatrix3x2D* matrix)
	{
		if (g_textVisual)
		{
			matrix->DX -= static_cast<DOUBLE>(g_textGlowSize) * (g_textVisual->IsRTL() ? -1.f : 1.f);
			matrix->DY = static_cast<DOUBLE>(static_cast<LONG>(static_cast<DOUBLE>(g_textVisual->GetHeight() - g_textRcSize.cy) / 2. - 0.5));
		}

		return g_funCChannel_MatrixTransformUpdate.call_org(This, handleIndex, matrix);
	}
}
