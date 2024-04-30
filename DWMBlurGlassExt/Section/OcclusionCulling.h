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
	void Refresh();

	HRESULT WINAPI CWindowList_StyleChange(DWM::CWindowList* This, DWM::IDwmWindow* windowContext);
	HRESULT WINAPI CWindowList_CloakChange(DWM::CWindowList* This, DWM::IDwmWindow* windowContext1,  DWM::IDwmWindow* windowContext2, bool cloaked);
	HRESULT WINAPI CWindowList_CheckForMaximizedChange(DWM::CWindowList* This, DWM::CWindowData* data);

	HRESULT WINAPI CArrayBasedCoverageSet_AddAntiOccluderRect(
		void* This,
		DWM::MilRectF* a2,
		int a3,
		const struct CMILMatrix* a4);
}