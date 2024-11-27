/**
 * FileName: CustomBackdropEffect.cpp
 *
 * Copyright (C) 2024 ALTaleX531, Maplespe
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
#include "GeometryRecorder.h"
#include "OcclusionCulling.h"
#include "HookDef.h"
#include "wil.h"
#include "CommonDef.h"

namespace MDWMBlurGlassExt::CustomBackdrop
{
	using namespace CommonDef;
	using namespace MDWMBlurGlass;
	using namespace DWM;

	std::atomic_bool g_startup = false;

	size_t g_captureRef{ 0 };
	std::vector<com_ptr<CBaseGeometryProxy>> g_geometryBuffer{};

	ULONGLONG g_oldBackdropBlurCachingThrottleQPCTimeDelta{ 0ull };

	FORCEINLINE bool IsBackdropAllowed(CTopLevelWindow* This)
	{
		if (g_configData.powerSavingMode && g_configData.disableOnBattery)
		{
			return false;
		}

		if (This && GetActualBackdropKind(This) == CompositedBackdropKind::SystemBackdrop && !g_configData.applyglobal)
		{
			CVisualManager::Trim(This);
			return false;
		}
		return true;
	}

	MinHook g_funCTopLevelWindow_CalculateBackgroundType
	{
		"CTopLevelWindow::CalculateBackgroundType",
		CTopLevelWindow_CalculateBackgroundType
	};

	MinHook g_funCDrawGeometryInstruction_Create
	{
		"CDrawGeometryInstruction::Create",
		CDrawGeometryInstruction_Create
	};

	MinHook g_funCTopLevelWindow_UpdateNCAreaBackground
	{
		"CTopLevelWindow::UpdateNCAreaBackground",
		CTopLevelWindow_UpdateNCAreaBackground
	};

	MinHook g_funCTopLevelWindow_UpdateClientBlur
	{
		"CTopLevelWindow::UpdateClientBlur",
		CTopLevelWindow_UpdateClientBlur
	};

	MinHook g_funCTopLevelWindow_UpdateAccent
	{
		"CTopLevelWindow::UpdateAccent",
		CTopLevelWindow_UpdateAccent
	};

	MinHook g_funCTopLevelWindow_UpdateSystemBackdropVisual
	{
		"CTopLevelWindow::UpdateSystemBackdropVisual",
		CTopLevelWindow_UpdateSystemBackdropVisual
	};

	MinHook g_funCTopLevelWindow_CloneVisualTree
	{
		"CTopLevelWindow::CloneVisualTree",
		CTopLevelWindow_CloneVisualTree
	};

	MinHook g_funCTopLevelWindow_CloneVisualTree_1903
	{
		"CTopLevelWindow::CloneVisualTree",
		CTopLevelWindow_CloneVisualTree_Pre1903
	};

	MinHook g_funCTopLevelWindow_OnClipUpdated
	{
		"CTopLevelWindow::OnClipUpdated",
		CTopLevelWindow_OnClipUpdated
	};

	MinHook g_funCWindowList_UpdateAccentBlurRect
	{
		"CWindowList::UpdateAccentBlurRect",
		CWindowList_UpdateAccentBlurRect
	};

	CompositedBackdropKind GetActualBackdropKind(CTopLevelWindow* This, bool overrideAccent)
	{
		auto backgroundType{ CompositedBackdropKind::Legacy };
		auto windowData{ This->GetData() };

		if (os::buildNumber < 22000)
		{
			backgroundType = windowData ?
				(windowData->GetAccentPolicy()->IsActive() ? CompositedBackdropKind::Accent : CompositedBackdropKind::Legacy) :
				CompositedBackdropKind::Legacy;
		}
		else if (os::buildNumber < 22621)
		{
			backgroundType = windowData ?
				(
					*reinterpret_cast<DWORD*>(reinterpret_cast<ULONG_PTR>(windowData) + 204) ?
					CompositedBackdropKind::SystemBackdrop :
					(
						windowData->GetAccentPolicy()->IsActive() ?
						CompositedBackdropKind::Accent :
						CompositedBackdropKind::Legacy
						)
					) :
				CompositedBackdropKind::Legacy;
		}
		else
		{
			auto calculatedBackdropType{ g_funCTopLevelWindow_CalculateBackgroundType.call_org(This) };
			if (calculatedBackdropType == 2 || calculatedBackdropType == 3)
			{
				backgroundType = CompositedBackdropKind::SystemBackdrop;
			}
			else if (calculatedBackdropType == 4 || calculatedBackdropType == 0)
			{
				backgroundType = CompositedBackdropKind::Legacy;
			}
			else
			{
				backgroundType = CompositedBackdropKind::Accent;
			}
		}

		if (!g_configData.overrideAccent && overrideAccent && backgroundType == CompositedBackdropKind::Accent)
		{
			backgroundType = CompositedBackdropKind::Legacy;
		}
		return backgroundType;
	}

	void Attach()
	{
		if (g_startup) return;
		g_startup = true;

		if (os::buildNumber >= 22621)
		{
			g_funCTopLevelWindow_CalculateBackgroundType.Attach();
		}

		g_funCDrawGeometryInstruction_Create.Attach();
		g_funCTopLevelWindow_UpdateNCAreaBackground.Attach();
		g_funCTopLevelWindow_UpdateClientBlur.Attach();
		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<2, true>();
		g_funCTopLevelWindow_UpdateAccent.Attach();
		g_CTopLevelWindow_CTopLevelWindow_Destructor_HookDispatcher.enable_hook_routine<0, true>();
		g_funCWindowList_UpdateAccentBlurRect.Attach();

		if (os::buildNumber < 18362)
		{
			g_funCTopLevelWindow_CloneVisualTree_1903.Attach();
		}
		else if (os::buildNumber < 19041)
		{
			g_funCTopLevelWindow_OnClipUpdated.Attach();
			g_funCTopLevelWindow_CloneVisualTree.Attach();
		}
		else
		{
			g_funCTopLevelWindow_OnClipUpdated.Attach();
			g_CTopLevelWindow_InitializeVisualTreeClone_HookDispatcher.enable_hook_routine<0, true>();
		}
		if (os::buildNumber == 22000)
		{
			g_funCTopLevelWindow_UpdateSystemBackdropVisual.Attach();
		}

		GeometryRecorder::Attach();
		OcclusionCulling::Attach();
	}

	void Detach()
	{
		if (!g_startup) return;

		g_funCDrawGeometryInstruction_Create.Detach();
		g_funCTopLevelWindow_UpdateNCAreaBackground.Detach();
		g_funCTopLevelWindow_UpdateClientBlur.Detach();
		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<2, false>();
		g_funCTopLevelWindow_UpdateAccent.Detach();
		g_CTopLevelWindow_CTopLevelWindow_Destructor_HookDispatcher.enable_hook_routine<0, false>();
		g_funCWindowList_UpdateAccentBlurRect.Detach();

		if (os::buildNumber < 18362)
		{
			g_funCTopLevelWindow_CloneVisualTree_1903.Detach();
		}
		else if (os::buildNumber < 19041)
		{
			g_funCTopLevelWindow_OnClipUpdated.Detach();
			g_funCTopLevelWindow_CloneVisualTree.Detach();
		}
		else
		{
			g_funCTopLevelWindow_OnClipUpdated.Detach();
			g_CTopLevelWindow_InitializeVisualTreeClone_HookDispatcher.enable_hook_routine<0, false>();
		}
		if (os::buildNumber == 22000)
		{
			g_funCTopLevelWindow_UpdateSystemBackdropVisual.Detach();
		}
		if (os::buildNumber >= 22621)
		{
			g_funCTopLevelWindow_CalculateBackgroundType.Detach();
		}

		CVisualManager::Shutdown();
		GeometryRecorder::Detach();
		OcclusionCulling::Detach();

		g_startup = false;
	}

	void Refresh()
	{
		if (g_startup)
		{
			CVisualManager::RefreshEffectConfig();

			auto lock{ wil::EnterCriticalSection(CDesktopManager::s_csDwmInstance) };
			if (!IsBackdropAllowed(nullptr))
			{
				CVisualManager::Shutdown();
			}
			else
			{
				ULONG_PTR desktopID{ 0 };
				GetDesktopID(1, &desktopID);
				auto windowList{ CDesktopManager::s_pDesktopManagerInstance->GetWindowList()->GetWindowListForDesktop(desktopID) };
				for (auto i{ windowList->Blink }; i != windowList; i = i->Blink)
				{
					auto data{ reinterpret_cast<DWM::CWindowData*>(i) };
					if (!data->IsWindowVisibleAndUncloaked()) { continue; }
					auto hwnd{ data->GetHWND() };
					if (!hwnd || !IsWindow(hwnd)) { continue; }
					auto window{ data->GetWindow() };
					if (!window || window->IsTrullyMinimized()) { continue; }

					auto backdrop{ CVisualManager::GetOrCreate(window, window->HasNonClientBackground() || GetActualBackdropKind(window) == CompositedBackdropKind::Accent) };
					if (backdrop)
					{
						backdrop->ValidateVisual();
					}
					if(GetActualBackdropKind(window, false) == CompositedBackdropKind::Accent)
					{
						window->OnAccentPolicyUpdated();
					}
				}
			}
		}
		bool nosupport = os::buildNumber < 22000 && g_configData.effectType == effectType::Mica;

		if (g_configData.blurmethod == blurMethod::CustomBlur && !g_startup && !nosupport && !g_configData.powerSavingMode)
			Attach();
		else if ((g_configData.blurmethod != blurMethod::CustomBlur || g_configData.powerSavingMode || nosupport) && g_startup)
			Detach();
	}

	HRESULT CDrawGeometryInstruction_Create(CBaseLegacyMilBrushProxy* brush, CBaseGeometryProxy* geometry,
		CDrawGeometryInstruction** instruction)
	{
		if (g_captureRef)
		{
			com_ptr<CBaseGeometryProxy> referencedGeometry{ nullptr };
			copy_from_abi(referencedGeometry, geometry);
			g_geometryBuffer.push_back(referencedGeometry);
		}

		return g_funCDrawGeometryInstruction_Create.call_org(brush, geometry, instruction);
	}

	HRESULT CTopLevelWindow_UpdateNCAreaBackground(CTopLevelWindow* This)
	{
		if (!IsBackdropAllowed(This))
		{
			return g_funCTopLevelWindow_UpdateNCAreaBackground.call_org(This);
		}
		auto data{ This->GetData() };
		if (!data || !data->IsWindowVisibleAndUncloaked() || This->IsTrullyMinimized())
		{
			CVisualManager::Trim(This);
			return g_funCTopLevelWindow_UpdateNCAreaBackground.call_org(This);
		}

		HRESULT hr{ S_OK };

		com_ptr backdrop{ CVisualManager::GetOrCreate(This) };

		if (This->HasNonClientBackground())
		{
			GeometryRecorder::BeginCapture();

			DWORD oldSystemBackdropType{ 0 };
			if (os::buildNumber == 22000)
			{
				oldSystemBackdropType = *reinterpret_cast<DWORD*>(reinterpret_cast<ULONG_PTR>(data) + 204);
				*reinterpret_cast<DWORD*>(reinterpret_cast<ULONG_PTR>(data) + 204) = 0;
			}
			if (os::buildNumber >= 26100)
			{
				oldSystemBackdropType = reinterpret_cast<DWORD*>(data)[200];
				reinterpret_cast<DWORD*>(data)[200] = 0;
			}

			hr = g_funCTopLevelWindow_UpdateNCAreaBackground.call_org(This);

			if (os::buildNumber == 22000)
			{
				*reinterpret_cast<DWORD*>(reinterpret_cast<ULONG_PTR>(data) + 204) = oldSystemBackdropType;
			}
			if (os::buildNumber >= 22621)
			{
				reinterpret_cast<DWORD*>(data)[200] = oldSystemBackdropType;
			}

			if (SUCCEEDED(hr))
			{
				backdrop = CVisualManager::GetOrCreate(This, true);
				// the titlebar region has been updated
				// let's update our backdrop region
				if (GeometryRecorder::GetGeometryCount() && backdrop)
				{
					auto borderGeometry{ This->GetBorderGeometry() };
					auto captionGeometry{ This->GetTitlebarGeometry() };

					HRGN borderRegion{ GeometryRecorder::GetRegionFromGeometry(borderGeometry) };
					HRGN captionRegion{ GeometryRecorder::GetRegionFromGeometry(captionGeometry) };
					wil::unique_hrgn realBorderRegion{ CreateRectRgn(0, 0, 0, 0) };
					wil::unique_hrgn emptyRegion{ CreateRectRgn(0, 0, 0, 0) };

					if (!g_configData.extendBorder && !data->IsFrameExtendedIntoClientAreaLRB())
					{
						RECT captionBox{};
						RECT borderBox{};
						GetRgnBox(captionRegion, &captionBox);
						if (GetRgnBox(borderRegion, &borderBox) != NULLREGION)
						{
							LONG borderWidth{ captionBox.top - borderBox.top };
							wil::unique_hrgn nonBorderRegion{ CreateRectRgn(borderBox.left + borderWidth, borderBox.top + borderWidth, borderBox.right - borderWidth, borderBox.bottom - borderWidth) };
							CombineRgn(realBorderRegion.get(), borderRegion, nonBorderRegion.get(), RGN_DIFF);
						}
					}

					backdrop->SetBorderRegion(borderRegion);
					backdrop->SetCaptionRegion(captionRegion);

					if (borderGeometry)
					{
						ResourceHelper::CreateGeometryFromHRGN(realBorderRegion.get(), &borderGeometry);
					}
					if (captionGeometry)
					{
						ResourceHelper::CreateGeometryFromHRGN(emptyRegion.get(), &captionGeometry);
					}
					if (EqualRgn(realBorderRegion.get(), emptyRegion.get()))
					{
						This->GetLegacyVisual()->ClearInstructions();
					}
				}
			}

			GeometryRecorder::EndCapture();
		}
		else
		{
			backdrop = CVisualManager::GetOrCreate(This, GetActualBackdropKind(This) != CompositedBackdropKind::Legacy);
			hr = g_funCTopLevelWindow_UpdateNCAreaBackground.call_org(This);

			// let's update our backdrop region
			if (SUCCEEDED(hr) && backdrop)
			{
				wil::unique_hrgn emptyRegion{ CreateRectRgn(0, 0, 0, 0) };
				backdrop->SetBorderRegion(emptyRegion.get());
				backdrop->SetCaptionRegion(emptyRegion.get());
			}
			else
			{
				CVisualManager::Trim(This);
			}
		}

		if (backdrop)
		{
			backdrop->UpdateNCBackground();
			backdrop->ValidateVisual();
		}

		return hr;
	}

	HRESULT CTopLevelWindow_UpdateClientBlur(CTopLevelWindow* This)
	{
		if (!IsBackdropAllowed(This))
		{
			return g_funCTopLevelWindow_UpdateClientBlur.call_org(This);
		}
		auto data{ This->GetData() };
		if (!data || !data->IsWindowVisibleAndUncloaked() || This->IsTrullyMinimized())
		{
			CVisualManager::Trim(This);
			return g_funCTopLevelWindow_UpdateClientBlur.call_org(This);
		}

		GeometryRecorder::BeginCapture();
		g_captureRef++;

		HRESULT hr{ g_funCTopLevelWindow_UpdateClientBlur.call_org(This) };
		if (SUCCEEDED(hr))
		{
			// the dwmblurregion is updated
			// UpdateClientBlur will only create at most one geometry draw instrution
			if (!g_geometryBuffer.empty())
			{
				HRGN clientBlurRegion{ GeometryRecorder::GetRegionFromGeometry(g_geometryBuffer.back().get()) };
				RECT clientBlurBox{};
				// let's update our backdrop region
				auto backdrop{ CVisualManager::GetOrCreate(This, GetRgnBox(clientBlurRegion, &clientBlurBox) != NULLREGION && !IsRectEmpty(&clientBlurBox), true) };
				if (backdrop)
				{
					backdrop->SetClientBlurRegion(clientBlurRegion);
					backdrop->ValidateVisual();
				}
			}
		}

		g_captureRef--;
		g_geometryBuffer.clear();
		GeometryRecorder::EndCapture();

		return hr;
	}

	HRESULT CTopLevelWindow_ValidateVisual(CTopLevelWindow* This)
	{
		if (!IsBackdropAllowed(This))
		{
			return S_OK;
		}
		auto data{ This->GetData() };
		if (!data || !data->IsWindowVisibleAndUncloaked() || This->IsTrullyMinimized())
		{
			CVisualManager::Trim(This);
			return S_OK;
		}

		auto kind{ (GetActualBackdropKind(This)) };
		auto accentPolicy{ data->GetAccentPolicy() };
		auto oldAccentState{ accentPolicy->nAccentState };

		if (kind == CompositedBackdropKind::Accent) { accentPolicy->nAccentState = ACCENT_DISABLED; }
		HRESULT hr{ g_CTopLevelWindow_ValidateVisual_HookDispatcher.call_org(This) };
		g_CTopLevelWindow_ValidateVisual_HookDispatcher.modify_handle_type_for_temporary(handling_type::skip);
		if (kind == CompositedBackdropKind::Accent) { accentPolicy->nAccentState = oldAccentState; }

		if (SUCCEEDED(hr))
		{
			if (auto backdrop{ CVisualManager::GetOrCreate(This) })
			{
				// update existing backdrop
				backdrop->ValidateVisual();
			}
		}

		return hr;
	}

	HRESULT CTopLevelWindow_UpdateAccent(CTopLevelWindow* This, bool visibleAndUncloaked)
	{
		if (!IsBackdropAllowed(This))
		{
			return g_funCTopLevelWindow_UpdateAccent.call_org(This, visibleAndUncloaked);
		}
		auto data{ This->GetData() };
		if (!data || !data->IsWindowVisibleAndUncloaked() || This->IsTrullyMinimized())
		{
			CVisualManager::Trim(This);
			return g_funCTopLevelWindow_UpdateAccent.call_org(This, visibleAndUncloaked);
		}

		auto kind{ (GetActualBackdropKind(This)) };
		HRESULT hr{ S_OK };
		if (kind == CompositedBackdropKind::Accent)
		{
			auto accentPolicy{ data->GetAccentPolicy() };
			auto oldAccentState{ accentPolicy->nAccentState };

			accentPolicy->nAccentState = ACCENT_DISABLED;
			hr = g_funCTopLevelWindow_UpdateAccent.call_org(This, visibleAndUncloaked);
			accentPolicy->nAccentState = oldAccentState;

			if (com_ptr backdrop{ CVisualManager::GetOrCreate(This, kind == CompositedBackdropKind::Accent) })
			{
				backdrop->ValidateVisual();
			}
		}
		else
		{
			hr = g_funCTopLevelWindow_UpdateAccent.call_org(This, visibleAndUncloaked);
		}

		return hr;
	}

	DWORD CTopLevelWindow_CalculateBackgroundType(CTopLevelWindow* This)
	{
		if (!IsBackdropAllowed(This))
		{
			return g_funCTopLevelWindow_CalculateBackgroundType.call_org(This);
		}

		auto result{ g_funCTopLevelWindow_CalculateBackgroundType.call_org(This) };
		if (result == 4 || result == 3 || result == 2)
		{
			result = 0;
		}

		return result;
	}

	HRESULT CTopLevelWindow_UpdateSystemBackdropVisual(CTopLevelWindow* This)
	{
		if (!IsBackdropAllowed(This))
		{
			return g_funCTopLevelWindow_UpdateSystemBackdropVisual.call_org(This);
		}
		auto data{ This->GetData() };
		if (!data || !data->IsWindowVisibleAndUncloaked() || This->IsTrullyMinimized())
		{
			CVisualManager::Trim(This);
			return g_funCTopLevelWindow_UpdateSystemBackdropVisual.call_org(This);
		}

		HRESULT hr{ S_OK };
		auto oldSystemBackdropType{ *reinterpret_cast<DWORD*>(reinterpret_cast<ULONG_PTR>(data) + 204) };
		// trick dwm into thinking the window does not enable system backdrop
		*reinterpret_cast<DWORD*>(reinterpret_cast<ULONG_PTR>(data) + 204) = 0;
		hr = g_funCTopLevelWindow_UpdateSystemBackdropVisual.call_org(This);
		*reinterpret_cast<DWORD*>(reinterpret_cast<ULONG_PTR>(data) + 204) = oldSystemBackdropType;

		return hr;
	}

	void CTopLevelWindow_Destructor(CTopLevelWindow* This)
	{
		CVisualManager::Remove(This, true);
	}

	HRESULT CTopLevelWindow_InitializeVisualTreeClone(CTopLevelWindow* This, CTopLevelWindow* window,
		UINT cloneOptions)
	{
		auto data{ This->GetData() };
		if (!data)
		{
			return S_OK;
		}

		DWORD oldSystemBackdropType{ 0 };
		if (os::buildNumber >= 26100)
		{
			oldSystemBackdropType = reinterpret_cast<DWORD*>(data)[200];
			reinterpret_cast<DWORD*>(data)[200] = 0;
		}

		if (os::buildNumber == 22000)
		{
			*reinterpret_cast<DWORD*>(reinterpret_cast<ULONG_PTR>(data) + 204) = oldSystemBackdropType;
		}
		if (SUCCEEDED(g_CTopLevelWindow_InitializeVisualTreeClone_HookDispatcher.return_value()))
		{
			CVisualManager::TryClone(This, window);
		}
		return S_OK;
	}

	HRESULT CTopLevelWindow_CloneVisualTree(CTopLevelWindow* This, CTopLevelWindow** window, bool unknown)
	{
		HRESULT hr{ g_funCTopLevelWindow_CloneVisualTree.call_org(This, window, unknown) };

		if (SUCCEEDED(hr) && window && *window)
		{
			CVisualManager::TryClone(This, *window);
		}
		return hr;
	}

	HRESULT CTopLevelWindow_CloneVisualTree_Pre1903(DWM::CTopLevelWindow* This, DWM::CTopLevelWindow** window,
		bool unknown1, bool unknown2, bool unknown3)
	{
		HRESULT hr{ g_funCTopLevelWindow_CloneVisualTree_1903.call_org(This, window, unknown1, unknown2, unknown3) };

		if (SUCCEEDED(hr) && window && *window)
		{
			CVisualManager::TryClone(This, *window);
		}
		return hr;
	}

	HRESULT CTopLevelWindow_OnClipUpdated(CTopLevelWindow* This)
	{
		if (!IsBackdropAllowed(This))
		{
			return g_funCTopLevelWindow_OnClipUpdated.call_org(This);
		}
		auto data{ This->GetData() };
		if (!data || !data->IsWindowVisibleAndUncloaked() || This->IsTrullyMinimized())
		{
			CVisualManager::Trim(This);
			return g_funCTopLevelWindow_OnClipUpdated.call_org(This);
		}

		HRESULT hr{ g_funCTopLevelWindow_OnClipUpdated.call_org(This) };

		if (SUCCEEDED(hr))
		{
			auto kind{ GetActualBackdropKind(This) };
			winrt::com_ptr<DCompPrivate::ICompositedBackdropVisual> backdrop{ CVisualManager::GetOrCreate(This, kind == CompositedBackdropKind::Accent) };
			if (backdrop)
			{
				wil::unique_hrgn clipRgn{ CreateRectRgn(0, 0, 0, 0) };
				if (GetWindowRgn(data->GetHWND(), clipRgn.get()) != ERROR)
				{
					backdrop->SetGdiWindowRegion(clipRgn.get());
				}
				else
				{
					backdrop->SetGdiWindowRegion(nullptr);
				}

				backdrop->ValidateVisual();
			}
		}
		return hr;
	}

	HRESULT CWindowList_UpdateAccentBlurRect(CWindowList* This,
		const MILCMD_DWM_REDIRECTION_ACCENTBLURRECTUPDATE* milCmd)
	{
		if (!IsBackdropAllowed(nullptr))
		{
			return g_funCWindowList_UpdateAccentBlurRect.call_org(This, milCmd);
		}

		HRESULT hr{ g_funCWindowList_UpdateAccentBlurRect.call_org(This, milCmd) };

		CWindowData* data{ nullptr };
		CTopLevelWindow* window{ nullptr };
		auto lock{ wil::EnterCriticalSection(CDesktopManager::s_csDwmInstance) };
		if (SUCCEEDED(hr) && SUCCEEDED(hr = This->GetSyncedWindowDataByHwnd(milCmd->GetHwnd(), &data)) && data && (window = data->GetWindow()))
		{
			auto kind{ GetActualBackdropKind(window) };
			auto lprc{ milCmd->GetRect() };
			if (
				lprc &&
				(
					lprc->right <= lprc->left ||
					lprc->bottom <= lprc->top
					)
				)
			{
				lprc = nullptr;
			}
			com_ptr backdrop{ CVisualManager::GetOrCreateForAccentBlurRect(window, lprc, kind == CompositedBackdropKind::Accent && lprc) };
			if (backdrop)
			{
				backdrop->SetAccentRect(lprc);
				backdrop->ValidateVisual();
			}
		}
		return hr;
	}
}
