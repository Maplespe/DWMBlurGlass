/**
 * FileName: AccentBlurEffect.h
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

namespace MDWMBlurGlassExt::AccentBlur
{
	void Attach();
	void Detach();
	void Refresh();

	bool CheckWindowType(HWND hWnd);

	DWORD64 WINAPI CFilterEffect_CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive
	(
		DWM::CFilterEffect* This,
		DWM::MilRectF* a2,
		DWM::MilRectF* a3,
		DWM::MilRectF* a4,
		DWM::MilRectF* a5,
		DWM::MilRectF* a6
	);

	DWORD64 WINAPI CD2DContext_FillEffect
	(
		ID2D1DeviceContext** This,
		const DWM::ID2DContextOwner* a2,
		ID2D1Effect* inputEffect,
		const D2D_RECT_F* srcRect,
		const D2D_POINT_2F* dstPoint,
		D2D1_INTERPOLATION_MODE interMode,
		D2D1_COMPOSITE_MODE mode
	);

	DWORD64 WINAPI CSolidColorLegacyMilBrushProxy_Update(
		DWM::CSolidColorLegacyMilBrushProxy* This,
		double a2,
		const D3DCOLORVALUE* a3
	);

	DWORD64 WINAPI CGlassColorizationParameters_AdjustWindowColorization_Win10(
		DWM::GpCC* a1,
		DWM::GpCC* a2,
		float a3,
		char a4
	);

	DWORD64 WINAPI CGlassColorizationParameters_AdjustWindowColorization_Win11(
		DWM::GpCC* a1,
		DWM::GpCC* a2,
		float a3,
		short a4
	);

	void WINAPI CAccent_UpdateAccentBlurRect(
		DWM::CAccent* This,
		const RECT* a2
	);

	HRESULT WINAPI CAccent_UpdateAccentPolicy(
		DWM::CAccent* This,
		RECT* a2,
		DWM::ACCENT_POLICY* a3,
		DWM::CBaseGeometryProxy* a4
	);

	void WINAPI CTopLevelWindow_OnAccentPolicyUpdated(
		DWM::CTopLevelWindow* This
	);

	HRESULT WINAPI CTopLevelWindow_ValidateVisual(
		DWM::CTopLevelWindow* This
	);
}