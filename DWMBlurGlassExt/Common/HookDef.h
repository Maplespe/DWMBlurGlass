/**
 * FileName: HookDef.h
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
#include "../Section/AccentBlurEffect.h"
#include "../Section/CustomButton.h"
#include "../Section/TitleTextTweaker.h"
#include "../Section/BlurRadiusTweaker.h"
#include "../Section/CustomBackdropEffect.h"
#include "../Section/DwmAPIEffect.h"

namespace MDWMBlurGlassExt
{
	namespace Common
	{
		HRESULT WINAPI CTopLevelWindow_ValidateVisual(DWM::CTopLevelWindow* This);
	}

	inline MiniHookDispatcher g_CTopLevelWindow_ValidateVisual_HookDispatcher =
	{
		"CTopLevelWindow::ValidateVisual",
		std::array
		{
			detour_info(AccentBlur::CTopLevelWindow_ValidateVisual, call_type::before),
			detour_info(CustomButton::CTopLevelWindow_ValidateVisual, call_type::before),
			detour_info(CustomBackdrop::CTopLevelWindow_ValidateVisual, call_type::after),
			detour_info(DwmAPIEffect::CTopLevelWindow_ValidateVisual, call_type::after),
			detour_info(Common::CTopLevelWindow_ValidateVisual, call_type::after)
		}
	};

	inline MiniHookDispatcher g_CD2DContext_FillEffect_HookDispatcher =
	{
		"CD2DContext::FillEffect",
		std::array
		{
			detour_info(BlurRadiusTweaker::CD2DContext_FillEffect, call_type::before),
			detour_info(AccentBlur::CD2DContext_FillEffect, call_type::after)
		}
	};

	inline MiniHookDispatcher g_CTopLevelWindow_InitializeVisualTreeClone_HookDispatcher =
	{
		"CTopLevelWindow::InitializeVisualTreeClone",
		std::array
		{
			detour_info(CustomBackdrop::CTopLevelWindow_InitializeVisualTreeClone, call_type::after),
			detour_info(CustomButton::CTopLevelWindow_InitializeVisualTreeClone, call_type::after)
		}
	};

	inline MiniHookDispatcher g_CTopLevelWindow_CTopLevelWindow_Destructor_HookDispatcher =
	{
		"CTopLevelWindow::~CTopLevelWindow",
		std::array
		{
			detour_info(CustomBackdrop::CTopLevelWindow_Destructor, call_type::before),
			detour_info(CustomButton::CTopLevelWindow_Destructor, call_type::before)
		}
	};

	HRGN WINAPI MyCreateRoundRectRgn(
		int x1,
		int y1,
		int x2, 
		int y2, 
		int w, 
		int h
	);

	DWORD64 WINAPI HrgnFromRects(
		const tagRECT* Src, unsigned int a2, HRGN* a3
	);

	/*DWORD64 WINAPI CWindowList_BlurBehindChange(
		DWM::CWindowList* This,
		DWM::IDwmWindow* a2,
		const DWM_BLURBEHIND* a3
	);*/
}
