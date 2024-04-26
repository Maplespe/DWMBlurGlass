/**
 * FileName: CustomBackdropEffect.cpp
 *
 * Copyright (C) 2024 Maplespe
 *
 * This file is part of MToolBox and DWMBlurGlass and MDWMBlurGlassExt.
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
#include "VisualManager.hpp"
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
	CVisualManager g_visualManager{};

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

	MinHook g_funCTopLevelWindow_OnClipUpdated
	{
		"CTopLevelWindow::OnClipUpdated",
		CTopLevelWindow_OnClipUpdated
	};
	MinHook g_funCTopLevelWindow_OnAccentPolicyUpdated
	{
		"CTopLevelWindow::OnAccentPolicyUpdated",
		CTopLevelWindow_OnAccentPolicyUpdated
	};
	MinHook g_funCWindowList_UpdateAccentBlurRect
	{
		"CWindowList::UpdateAccentBlurRect",
		CWindowList_UpdateAccentBlurRect
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

		// accent overrider
		g_funCTopLevelWindow_OnClipUpdated.Attach();
		g_funCTopLevelWindow_OnAccentPolicyUpdated.Attach();
		g_funCWindowList_UpdateAccentBlurRect.Attach();
	}

	void Detach()
	{
		if (!g_startup) return;

		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<2, false>();
		g_funResourceHelper_CreateGeometryFromHRGN.Detach();
		g_funCTopLevelWindow_UpdateNCAreaBackground.Detach();
		g_funCTopLevelWindow_InitializeVisualTreeClone.Detach();
		g_funCTopLevelWindow_Destructor.Detach();

		// accent overrider
		g_funCTopLevelWindow_OnClipUpdated.Detach();
		g_funCTopLevelWindow_OnAccentPolicyUpdated.Detach();
		g_funCWindowList_UpdateAccentBlurRect.Detach();

		g_visualManager.Shutdown();

		g_startup = false;
	}

	void Refresh()
	{
		if (g_startup)
			g_visualManager.RefreshEffectConfig();

		bool nosupport = os::buildNumber < 22000 && g_configData.effectType == effectType::Mica;

		if (g_configData.blurmethod == blurMethod::CustomBlur && !g_startup && !nosupport && !g_configData.powerSavingMode)
			Attach();
		else if ((g_configData.blurmethod != blurMethod::CustomBlur || g_configData.powerSavingMode || nosupport) && g_startup)
			Detach();
	}

	HRESULT CTopLevelWindow_InitializeVisualTreeClone(CTopLevelWindow* This,
	                                                  CTopLevelWindow* topLevelWindow, UINT cloneOptions)
	{
		HRESULT hr{ S_OK };

		hr = g_funCTopLevelWindow_InitializeVisualTreeClone.call_org(This, topLevelWindow, cloneOptions);

		if (SUCCEEDED(hr))
		{
			g_visualManager.TryCloneBackdropForWindow(This, topLevelWindow);
			g_visualManager.TryCloneAccentBackdropForWindow(This, topLevelWindow);
		}

		return hr;
	}
	HRESULT CTopLevelWindow_UpdateNCAreaBackground(CTopLevelWindow* This)
	{
		// temporary patch for round corner fix
		{
			if (This->GetData())
			{
				CommonDef::g_window = This->GetData()->GetHWND();
			}
		}
		g_windowOfInterest = This;
		g_borderGeometry = &This->GetBorderGeometry();
		g_titlebarGeometry = &This->GetTitlebarGeometry();

		HRESULT hr = g_funCTopLevelWindow_UpdateNCAreaBackground.call_org(This);

		g_windowOfInterest = nullptr;
		g_borderGeometry = nullptr;
		g_titlebarGeometry = nullptr;

		winrt::com_ptr<CCompositedBackdrop> backdrop{ nullptr };
		CWindowData* windowData{ This->GetData() };
		if (
			windowData &&
			This->HasNonClientBackground() &&
			AccentBlur::CheckWindowType(windowData->GetHWND()) &&
			!windowData->GetAccentPolicy()->IsAccentBlurEnabled() &&
			(backdrop = g_visualManager.GetOrCreateBackdrop(This, true))
			)
		{
			if (
				g_borderRgn &&
				g_titlebarRgn &&
				!This->IsSystemBackdropApplied()
			)
			{
				wil::unique_hrgn compositedRgn{ CreateRectRgn(0, 0, 0, 0) };

				CombineRgn(
					compositedRgn.get(),
					g_borderRgn.get(),
					g_titlebarRgn.get(),
					RGN_OR
				);
				backdrop->UpdateClipRegion(compositedRgn.get());
			}
			else
			{
				backdrop->UpdateClipRegion(nullptr);
			}
			backdrop->Update();
		}
		else if (g_visualManager.GetOrCreateBackdrop(This))
		{
			g_visualManager.Remove(This);
		}

		g_borderRgn.reset();
		g_titlebarRgn.reset();

		// temporary patch for round corner fix
		{
			if (CommonDef::g_window)
			{
				CommonDef::g_window = nullptr;
			}
		}

		return hr;
	}
	HRESULT CTopLevelWindow_ValidateVisual(CTopLevelWindow* This)
	{
		winrt::com_ptr<CCompositedBackdrop> backdrop{ nullptr };
		CWindowData* windowData{ This->GetData() };
		if (windowData)
		{
			{
				winrt::com_ptr<CCompositedBackdrop> backdrop{ nullptr };
				if (
					This->HasNonClientBackground() &&
					AccentBlur::CheckWindowType(windowData->GetHWND()) &&
					(backdrop = g_visualManager.GetOrCreateBackdrop(This, true))
				)
				{
					backdrop->UpdateGlassReflection();
				}
			}
			{
				winrt::com_ptr<CCompositedAccentBackdrop> backdrop{ nullptr };
				if (
					windowData->GetAccentPolicy()->IsAccentBlurEnabled() &&
					g_configData.overrideAccent &&
					(backdrop = g_visualManager.GetOrCreateAccentBackdrop(This, true))
				)
				{
					backdrop->Update();
				}
				else if (
					(
						!windowData->GetAccentPolicy()->IsAccentBlurEnabled() ||
						!g_configData.overrideAccent
					) &&
					g_visualManager.GetOrCreateAccentBackdrop(This)
				)
				{
					g_visualManager.RemoveAccent(This);
				}
			}
		}

		return S_OK;
	}
	void CTopLevelWindow_Destructor(CTopLevelWindow* This)
	{
		g_visualManager.Remove(This);
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

	HRESULT STDMETHODCALLTYPE CTopLevelWindow_OnClipUpdated(DWM::CTopLevelWindow* This)
	{
		HRESULT hr{ S_OK };

		hr = g_funCTopLevelWindow_OnClipUpdated.call_org(This);

		if (SUCCEEDED(hr))
		{
			CWindowData* windowData{ This->GetData() };
			if (windowData)
			{
				winrt::com_ptr<CCompositedAccentBackdrop> backdrop{ nullptr };
				if (
					windowData->GetAccentPolicy()->IsAccentBlurEnabled() &&
					g_configData.overrideAccent &&
					(backdrop = g_visualManager.GetOrCreateAccentBackdrop(This, true))
					)
				{
					backdrop->UpdateClipRegion();
				}
				else if (
					(
						!windowData->GetAccentPolicy()->IsAccentBlurEnabled() ||
						!g_configData.overrideAccent
					) &&
					g_visualManager.GetOrCreateAccentBackdrop(This)
				)
				{
					g_visualManager.RemoveAccent(This);
				}
			}
		}

		return hr;
	}
	HRESULT STDMETHODCALLTYPE CTopLevelWindow_OnAccentPolicyUpdated(DWM::CTopLevelWindow* This)
	{
		HRESULT hr{ S_OK };

		hr = g_funCTopLevelWindow_OnAccentPolicyUpdated.call_org(This);

		if (SUCCEEDED(hr))
		{
			CWindowData* windowData{ This->GetData() };
			if (windowData)
			{
				winrt::com_ptr<CCompositedAccentBackdrop> backdrop{ nullptr };
				if (
					windowData->GetAccentPolicy()->IsAccentBlurEnabled() &&
					g_configData.overrideAccent &&
					(backdrop = g_visualManager.GetOrCreateAccentBackdrop(This, true))
					)
				{
					backdrop->UpdateAccentPolicy();
				}
				else if (
					(
						!windowData->GetAccentPolicy()->IsAccentBlurEnabled() ||
						!g_configData.overrideAccent
						) &&
					g_visualManager.GetOrCreateAccentBackdrop(This)
					)
				{
					g_visualManager.RemoveAccent(This);
				}
			}
		}

		return hr;
	}
	HRESULT STDMETHODCALLTYPE CWindowList_UpdateAccentBlurRect(DWM::CWindowList* This, const MILCMD_DWM_REDIRECTION_ACCENTBLURRECTUPDATE* milCmd)
	{
		HRESULT hr{ S_OK };

		hr = g_funCWindowList_UpdateAccentBlurRect.call_org(This, milCmd);

		DWM::CWindowData* windowData{ nullptr };
		DWM::CTopLevelWindow* window{ nullptr };
		winrt::com_ptr<CCompositedAccentBackdrop> backdrop{ nullptr };

		if (
			SUCCEEDED(hr) &&
			SUCCEEDED(This->GetSyncedWindowDataByHwnd(milCmd->GetHwnd(), &windowData)) &&
			windowData &&
			(window = windowData->GetWindow()) &&
			window->GetAccent() &&
			(backdrop = g_visualManager.GetOrCreateAccentBackdrop(window))
			)
		{
			backdrop->UpdateClipRegion();
		}

		return hr;
	}
}
