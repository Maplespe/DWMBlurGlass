/**
 * FileName: OcclusionCulling.h
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
#pragma once
#include "DWMStruct.h"

namespace MDWMBlurGlassExt::OcclusionCulling
{
	void Attach();
	void Detach();

	DWM::Core::CWindowBackgroundTreatment* CVisual_GetWindowBackgroundTreatmentInternal(DWM::Core::CVisual* This);

	HRESULT STDMETHODCALLTYPE CArrayBasedCoverageSet_AddAntiOccluderRect(
		DWM::Core::CArrayBasedCoverageSet* This,
		const D2D1_RECT_F& lprc,
		int depth,
		const MilMatrix3x2D* matrix
	);

	bool STDMETHODCALLTYPE CArrayBasedCoverageSet_IsCovered(
		DWM::Core::CArrayBasedCoverageSet* This,
		const D2D1_RECT_F& lprc,
		int depth,
		bool deprecated
	);

	HRESULT STDMETHODCALLTYPE COcclusionContext_PostSubgraph(
		DWM::Core::COcclusionContext* This,
		DWM::Core::CVisualTree* visualTree,
		bool* unknown
	);
}