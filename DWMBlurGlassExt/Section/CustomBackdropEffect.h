/**
 * FileName: CustomBackdropEffect.h
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
#pragma once
#include "DWMStruct.h"

namespace MDWMBlurGlassExt::CustomBackdrop
{
	void Attach();
	void Detach();
	void Refresh();

	HRESULT WINAPI CTopLevelWindow_InitializeVisualTreeClone(
		DWM::CTopLevelWindow* This,
		DWM::CTopLevelWindow* topLevelWindow,
		UINT cloneOptions
	);

	HRESULT WINAPI CTopLevelWindow_UpdateNCAreaBackground(DWM::CTopLevelWindow* This);

	HRESULT WINAPI CTopLevelWindow_ValidateVisual(DWM::CTopLevelWindow* This);

	void WINAPI CTopLevelWindow_Destructor(DWM::CTopLevelWindow* This);

	HRESULT WINAPI ResourceHelper_CreateGeometryFromHRGN(HRGN hrgn, DWM::CRgnGeometryProxy** geometry);
}