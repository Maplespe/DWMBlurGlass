/**
 * FileName: CustomBackdropEffect.cpp
 *
 * Copyright (C) 2024 Maplespe
 *
 * This file is part of MToolBox and DWMBlurGlass and AcrylicEverywhere.
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
#include "CustomBackdropEffect.h"
#include "BackdropMaterials.h"
#include "HookDef.h"
#include "CommonDef.h"
#include "wil.h"
#include "AccentBlurEffect.h"

namespace MDWMBlurGlassExt::CustomBackdrop
{
	using namespace CommonDef;
	using namespace MDWMBlurGlass;
	using namespace DWM;

	std::atomic_bool g_startup = false;

	CTopLevelWindow* g_windowOfInterest = nullptr;
	CRgnGeometryProxy* const* g_borderGeometry = nullptr;
	wil::unique_hrgn g_borderRgn = nullptr;
	CRgnGeometryProxy* const* g_titlebarGeometry = nullptr;
	wil::unique_hrgn g_titlebarRgn = nullptr;
	CBackdropManager g_backdropManager{};

	MinHook g_funCTopLevelWindow_InitializeVisualTreeClone
	{
		"CTopLevelWindow::InitializeVisualTreeClone",
		CTopLevelWindow_InitializeVisualTreeClone
	};

	MinHook g_funCTopLevelWindow_UpdateNCAreaBackground
	{
		"CTopLevelWindow::UpdateNCAreaBackground",
		CTopLevelWindow_UpdateNCAreaBackground
	};

	MinHook g_funCTopLevelWindow_Destructor
	{
		"CTopLevelWindow::~CTopLevelWindow",
		CTopLevelWindow_Destructor
	};

	MinHook g_funResourceHelper_CreateGeometryFromHRGN
	{
		"ResourceHelper::CreateGeometryFromHRGN",
		ResourceHelper_CreateGeometryFromHRGN
	};

	void Attach()
	{
		if (g_startup) return;
		g_startup = true;

		g_funCTopLevelWindow_Destructor.Attach();
		g_funCTopLevelWindow_InitializeVisualTreeClone.Attach();
		g_funCTopLevelWindow_UpdateNCAreaBackground.Attach();
		g_funResourceHelper_CreateGeometryFromHRGN.Attach();
		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<2, true>();

	}

	void Detach()
	{
		if (!g_startup) return;

		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<2, false>();
		g_funResourceHelper_CreateGeometryFromHRGN.Detach();
		g_funCTopLevelWindow_UpdateNCAreaBackground.Detach();
		g_funCTopLevelWindow_InitializeVisualTreeClone.Detach();
		g_funCTopLevelWindow_Destructor.Detach();

		g_backdropManager.Shutdown();

		g_startup = false;
	}

	void Refresh()
	{
		if (g_startup)
			g_backdropManager.RefreshEffectConfig();
		if (g_configData.blurmethod == blurMethod::CustomBlur && !g_startup)
			Attach();
		else if (g_configData.blurmethod != blurMethod::CustomBlur && g_startup)
			Detach();
	}

	HRESULT CTopLevelWindow_InitializeVisualTreeClone(CTopLevelWindow* This,
		CTopLevelWindow* topLevelWindow, UINT cloneOptions)
	{
		bool cloneAllowed{ false };
		auto backdrop{ g_backdropManager.GetOrCreateBackdrop(This) };
		if (backdrop)
		{
			cloneAllowed = backdrop->GetVisual()->AllowVisualTreeClone(false);
		}

		HRESULT hr = g_funCTopLevelWindow_InitializeVisualTreeClone.call_org(This, topLevelWindow, cloneOptions);

		if (backdrop)
		{
			backdrop->GetVisual()->AllowVisualTreeClone(cloneAllowed);
			if (SUCCEEDED(hr))
			{
				auto clonedBackdrop{ std::make_shared<CCompositedBackdrop>(g_configData.effectType, g_configData.reflection) };
				backdrop->InitializeVisualTreeClone(clonedBackdrop.get());
				g_backdropManager.CreateWithGivenBackdrop(topLevelWindow, clonedBackdrop);
			}
		}

		return hr;
	}

	HRESULT CTopLevelWindow_UpdateNCAreaBackground(CTopLevelWindow* This)
	{
		g_windowOfInterest = This;
		g_borderGeometry = &This->GetBorderGeometry();
		g_titlebarGeometry = &This->GetTitlebarGeometry();

		HRESULT hr = g_funCTopLevelWindow_UpdateNCAreaBackground.call_org(This);

		g_windowOfInterest = nullptr;
		g_borderGeometry = nullptr;
		g_titlebarGeometry = nullptr;

		std::shared_ptr<CCompositedBackdrop> backdrop{ nullptr };
		CWindowData* windowData{ This->GetData() };
		bool policyActive = false;
		if (windowData)
		{
			if (auto policy = windowData->GetAccentPolicy(); policy->nAccentState == 5)
				policyActive = false;
			else
				policyActive = CAccent::s_IsPolicyActive(policy);
		}
		if (windowData && !policyActive && AccentBlur::CheckWindowType(windowData->GetHWND())
			&& This->HasNonClientBackground() && (backdrop = g_backdropManager.GetOrCreateBackdrop(This, true)))
		{
			if (g_borderRgn && g_titlebarRgn)
			{
				wil::unique_hrgn compositedRgn{ CreateRectRgn(0, 0, 0, 0) };

				CombineRgn(
					compositedRgn.get(),
					g_borderRgn.get(),
					g_titlebarRgn.get(),
					RGN_OR
				);
				backdrop->Update(
					This,
					This->IsSystemBackdropApplied() ? nullptr : compositedRgn.get()
				);
			}
			else
			{
				backdrop->Update(
					This,
					This->IsSystemBackdropApplied() ? nullptr : backdrop->GetClipRegion()
				);
			}
			This->ShowNCBackgroundVisualList(false);
		}
		else if (g_backdropManager.GetOrCreateBackdrop(This))
		{
			g_backdropManager.Remove(This);
		}

		g_borderRgn.reset();
		g_titlebarRgn.reset();

		return hr;
	}

	HRESULT CTopLevelWindow_ValidateVisual(CTopLevelWindow* This)
	{
		std::shared_ptr<CCompositedBackdrop> backdrop{ nullptr };
		CGlassReflectionBackdrop* glassReflection{ nullptr };
		CWindowData* windowData{ This->GetData() };
		bool policyActive = false;
		if (windowData)
		{
			g_window = windowData->GetHWND();
			if (auto policy = windowData->GetAccentPolicy(); policy->nAccentState == 5)
				policyActive = false;
			else
				policyActive = CAccent::s_IsPolicyActive(policy);
		}
		if (windowData && AccentBlur::CheckWindowType(windowData->GetHWND()) && 
			!policyActive && (backdrop = g_backdropManager.GetOrCreateBackdrop(This))
			&& (glassReflection = backdrop->GetGlassReflection()))
		{
			glassReflection->UpdateBackdrop(This);
		}
		return S_OK;
	}

	void CTopLevelWindow_Destructor(CTopLevelWindow* This)
	{
		g_backdropManager.Remove(This);
		g_funCTopLevelWindow_Destructor.call_org(This);
	}

	HRESULT ResourceHelper_CreateGeometryFromHRGN(HRGN hrgn, CRgnGeometryProxy** geometry)
	{
		if (g_windowOfInterest)
		{
			if (geometry == g_borderGeometry)
			{
				if (!g_borderRgn)
				{
					g_borderRgn.reset(CreateRectRgn(0, 0, 0, 0));
				}
				CombineRgn(g_borderRgn.get(), hrgn, nullptr, RGN_COPY);
			}
			if (geometry == g_titlebarGeometry)
			{
				if (!g_titlebarRgn)
				{
					g_titlebarRgn.reset(CreateRectRgn(0, 0, 0, 0));
				}
				CombineRgn(g_titlebarRgn.get(), hrgn, nullptr, RGN_COPY);
			}
		}
		return g_funResourceHelper_CreateGeometryFromHRGN.call_org(hrgn, geometry);
	}
}
