/**
 * FileName: OcclusionCulling.cpp
 *
 * Copyright (C) 2024 ALTaleX、Maplespe
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
#include "OcclusionCulling.h"
#include "../DWMBlurGlass.h"
#include "CustomBackdropEffect.h"
#include "VisualManager.hpp"
#include "wil.h"

namespace MDWMBlurGlassExt::OcclusionCulling
{
	using namespace MDWMBlurGlass;
	using namespace CommonDef;
	using namespace DWM;

	std::atomic_bool g_startup = false;

	MinHook g_funCWindowList_StyleChange
	{
		"CWindowList::StyleChange", CWindowList_StyleChange
	};

	MinHook g_funCWindowList_CloakChange
	{
		"CWindowList::CloakChange", CWindowList_CloakChange
	};

	MinHook g_funCWindowList_CheckForMaximizedChange
	{
		"CWindowList::CheckForMaximizedChange", CWindowList_CheckForMaximizedChange
	};

	MinHook g_funCArrayBasedCoverageSet_AddAntiOccluderRect
	{
		"CArrayBasedCoverageSet::AddAntiOccluderRect", CArrayBasedCoverageSet_AddAntiOccluderRect
	};

	void Attach()
	{
		if (g_startup) return;
		g_startup = true;

		/*g_funCWindowList_CloakChange.Attach();
		g_funCWindowList_StyleChange.Attach();
		g_funCWindowList_CheckForMaximizedChange.Attach();*/
		g_funCArrayBasedCoverageSet_AddAntiOccluderRect.Attach();
	}

	void Detach()
	{
		if (!g_startup) return;

		/*g_funCWindowList_CloakChange.Detach();
		g_funCWindowList_StyleChange.Detach();
		g_funCWindowList_CheckForMaximizedChange.Detach();*/
		g_funCArrayBasedCoverageSet_AddAntiOccluderRect.Detach();

		g_startup = false;
	}

	void Refresh()
	{
		if (g_configData.blurmethod == blurMethod::CustomBlur && g_configData.occlusionCulling && !g_startup)
			Attach();
		else if ((!g_configData.occlusionCulling || g_configData.blurmethod != blurMethod::CustomBlur) && g_startup)
			Detach();
	}

	bool IsSubRegion(HRGN child, HRGN parent)
	{
		static wil::unique_hrgn region{ CreateRectRgn(0, 0, 0, 0) };
		auto result{ CombineRgn(region.get(), child, parent, g_configData.cullingLevel > 0 ? RGN_OR : RGN_AND) };

		if (g_configData.cullingLevel > 0)
		{
			return static_cast<bool>(EqualRgn(region.get(), parent));
		}
		return result != NULLREGION;

	}

	bool IsRegionIntersect(HRGN region1, HRGN region2)
	{
		static wil::unique_hrgn region{ CreateRectRgn(0, 0, 0, 0) };
		return CombineRgn(region.get(), region1, region2, RGN_AND) != NULLREGION;
	}

	void CalculateOcclusionRegionForWindow(
		wil::unique_hrgn& occlusionRegion,
		HWND hwnd,
		CWindowData* data,
		CTopLevelWindow* window,
		wil::unique_hrgn& windowOcclusionRegion,
		wil::unique_hrgn& backdropRegion
	)
	{
		RECT windowRect{};
		//if (
		//	/*data->IsWindowVisibleAndUncloaked() &&*/
		//	!IsMinimized(hwnd) &&
		//	!(GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED) &&
		//	GetClassWord(hwnd, GCW_ATOM) != RegisterWindowMessageW(L"#32769")
		//	)
		//{
		//	window->GetActualWindowRect(&windowRect, false, true, true);
		//	windowOcclusionRegion.reset(CreateRectRgnIndirect(&windowRect));
		//}

		auto backdrop{ CustomBackdrop::GetCVisualManager()->GetOrCreateBackdrop(window) };
		if (backdrop)
		{
			CopyRgn(backdropRegion.get(), backdrop->GetCompositedRegion());
			OffsetRgn(backdropRegion.get(), windowRect.left, windowRect.top);
			//CombineRgn(windowOcclusionRegion.get(), windowOcclusionRegion.get(), backdropRegion.get(), RGN_DIFF);
		}

		//CombineRgn(occlusionRegion.get(), occlusionRegion.get(), windowOcclusionRegion.get(), RGN_OR);
	}

	void UpdateOcclusionInfo(CWindowList* This)
	{
		ULONG_PTR desktopID{ 0 };
		GetDesktopID(1, &desktopID);

		bool fullyOccluded{ false };
		auto visualManager{ CustomBackdrop::GetCVisualManager() };
		auto backdropCount{ visualManager->GetBackdropCount() };

		RECT fullOcclusionRect
		{
			GetSystemMetrics(SM_XVIRTUALSCREEN),
			GetSystemMetrics(SM_YVIRTUALSCREEN),
			GetSystemMetrics(SM_XVIRTUALSCREEN) + GetSystemMetrics(SM_CXVIRTUALSCREEN),
			GetSystemMetrics(SM_YVIRTUALSCREEN) + GetSystemMetrics(SM_CYVIRTUALSCREEN)
		};
		wil::unique_hrgn fullOcclusionRegion
		{
			CreateRectRgnIndirect(
				&fullOcclusionRect
			)
		};
		wil::unique_hrgn occlusionRegion{ CreateRectRgn(0, 0, 0, 0) };
		auto windowList{ This->GetWindowListForDesktop(desktopID) };
		for (auto i{ windowList->Blink }; i != windowList; i = i->Blink)
		{
			auto data{ reinterpret_cast<CWindowData*>(i) };
			auto hwnd{ data->GetHWND() };
			if (!hwnd || !IsWindow(hwnd)) { continue; }
			auto window{ data->GetWindow() };
			if (!window) { continue; }

			if (!IsWindowVisible(hwnd)) continue;

			auto backdrop{ CustomBackdrop::GetCVisualManager()->GetOrCreateBackdrop(window) };

			if (!fullyOccluded)
			{
				static wil::unique_hrgn backdropRegion{ CreateRectRgn(0, 0, 0, 0) };

				// more accurate but more time-consuming implementation
				if (g_configData.cullingLevel > 0)
				{
					wil::unique_hrgn windowOcclusionRegion{ CreateRectRgn(0, 0, 0, 0) };

					CalculateOcclusionRegionForWindow(occlusionRegion, hwnd, data, window, windowOcclusionRegion, backdropRegion);

					if (EqualRgn(occlusionRegion.get(), fullOcclusionRegion.get()))
					{
						fullyOccluded = true;
					}
				}
				// much faster one...
				else
				{
					RECT windowRect{};
					window->GetActualWindowRect(&windowRect, false, true, true);
					backdropRegion.reset(CreateRectRgnIndirect(&windowRect));

					if (
						data->IsWindowVisibleAndUncloaked() &&
						!(GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED)
						)
					{
						if (
							EqualRect(&windowRect, &fullOcclusionRect) &&
							GetClassWord(hwnd, GCW_ATOM) != RegisterWindowMessageW(L"#32769")
							)
						{
							fullyOccluded = true;
						}
						if (IsZoomed(hwnd))
						{
							CombineRgn(occlusionRegion.get(), occlusionRegion.get(), backdropRegion.get(), RGN_OR);
							continue;
						}
					}
				}

				if (backdrop)
				{
					backdrop->MarkAsOccluded(IsSubRegion(backdropRegion.get(), occlusionRegion.get()) ? true : false);
					backdropCount -= 1;
				}
			}
			else if (backdrop)
			{
				backdrop->MarkAsOccluded(true);
				backdropCount -= 1;
			}

			if (!backdropCount)
			{
				break;
			}
		}

		if (backdropCount != visualManager->GetBackdropCount())
		{
			CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice()->Commit();
		}
	}

	HRESULT CWindowList_StyleChange(CWindowList* This, IDwmWindow* windowContext)
	{
		HRESULT hr = g_funCWindowList_StyleChange.call_org(This, windowContext);

		if (g_configData.cullingLevel >= 0)
		{
			UpdateOcclusionInfo(This);
		}

		return hr;
	}

	HRESULT CWindowList_CloakChange(CWindowList* This, IDwmWindow* windowContext1,
		IDwmWindow* windowContext2, bool cloaked)
	{
		HRESULT hr = g_funCWindowList_CloakChange.call_org(This, windowContext1, windowContext2, cloaked);

		if (g_configData.cullingLevel >= 0)
		{
			UpdateOcclusionInfo(This);
		}

		return hr;
	}

	HRESULT CWindowList_CheckForMaximizedChange(CWindowList* This, CWindowData* data)
	{
		HRESULT hr = g_funCWindowList_CheckForMaximizedChange.call_org(This, data);

		if (g_configData.cullingLevel > 0)
		{
			UpdateOcclusionInfo(This);
		}

		return hr;
	}

	HRESULT CArrayBasedCoverageSet_AddAntiOccluderRect(void* This,
		MilRectF* a2,
		int a3,
		const struct CMILMatrix* a4)
	{
		return S_OK;
	}
}
