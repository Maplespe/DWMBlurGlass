/**
 * FileName: AccentBlurEffect.cpp
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
#include "AccentBlurEffect.h"
#include "../Common/CommonDef.h"

namespace MDWMBlurGlassExt::AccentBlur
{
	using namespace MDWMBlurGlass;
	using namespace CommonDef;
	using namespace DWM;

	std::atomic_bool g_startup = false;

	thread_local MilRectF g_dstRect = { 0 };
	thread_local RECT g_blurRegion{ 0, 0, -1, -1 };
	thread_local CAccent* g_accentThis = nullptr;
	CWindowData* g_windowData = nullptr;

	com_ptr<ID2D1Bitmap1> g_aeroPeekBmp = nullptr;
	com_ptr<ID2D1Effect> g_scaleEffect = nullptr;

	//Aero reflection
	MinHook g_funCFilterEffect_CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive
	{
		"CFilterEffect::CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive", CFilterEffect_CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive
	};

	//BlendColor
	MinHook g_funCSolidColorLegacyMilBrushProxy_Update
	{
		"CSolidColorLegacyMilBrushProxy::Update", CSolidColorLegacyMilBrushProxy_Update
	};

	MinHook g_funCGlassColorizationParameters_AdjustWindowColorization_Win10
	{
		"CGlassColorizationParameters::AdjustWindowColorization", CGlassColorizationParameters_AdjustWindowColorization_Win10
	};

	MinHook g_funCGlassColorizationParameters_AdjustWindowColorization_Win11
	{
		"CGlassColorizationParameters::AdjustWindowColorization", CGlassColorizationParameters_AdjustWindowColorization_Win11
	};

	MinHook g_funCAccent_UpdateAccentBlurRect
	{
		"CAccent::UpdateAccentBlurRect", CAccent_UpdateAccentBlurRect
	};

	MinHook g_funCAccent_UpdateAccentPolicy
	{
		"CAccent::UpdateAccentPolicy", CAccent_UpdateAccentPolicy
	};

	MinHook g_funCTopLevelWindow_OnAccentPolicyUpdated
	{
		"CTopLevelWindow::OnAccentPolicyUpdated", CTopLevelWindow_OnAccentPolicyUpdated
	};

	void Attach()
	{
		if (g_startup) return;
		g_startup = true;

		if (os::buildNumber < 22000)
		{
			g_funCFilterEffect_CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive.Attach();
			g_CD2DContext_FillEffect_HookDispatcher.enable_hook_routine<1, true>();
			g_funCSolidColorLegacyMilBrushProxy_Update.Attach();
			g_funCGlassColorizationParameters_AdjustWindowColorization_Win10.Attach();
			g_funCAccent_UpdateAccentBlurRect.Attach();
			g_funCAccent_UpdateAccentPolicy.Attach();
		}
		else if(os::buildNumber >= 22000)
		{
			g_funCGlassColorizationParameters_AdjustWindowColorization_Win11.Attach();
		}
		g_funCTopLevelWindow_OnAccentPolicyUpdated.Attach();
		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<0, true>();

	}

	void Detach()
	{
		if (!g_startup) return;

		if (os::buildNumber < 22000)
		{
			g_funCAccent_UpdateAccentPolicy.Detach();
			g_funCAccent_UpdateAccentBlurRect.Detach();
			g_funCGlassColorizationParameters_AdjustWindowColorization_Win10.Detach();
			g_funCSolidColorLegacyMilBrushProxy_Update.Detach();
			g_CD2DContext_FillEffect_HookDispatcher.enable_hook_routine<1, false>();
			g_funCFilterEffect_CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive.Detach();
		}
		else if (os::buildNumber >= 22000)
		{
			g_funCGlassColorizationParameters_AdjustWindowColorization_Win11.Detach();
		}
		g_funCTopLevelWindow_OnAccentPolicyUpdated.Detach();
		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<0, false>();

		if (g_scaleEffect)
			g_scaleEffect = nullptr;
		if (g_aeroPeekBmp)
			g_aeroPeekBmp = nullptr;

		g_startup = false;
	}

	void Refresh()
	{
		if (g_scaleEffect)
			g_scaleEffect = nullptr;
		if (g_aeroPeekBmp)
			g_aeroPeekBmp = nullptr;

		if (g_configData.blurmethod == blurMethod::AccentBlur && !g_startup && !g_configData.powerSavingMode)
			Attach();
		else if ((g_configData.blurmethod != blurMethod::AccentBlur || g_configData.powerSavingMode) && g_startup)
			Detach();
	}

	bool CheckWindowType(HWND hWnd)
	{
		//启用NC渲染的窗口才有效
		BOOL enableNC = FALSE;
		DwmGetWindowAttribute(hWnd, DWMWA_NCRENDERING_ENABLED, &enableNC, sizeof enableNC);

		//忽略已有Backdrop效果的窗口
		DWM_SYSTEMBACKDROP_TYPE type = DWMSBT_NONE;
		if (os::buildNumber >= 22621)
			DwmGetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &type, sizeof type);

		if (g_configData.applyglobal && (type == DWMSBT_MAINWINDOW || type == DWMSBT_TABBEDWINDOW))
			type = DWMSBT_NONE;

		return enableNC && (type == DWMSBT_NONE || type == DWMSBT_AUTO);
	}

	DWORD64 CFilterEffect_CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive(
		CFilterEffect* This,
		MilRectF* a2,
		MilRectF* a3,
		MilRectF* a4,
		MilRectF* a5,
		MilRectF* a6)
	{
		g_dstRect = *a4;
		return g_funCFilterEffect_CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive.call_org(This, a2, a3, a4, a5, a6);
	}

	DWORD64 CD2DContext_FillEffect(
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

			if (!g_aeroPeekBmp && g_configData.reflection)
			{
				g_aeroPeekBmp = CreateD2DBitmap(CDesktopManager::s_pDesktopManagerInstance->GetWICFactory(), This[30], Utils::GetCurrentDir() + L"\\data\\AeroPeek.png");
				if (g_aeroPeekBmp)
					This[30]->CreateEffect(CLSID_D2D1Scale, g_scaleEffect.put());
			}
			if (g_aeroPeekBmp)
			{
				g_scaleEffect->SetInput(0, g_aeroPeekBmp.get());

				auto size = g_aeroPeekBmp->GetSize();

				std::vector<RECT> monitorList;
				EnumMonitors(monitorList);

				MilRectF scaleRect = g_dstRect;
				D2D1_SIZE_F dstSize = { scaleRect.right - scaleRect.left, scaleRect.bottom - scaleRect.top };
				scaleRect.left = -scaleRect.left;
				scaleRect.top = -scaleRect.top;
				scaleRect.right = scaleRect.left + dstSize.width;
				scaleRect.bottom = scaleRect.top + dstSize.height;

				if (scaleRect.left < 0)
					scaleRect.left = 0.f;
				if (scaleRect.top < 0)
					scaleRect.top = 0.f;

				scaleRect.left *= 2.f;
				scaleRect.top *= 2.f;
				scaleRect.right *= 2.f;
				scaleRect.bottom *= 2.f;

				//查找目标矩形在哪个显示器
				SIZE monitorSize = { 0 };
				for (auto& rect : monitorList)
				{
					RECT _rect{ (int)scaleRect.left, (int)scaleRect.top, (int)scaleRect.right, (int)scaleRect.bottom };
					if (!IsRectInside(rect, _rect))
						continue;
					monitorSize.cx = rect.right - rect.left;
					monitorSize.cy = rect.bottom - rect.top;
					scaleRect.left -= (float)rect.left;
					scaleRect.top -= (float)rect.top;
					scaleRect.right = scaleRect.left + (float)monitorSize.cx;
					scaleRect.bottom = scaleRect.top + (float)monitorSize.cy;
					break;
				}

				scaleRect.left *= 0.5f;
				scaleRect.top *= 0.5f;
				scaleRect.right *= 0.5f;
				scaleRect.bottom *= 0.5f;

				float scaleX;
				float scaleY;
				if ((float)monitorSize.cx > size.width)
					scaleX = (float)monitorSize.cx / size.width;
				else
					scaleX = 1.f / (size.width / (float)monitorSize.cx / 1.f);

				if ((float)monitorSize.cy > size.height)
					scaleY = (float)monitorSize.cy / size.height;
				else
					scaleY = 1.f / (size.height / (float)monitorSize.cy);

				g_scaleEffect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(scaleX * 0.5f, scaleY * 0.5f));

				This[30]->DrawImage(g_scaleEffect.get(), nullptr, (D2D1_RECT_F*)&scaleRect);

				return S_OK;
			}
		}
		return S_OK;
	}

	DWORD64 CSolidColorLegacyMilBrushProxy_Update(
		CSolidColorLegacyMilBrushProxy* This, double a2,
		const D3DCOLORVALUE* a3)
	{
		auto color = *a3;

		bool isLight = Utils::IsAppUseLightMode();
		if (GetForegroundWindow() == g_window)
			color.a = float(((isLight ? g_configData.activeBlendColor : g_configData.activeBlendColorDark) >> 24) & 0xff) / 255.f;
		else
			color.a = float(((isLight ? g_configData.inactiveBlendColor : g_configData.inactiveBlendColorDark) >> 24) & 0xff) / 255.f;
		return g_funCSolidColorLegacyMilBrushProxy_Update.call_org(This, a2, &color);
	}

	DWORD64 CGlassColorizationParameters_AdjustWindowColorization_Win10(
		GpCC* a1,
		GpCC* a2,
		float a3,
		char a4)
	{
		auto ret = g_funCGlassColorizationParameters_AdjustWindowColorization_Win10.call_org(a1, a2, a3, a4);

		COLORREF color;
		bool isLight = true;
		if (g_windowData)
			isLight = !g_windowData->IsUsingDarkMode();
		if(GetForegroundWindow() == g_window)
			color = isLight ? g_configData.activeBlendColor : g_configData.activeBlendColorDark;
		else
			color = isLight ? g_configData.inactiveBlendColor : g_configData.inactiveBlendColorDark;

		if (g_configData.useAccentColor)
		{
			color = g_accentColor;
		}

		a1->a = 255;
		a1->r = GetRValue(color);
		a1->g = GetGValue(color);
		a1->b = GetBValue(color);
		return ret;
	}

	DWORD64 CGlassColorizationParameters_AdjustWindowColorization_Win11(
		GpCC* a1,
		GpCC* a2,
		float a3,
		short a4)
	{
		auto ret = g_funCGlassColorizationParameters_AdjustWindowColorization_Win11.call_org(a1, a2, a3, a4);

		COLORREF color;
		bool isLight = true;
		if (g_windowData)
			isLight = !g_windowData->IsUsingDarkMode();
		if (GetForegroundWindow() == g_window)
			color = isLight ? g_configData.activeBlendColor : g_configData.activeBlendColorDark;
		else
			color = isLight ? g_configData.inactiveBlendColor : g_configData.inactiveBlendColorDark;

		a1->a = (color >> 24) & 0xff;

		if (g_configData.useAccentColor)
		{
			color = g_accentColor;
		}

		a1->r = GetRValue(color);
		a1->g = GetGValue(color);
		a1->b = GetBValue(color);

		return ret;
	}

	void CAccent_UpdateAccentBlurRect(
		CAccent* This,
		const RECT* a2)
	{
		g_accentThis = This;
		g_blurRegion = *a2;
		return g_funCAccent_UpdateAccentBlurRect.call_org(This, a2);
	}

	HRESULT CAccent_UpdateAccentPolicy(
		CAccent* This,
		RECT* a2,
		ACCENT_POLICY* a3,
		CBaseGeometryProxy* a4)
	{
		thread_local auto redirected{ false };
		thread_local RECT windowRect = { -1 };

		if (!redirected && GetWindowLongPtrW(g_window, GWL_STYLE) & WS_CAPTION)
		{
			g_windowList->GetExtendedFrameBounds(g_window, &windowRect);

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

	void CTopLevelWindow_OnAccentPolicyUpdated(CTopLevelWindow* This)
	{
		if (const auto windowData = This->GetData())
		{
			const HWND hWnd = windowData->GetHWND();
			const auto accentPolicy = windowData->GetAccentPolicy();

			if (!hWnd || !accentPolicy)
				return g_funCTopLevelWindow_OnAccentPolicyUpdated.call_org(This);

			if (CheckWindowType(hWnd) && This->HasNonClientBackground() &&
				(accentPolicy->nAccentState == ACCENT_DISABLED
					|| accentPolicy->nAccentState == ACCENT_ENABLE_HOSTBACKDROP)//对于HostBackdrop 启用之后就可以更改flag了 否则会造成Backdrop不刷新的问题
				)
			{
				accentPolicy->nAccentState = ACCENT_ENABLE_BLURBEHIND;
				accentPolicy->nFlags = 3584;
			}
		}
		g_funCTopLevelWindow_OnAccentPolicyUpdated.call_org(This);
	}

	HRESULT CTopLevelWindow_ValidateVisual(CTopLevelWindow* This)
	{
		if (const auto windowData = This->GetData())
		{
			const HWND hWnd = windowData->GetHWND();
			const auto accentPolicy = windowData->GetAccentPolicy();

			g_window = hWnd;
			g_windowData = windowData;

			if (os::buildNumber < 22000 && CheckWindowType(hWnd) && This->HasNonClientBackground() && accentPolicy &&
				(accentPolicy->nAccentState == ACCENT_DISABLED || accentPolicy->nAccentState == ACCENT_ENABLE_BLURBEHIND))
			{
				auto collection = This->GetNCAreaVisualCollection();
				auto visual1 = This->GetLegacyVisual();
				auto visual2 = This->GetClientBlurVisual();

				if (visual1 && visual2)
				{
					collection->Remove(visual2);
					collection->InsertRelative(visual2, visual1, false, true);
				}
			}
		}
		return S_OK;
	}
}
