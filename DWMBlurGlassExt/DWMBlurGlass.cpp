/**
 * FileName: DWMBlurGlass.cpp
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
#include "DefFunctionList.h"
#include "DWMBlurGlass.h"
#include "HookDef.h"
#include "VersionHelper.h"
#include "Helper.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

namespace MDWMBlurGlassExt
{
	using namespace MDWMBlurGlass;

	bool g_startup = false;
	ConfigData g_configData;

	PVOID* g_pDesktopManagerInstance_ptr = nullptr;
	PVOID g_pCreateBitmapFromHBITMAP = nullptr;
	IWICImagingFactory2* g_wicImageFactory = nullptr;
	CWindowList* g_windowList = nullptr;

	decltype(DrawTextW)* g_funDrawTextW = nullptr;
	decltype(FillRect)* g_funFillRect = nullptr;
	decltype(Vtbl::IWICImagingFactory2Vtbl::CreateBitmapFromHBITMAP) g_funCreateBitmapFromHBITMAP = nullptr;

	decltype(CWindowList_GetExtendedFrameBounds)* g_funCWindowList_GetExtendedFrameBounds = nullptr;
	decltype(CTopLevelWindow_OnAccentPolicyUpdated)* g_funCTopLevelWindow_OnAccentPolicyUpdated = nullptr;
	decltype(CTopLevelWindow_s_ChooseWindowFrameFromStyle)* g_funCTopLevelWindow_s_ChooseWindowFrameFromStyle = nullptr;
	decltype(CText_SetColor)* g_CText_SetColor = nullptr;

	thread_local RECT g_blurRegion{ 0, 0, -1, -1 };
	thread_local CAccent* g_accentThis = nullptr;
	thread_local HWND g_window = nullptr;

	//BlendColor
	MinHook g_funCRenderData_DrawSolidColorRectangle
	{
		"CRenderData::DrawSolidColorRectangle", CRenderData_DrawSolidColorRectangle
	};
	//Win11 BlurAmount
	MinHook g_funCRenderingTechnique_ExecuteBlur
	{
		"CRenderingTechnique::ExecuteBlur", CRenderingTechnique_ExecuteBlur
	};
	//Win10 BlurBehind
	MinHook g_funCD2DContext_FillEffect
	{
		"CD2DContext::FillEffect", CD2DContext_FillEffect
	};
	//Win10 WinUI HostBackdrop Brush
	MinHook g_funCCustomBlur_BuildEffect
	{
		"CCustomBlur::BuildEffect", CCustomBlur_BuildEffect
	};
	MinHook g_funCCustomBlur_DetermineOutputScale
	{
		"CCustomBlur::DetermineOutputScale", CCustomBlur_DetermineOutputScale
	};

	MinHook g_funCAccent_UpdateAccentBlurRect
	{
		"CAccent::UpdateAccentBlurRect", CAccent_UpdateAccentBlurRect
	};

	MinHook g_funCAccent_UpdateAccentPolicy
	{
		"CAccent::UpdateAccentPolicy", CAccent_UpdateAccentPolicy
	};

	MinHook g_funCTopLevelWindow_ValidateVisual
	{
		"CTopLevelWindow::ValidateVisual", CTopLevelWindow_ValidateVisual
	};
	//Win10 TextColor
	MinHook g_funCTopLevelWindow_UpdateWindowVisuals
	{
		"CTopLevelWindow::UpdateWindowVisuals", CTopLevelWindow_UpdateWindowVisuals
	};

	MinHook g_funCGlassColorizationParameters_AdjustWindowColorization
	{
		"CGlassColorizationParameters::AdjustWindowColorization", CGlassColorizationParameters_AdjustWindowColorization
	};

	MinHook g_funCTopLevelWindow_UpdateText
	{
		"CTopLevelWindow::UpdateText", CTopLevelWindow_UpdateText
	};

	bool Startup() try
	{
		if (g_startup) return true;
		g_startup = true;

		MHostLoadProcOffsetList();

		g_funCTopLevelWindow_s_ChooseWindowFrameFromStyle = (decltype(g_funCTopLevelWindow_s_ChooseWindowFrameFromStyle))MHostGetProcAddress("CTopLevelWindow::s_ChooseWindowFrameFromStyle");
		g_funCWindowList_GetExtendedFrameBounds = (decltype(g_funCWindowList_GetExtendedFrameBounds))MHostGetProcAddress("CWindowList::GetExtendedFrameBounds");
		g_funCTopLevelWindow_OnAccentPolicyUpdated = (decltype(g_funCTopLevelWindow_OnAccentPolicyUpdated))MHostGetProcAddress("CTopLevelWindow::OnAccentPolicyUpdated");
		g_CText_SetColor = (decltype(g_CText_SetColor))MHostGetProcAddress("CText::SetColor");

		if(os::buildNumber < 22000)
		{
			g_funCRenderData_DrawSolidColorRectangle.Attach();
			g_funCD2DContext_FillEffect.Attach();
			g_funCCustomBlur_BuildEffect.Attach();
			g_funCCustomBlur_DetermineOutputScale.Attach();
			g_funCAccent_UpdateAccentBlurRect.Attach();
			g_funCAccent_UpdateAccentPolicy.Attach();
			g_funCTopLevelWindow_UpdateWindowVisuals.Attach();
		}
		if(os::buildNumber >= 22000)
		{
			g_funCRenderingTechnique_ExecuteBlur.Attach();
			g_funCTopLevelWindow_UpdateText.Attach();
		}

		g_funCTopLevelWindow_ValidateVisual.Attach();
		g_funCGlassColorizationParameters_AdjustWindowColorization.Attach();

		g_pDesktopManagerInstance_ptr = (PVOID*)MHostGetProcAddress("CDesktopManager::s_pDesktopManagerInstance");
		if (g_pDesktopManagerInstance_ptr && *g_pDesktopManagerInstance_ptr)
		{
			if (os::buildNumber >= 22621)
			{
				// Windows 11 22H2
				g_wicImageFactory = (IWICImagingFactory2*)*((PVOID*)*g_pDesktopManagerInstance_ptr + 30);
			}
			else if(os::buildNumber < 22000)
			{
				// Windows 10
				g_wicImageFactory = (IWICImagingFactory2*)*((PVOID*)*g_pDesktopManagerInstance_ptr + 39);
				g_windowList = (CWindowList*)(*((PVOID*)*g_pDesktopManagerInstance_ptr + 61));
			}

			if (((Vtbl::IWICImagingFactory2*)g_wicImageFactory)->lpVtbl)
			{
				g_pCreateBitmapFromHBITMAP = &((Vtbl::IWICImagingFactory2*)g_wicImageFactory)->lpVtbl->CreateBitmapFromHBITMAP;
				g_funCreateBitmapFromHBITMAP = ((Vtbl::IWICImagingFactory2*)g_wicImageFactory)->lpVtbl->CreateBitmapFromHBITMAP;
			}

			if (g_pCreateBitmapFromHBITMAP)
				WriteMemory(g_pCreateBitmapFromHBITMAP, [&] { *(PVOID*)g_pCreateBitmapFromHBITMAP = MyCreateBitmapFromHBITMAP; });
		}

		if(os::buildNumber <= 22621)
		{
			HMODULE hModule = GetModuleHandleW(L"user32.dll");
			g_funFillRect = (decltype(g_funFillRect))GetProcAddress(hModule, "FillRect");
			g_funDrawTextW = (decltype(g_funDrawTextW))GetProcAddress(hModule, "DrawTextW");
			WriteIAT(GetModuleHandleW(L"udwm.dll"), "User32.dll", {{ "FillRect", MyFillRect }, { "DrawTextW", MyDrawTextW }});
		}

		return true;
	}
	catch(...)
	{
		return false;
	}

	void Shutdown()
	{
		if (!g_startup) return;

		if (os::buildNumber >= 22000)
		{
			g_funCTopLevelWindow_UpdateText.Detach();
			g_funCRenderingTechnique_ExecuteBlur.Detach();
		}
		if (os::buildNumber < 22000)
		{
			g_funCTopLevelWindow_UpdateWindowVisuals.Detach();
			g_funCAccent_UpdateAccentPolicy.Detach();
			g_funCAccent_UpdateAccentBlurRect.Detach();
			g_funCCustomBlur_DetermineOutputScale.Detach();
			g_funCCustomBlur_BuildEffect.Detach();
			g_funCD2DContext_FillEffect.Detach();
			g_funCRenderData_DrawSolidColorRectangle.Detach();
		}

		g_funCGlassColorizationParameters_AdjustWindowColorization.Detach();
		g_funCTopLevelWindow_ValidateVisual.Detach();

		if(g_pCreateBitmapFromHBITMAP)
			WriteMemory(g_pCreateBitmapFromHBITMAP, [&] {*(PVOID*)g_pCreateBitmapFromHBITMAP = g_funCreateBitmapFromHBITMAP; });

		if(os::buildNumber <= 22621)
		{
			WriteIAT(GetModuleHandleW(L"udwm.dll"), "User32.dll", {{ "DrawTextW", g_funDrawTextW } , { "FillRect", g_funFillRect }});
		}

		g_startup = false;
	}

	void Refresh()
	{
		auto path = Utils::GetCurrentDir() + L"\\data\\config.ini";
		auto ret = Utils::GetIniString(path, L"config", L"blurAmount");

		g_configData.applyglobal = Utils::GetIniString(path, L"config", L"applyglobal") == L"true";

		if (!ret.empty())
			g_configData.blurAmount = (float)std::clamp(_wtof(ret.data()), 0.0, 50.0);

		ret = Utils::GetIniString(path, L"config", L"activeTextColor");
		if (!ret.empty())
		{
			g_configData.activeTextColor = (COLORREF)_wtoll(ret.data());
			g_configData.activeTextColor = (g_configData.activeTextColor & 0x00FFFFFF) | 0xFF000000;
		}

		ret = Utils::GetIniString(path, L"config", L"inactiveTextColor");
		if (!ret.empty())
		{
			g_configData.inactiveTextColor = (COLORREF)_wtoll(ret.data());
			g_configData.inactiveTextColor = (g_configData.inactiveTextColor & 0x00FFFFFF) | 0xFF000000;
		}

		ret = Utils::GetIniString(path, L"config", L"titlebarBlendColor");
		if (!ret.empty())
			g_configData.titleBarBlendColor = (COLORREF)_wtoll(ret.data());
	}
	
	HRESULT __stdcall CRenderData_DrawSolidColorRectangle(
		CRenderData* This, CDrawingContext* a2, CDrawListEntryBuilder* a3,
		bool a4, const MilRectF* a5, D3DCOLORVALUE* a6)
	{
		a6->a = float(UINT(g_configData.titleBarBlendColor >> 24) & 0xff) / 255.f;
		return g_funCRenderData_DrawSolidColorRectangle.call_org(This, a2, a3, a4, a5, a6);
	}

	DWORD64 __stdcall CRenderingTechnique_ExecuteBlur(
		CRenderingTechnique* This, CDrawingContext* a2, const EffectInput* a3,
		const D2D_VECTOR_2F* a4, EffectInput* a5)
	{
		auto blurVector = *a4;
		blurVector.x = blurVector.y = (g_configData.blurAmount / 2.f) / 10.f;
		return g_funCRenderingTechnique_ExecuteBlur.call_org(This, a2, a3, &blurVector, a5);
	}

	DWORD64 __stdcall CD2DContext_FillEffect(
		ID2D1DeviceContext** This,
		const ID2DContextOwner* a2,
		ID2D1Effect* inputEffect,
		const D2D_RECT_F* srcRect,
		const D2D_POINT_2F* dstPoint,
		D2D1_INTERPOLATION_MODE interMode,
		D2D1_COMPOSITE_MODE mode)
	{
		const UINT32 count = inputEffect->GetPropertyCount();
		for (UINT i = 0; i < count; i++)
		{
			wchar_t name[256];
			inputEffect->GetPropertyName(i, name, 256);

			if (wcscmp(name, L"StandardDeviation") != 0)
				continue;

			float dv = 0.f;
			inputEffect->GetValue(i, &dv);

			//HostBackdrop 
			if (dv != 3.f)
			{
				return g_funCD2DContext_FillEffect.call_org(This, a2, inputEffect, srcRect, dstPoint, interMode, mode);
			}

			//BlurBehind
			inputEffect->SetValue(D2D1_DIRECTIONALBLUR_PROP_STANDARD_DEVIATION, g_configData.blurAmount);

			auto ret = S_OK;

			//无论如何 哪怕是0 都会有一点点的模糊 所以如果要透明 可以不绘制
			if(g_configData.blurAmount != 0.f)
				g_funCD2DContext_FillEffect.call_org(This, a2, inputEffect, srcRect, dstPoint, interMode, mode);

			inputEffect->SetValue(D2D1_DIRECTIONALBLUR_PROP_STANDARD_DEVIATION, 3.f);
			return ret;
		}

		return g_funCD2DContext_FillEffect.call_org(This, a2, inputEffect, srcRect, dstPoint, interMode, mode);
	}

	DWORD64 __stdcall CCustomBlur_BuildEffect(
		CCustomBlur* This,
		ID2D1Image* backdropImage,
		const D2D_RECT_F* srcRect,
		const D2D_SIZE_F* kSize,
		D2D1_GAUSSIANBLUR_OPTIMIZATION a5,
		const D2D_VECTOR_2F* a6,
		D2D_VECTOR_2F* a7)
	{
		//ID2D1Effect* cropEffect = *((ID2D1Effect**)This + 3);
		//ID2D1Effect* scaleEffect = *((ID2D1Effect**)This + 5);
		ID2D1Effect* directionalBlurKernelEffectX = *((ID2D1Effect**)This + 6);
		ID2D1Effect* directionalBlurKernelEffectY = *((ID2D1Effect**)This + 7);

		auto ret = g_funCCustomBlur_BuildEffect.call_org(This, backdropImage, srcRect, kSize, a5, a6, a7);

		directionalBlurKernelEffectX->SetValue(0, g_configData.blurAmount);
		directionalBlurKernelEffectY->SetValue(0, g_configData.blurAmount);

		return ret;
	}

	float __stdcall CCustomBlur_DetermineOutputScale(
		float a1,
		float a2, 
		D2D1_GAUSSIANBLUR_OPTIMIZATION a3)
	{
		//hostBackdrop画刷会预先使用矩阵缩放为2.5倍 再利用BuildEffect中的D2D Scale效果缩放回去 然后利用2次自定义高斯内核模糊 达到快速高模糊度计算
		//这里设置为1 则不缩放 否则BuildEffect中的图像会非常模糊 马赛克一样
		return 1.f;
	}

	int __stdcall MyFillRect(
		HDC hdc, 
		LPRECT rect,
		HBRUSH hbrush)
	{
		return g_funFillRect(hdc, rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}

	int MyDrawTextW(
		HDC hdc,
		LPCWSTR lpchText, 
		int cchText,
		LPRECT lprc,
		UINT format)
	{
		SetBkMode(hdc, TRANSPARENT);
		int result = 0;

		if ((format & DT_CALCRECT) || (format & DT_INTERNAL) || (format & DT_NOCLIP))
			return g_funDrawTextW(hdc, lpchText, cchText, lprc, format);

		if (FAILED(DrawTextWithAlpha(
			hdc,
			lpchText,
			cchText,
			lprc,
			format,
			result
		)))
		{
			return g_funDrawTextW(hdc, lpchText, cchText, lprc, format);
		}
		return result;
	}

	HRESULT __stdcall MyCreateBitmapFromHBITMAP(
		IWICImagingFactory2* This,
		HBITMAP hBitmap,
		HPALETTE hPalette,
		WICBitmapAlphaChannelOption options,
		IWICBitmap** ppIBitmap)
	{
		options = WICBitmapUsePremultipliedAlpha;
		return g_funCreateBitmapFromHBITMAP(This, hBitmap, hPalette, options, ppIBitmap);
	}

	void __stdcall CAccent_UpdateAccentBlurRect(
		CAccent* This,
		const RECT* a2)
	{
		g_accentThis = This;
		g_blurRegion = *a2;
		return g_funCAccent_UpdateAccentBlurRect.call_org(This, a2);
	}

	HRESULT __stdcall CAccent_UpdateAccentPolicy(
		CAccent* This,
		RECT* a2,
		ACCENT_POLICY* a3, 
		CBaseGeometryProxy* a4)
	{
		thread_local auto redirected{ false };
		thread_local RECT windowRect = { -1 };

		if (!redirected)
		{
			g_funCWindowList_GetExtendedFrameBounds(g_windowList, g_window, &windowRect);

			redirected = true;
			HRESULT hr = g_funCAccent_UpdateAccentPolicy.call_org(This, a2, a3, a4);

			if (!IsZoomed(g_window) && !IsIconic(g_window))
			{
				OffsetRect(&windowRect, -windowRect.left + (windowRect.left - a2->left), -windowRect.top);

				if (g_accentThis == This)
				{
					IntersectRect(&windowRect, &g_blurRegion, &windowRect);
					g_accentThis = nullptr;
					g_blurRegion = { 0, 0, -1, -1 };
				}
				g_funCAccent_UpdateAccentBlurRect.call_org(This, &windowRect);
			}
			else
			{
				windowRect = { -1 };
				if (g_accentThis == This)
				{
					g_accentThis = nullptr;
					windowRect = g_blurRegion;
				}
				g_funCAccent_UpdateAccentBlurRect.call_org(This, &windowRect);
			}
			redirected = false;
			g_window = nullptr;

			return hr;
		}

		return g_funCAccent_UpdateAccentPolicy.call_org(This, a2, a3, a4);
	}

	HRESULT __stdcall CTopLevelWindow_ValidateVisual(CTopLevelWindow* This)
	{
		PVOID windowData;
		ACCENT_POLICY* accent_policy;

		if (os::buildNumber >= 22000)
		{
			windowData = *((PVOID*)This + 94);
			accent_policy = (ACCENT_POLICY*)((ULONGLONG)windowData + 168);
		}
		else
		{
			windowData = *((PVOID*)This + 91);
			accent_policy = (ACCENT_POLICY*)((ULONGLONG)windowData + 152);
		}

		bool needUpdate{ false };
		if (windowData)
		{
			HWND hWnd = *((HWND*)windowData + 5);
			RECT* accentRect = (RECT*)((ULONGLONG)windowData + 48);
			g_window = hWnd;

			//扩展到非客户区的窗口才有效
			BOOL enableNC = FALSE;
			DwmGetWindowAttribute(hWnd, DWMWA_NCRENDERING_ENABLED, &enableNC, sizeof enableNC);

			//忽略已有Backdrop效果的窗口
			DWM_SYSTEMBACKDROP_TYPE type = DWMSBT_NONE;
			DwmGetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &type, sizeof type);

			if (g_configData.applyglobal && (type == DWMSBT_MAINWINDOW || type == DWMSBT_TABBEDWINDOW))
				type = DWMSBT_NONE;

			if (enableNC && (type == DWMSBT_NONE || type == DWMSBT_AUTO))
			{
				if (accent_policy->nAccentState == 0)
				{
					accent_policy->nAccentState = 3;
					accent_policy->nFlags = 3584;
					needUpdate = true;
				}
				//对于HostBackdrop 启用之后就可以更改flag了 否则会造成Backdrop不刷新的问题
				else if (accent_policy->nAccentState == 5)
				{
					needUpdate = true;
					accent_policy->nAccentState = 3;
					accent_policy->nFlags = 3584;
				}
			}
		}

		HRESULT hr = g_funCTopLevelWindow_ValidateVisual.call_org(This);
		//手动通知
		if (needUpdate)
		{
			g_funCTopLevelWindow_OnAccentPolicyUpdated(This);
		}

		g_window = nullptr;
		return hr;
	}

	DWORD64 __stdcall CTopLevelWindow_UpdateWindowVisuals(CTopLevelWindow* This)
	{
		auto frame = g_funCTopLevelWindow_s_ChooseWindowFrameFromStyle
		(
			(char)*((DWORD*)This + 148),
			0,
			(char)(*(BYTE*)(*((ULONG64*)This + 91) + 611) & 0x20) != 0
		);

		//对于Ribbon或扩展到NC区域的窗口 我们无法处理标题文本颜色 需要过滤掉 它应该由应用程序处理
		if (frame && (*((BYTE*)This + 592) & 8) != 0)
		{
			auto ret = g_funCTopLevelWindow_UpdateWindowVisuals.call_org(This);

			auto TreatAsActiveWindow = [&]
			{
				return (*((BYTE*)This + 592) & 0x40) != 0 || (*(BYTE*)(*((ULONG64*)This + 91) + 611) & 0x20) != 0;
			};

			auto text = *((CText**)This + 65);//CText This

			if (TreatAsActiveWindow())
				g_CText_SetColor(text, g_configData.activeTextColor);
			else
				g_CText_SetColor(text, g_configData.inactiveTextColor);

			return ret;
		}

		return g_funCTopLevelWindow_UpdateWindowVisuals.call_org(This);
	}

	DWORD64 __stdcall CGlassColorizationParameters_AdjustWindowColorization(
		GpCC* a1,
		float a2, 
		TMILFlagsEnum<ColorizationFlags> a3)
	{
		auto ret = g_funCGlassColorizationParameters_AdjustWindowColorization.call_org(a1, a2, a3);
		if (a3 == Color_TitleBackground)
		{
			a1->a = (g_configData.titleBarBlendColor >> 24) & 0xff;
			a1->r = GetRValue(g_configData.titleBarBlendColor);
			a1->g = GetGValue(g_configData.titleBarBlendColor);
			a1->b = GetBValue(g_configData.titleBarBlendColor);
		}
		return ret;
	}

	DWORD64 __stdcall CTopLevelWindow_UpdateText(
		CTopLevelWindow* This,
		CTopLevelWindow_WindowFrame* a2,
		double a3)
	{
		if (a2 && (*((BYTE*)This + 624) & 8) != 0)
		{
			auto ret = g_funCTopLevelWindow_UpdateText.call_org(This, a2, a3);

			auto text = (CDWriteText*)*((ULONG64*)This + 71);//CDWriteText This
			typedef void(__fastcall**** CDWriteText_SetColor)(CDWriteText*, UINT);//(***((void(__fastcall****)(ULONG64, __int64))This + 71))(*((ULONG64*)This + 71), inputVec);
			auto pfunCDWriteText_SetColor = ***((CDWriteText_SetColor)This + 71);//CDWriteText::SetColor(CDWriteText *this, int a2)

			auto CTopLevelWindow_TreatAsActiveWindow = [&]() -> bool
			{
				return (*((BYTE*)This + 624) & 0x40) != 0 || (*(BYTE*)(*((ULONG64*)This + 94) + 675) & 0x10) != 0;
			};

			if (CTopLevelWindow_TreatAsActiveWindow())
				pfunCDWriteText_SetColor(text, g_configData.activeTextColor);//Active
			else
				pfunCDWriteText_SetColor(text, g_configData.inactiveTextColor);//Inactive

			return ret;
		}

		return g_funCTopLevelWindow_UpdateText.call_org(This, a2, a3);
	}


}