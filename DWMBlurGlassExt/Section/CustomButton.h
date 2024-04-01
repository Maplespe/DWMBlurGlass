/**
 * FileName: CustomButton.h
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
#pragma once
#include "DWMStruct.h"

namespace MDWMBlurGlassExt::CustomButton
{
	void Attach();
	void Detach();
	void Refresh();

	HRESULT WINAPI CTopLevelWindow_ValidateVisual(DWM::CTopLevelWindow* This);

	HRESULT WINAPI CButton_UpdateLayout(DWM::CButton* This);

	HRESULT WINAPI CButton_DrawStateW(DWM::CButton* This, DWM::CButton* a2, unsigned int a3);
}