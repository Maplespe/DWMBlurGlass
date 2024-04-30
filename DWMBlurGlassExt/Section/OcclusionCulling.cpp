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

	ULONGLONG g_frameId{ 0ull };
	bool g_hasWindowBackgroundTreatment{ false };
	Core::CVisual* g_visual{ nullptr };
	Core::MyDynArrayImpl<Core::CZOrderedRect> g_validAntiOccluderList{};

	enum D2D1_DIRECTIONALBLURKERNEL_PROP
	{
		D2D1_DIRECTIONALBLURKERNEL_PROP_STANDARD_DEVIATION,
		D2D1_DIRECTIONALBLURKERNEL_PROP_DIRECTION,
		D2D1_DIRECTIONALBLURKERNEL_PROP_KERNEL_RANGE_FACTOR,
		D2D1_DIRECTIONALBLURKERNEL_PROP_OPTIMIZATION_TRANSFORM
	};
	enum D2D1_DIRECTIONALBLURKERNEL_DIRECTION
	{
		D2D1_DIRECTIONALBLURKERNEL_DIRECTION_X,
		D2D1_DIRECTIONALBLURKERNEL_DIRECTION_Y
	};
	enum D2D1_DIRECTIONALBLURKERNEL_OPTIMIZATION_TRANSFORM
	{
		D2D1_DIRECTIONALBLURKERNEL_OPTIMIZATION_TRANSFORM_IDENDITY,
		D2D1_DIRECTIONALBLURKERNEL_OPTIMIZATION_TRANSFORM_SCALE
	};

	MinHook g_funCVisual_GetWindowBackgroundTreatmentInternal
	{
		"CVisual::GetWindowBackgroundTreatmentInternal", 
		CVisual_GetWindowBackgroundTreatmentInternal
	};

	MinHook g_funCArrayBasedCoverageSet_AddAntiOccluderRect
	{
		"CArrayBasedCoverageSet::AddAntiOccluderRect",
		CArrayBasedCoverageSet_AddAntiOccluderRect
	};

	MinHook g_funCArrayBasedCoverageSet_IsCovered
	{
		"CArrayBasedCoverageSet::IsCovered",
		CArrayBasedCoverageSet_IsCovered
	};

	MinHook g_funCOcclusionContext_PostSubgraph
	{
		"COcclusionContext::PostSubgraph",
		COcclusionContext_PostSubgraph
	};

	void Attach()
	{
		if (g_startup) return;
		g_startup = true;

		if (os::buildNumber < 22000)
		{
			g_funCOcclusionContext_PostSubgraph.Attach();
			g_funCVisual_GetWindowBackgroundTreatmentInternal.Attach();
		g_funCArrayBasedCoverageSet_AddAntiOccluderRect.Attach();
			g_funCArrayBasedCoverageSet_IsCovered.Attach();
	}
	}

	void Detach()
	{
		if (!g_startup) return;

		if (os::buildNumber < 22000)
		{
			g_funCVisual_GetWindowBackgroundTreatmentInternal.Detach();
		g_funCArrayBasedCoverageSet_AddAntiOccluderRect.Detach();
			g_funCArrayBasedCoverageSet_IsCovered.Detach();
			g_funCOcclusionContext_PostSubgraph.Detach();
		}
		g_startup = false;
	}

	Core::CWindowBackgroundTreatment* CVisual_GetWindowBackgroundTreatmentInternal(Core::CVisual* This)
	{
		auto result{ g_funCVisual_GetWindowBackgroundTreatmentInternal.call_org(This) };
		if (g_visual)
		{
			g_hasWindowBackgroundTreatment = true;
		}

		return result;
	}

	HRESULT CArrayBasedCoverageSet_AddAntiOccluderRect(Core::CArrayBasedCoverageSet* This, const D2D1_RECT_F& lprc,
		int depth, const MilMatrix3x2D* matrix)
	{
		auto currentFrameId{ Core::GetCurrentFrameId() };
		if (g_frameId != currentFrameId)
		{
			g_frameId = currentFrameId;
			g_validAntiOccluderList.Clear();
					}
				}
				// much faster one...
				else
				{
					RECT windowRect{};
					window->GetActualWindowRect(&windowRect, false, true, true);
					backdropRegion.reset(CreateRectRgnIndirect(&windowRect));

					if (
			g_visual->GetOwningProcessId() != GetCurrentProcessId() /* actually this is useless, GetOwningProcessId only returns the process id of dwm, idk why */ ||
			g_hasWindowBackgroundTreatment
						)
					{
			Core::CZOrderedRect zorderedRect{ lprc, depth, lprc };
			zorderedRect.UpdateDeviceRect(matrix);
			g_validAntiOccluderList.Add(zorderedRect);
						}
						if (IsZoomed(hwnd))
						{
							CombineRgn(occlusionRegion.get(), occlusionRegion.get(), backdropRegion.get(), RGN_OR);
							continue;
						}
					}
				}

		return g_funCArrayBasedCoverageSet_AddAntiOccluderRect.call_org(This, lprc, depth, matrix);
	}

	bool CArrayBasedCoverageSet_IsCovered(Core::CArrayBasedCoverageSet* This, const D2D1_RECT_F& lprc, int depth,
		bool deprecated)
	{
		auto array{ This->GetAntiOccluderArray() };
		auto arrayBackup{ *array };
		*array = g_validAntiOccluderList;

		bool result{ g_funCArrayBasedCoverageSet_IsCovered.call_org(This, lprc, depth, deprecated) };

		*array = arrayBackup;
		return result;
	}

	HRESULT COcclusionContext_PostSubgraph(Core::COcclusionContext* This, Core::CVisualTree* visualTree,
		bool* unknown)
		{
		g_visual = This->GetVisual();
		g_hasWindowBackgroundTreatment = false;
		HRESULT hr{ g_funCOcclusionContext_PostSubgraph.call_org(This, visualTree, unknown) };
		g_visual = nullptr;
		g_hasWindowBackgroundTreatment = false;

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
