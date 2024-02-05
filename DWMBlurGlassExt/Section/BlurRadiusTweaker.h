/**
 * FileName: BlurRadiusTweaker.h
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

namespace MDWMBlurGlassExt::BlurRadiusTweaker
{
	void Attach();
	void Detach();
	void Refresh();

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

	DWORD64 WINAPI CRenderingTechnique_ExecuteBlur
	(
		DWM::CRenderingTechnique* This,
		DWM::CDrawingContext* a2,
		const DWM::EffectInput* a3,
		const D2D_VECTOR_2F* a4,
		DWM::EffectInput* a5
	);

	DWORD64 WINAPI CCustomBlur_BuildEffect(
		DWM::CCustomBlur* This,
		ID2D1Image* backdropImage,
		const D2D_RECT_F* srcRect,
		const D2D_SIZE_F* kSize,
		D2D1_GAUSSIANBLUR_OPTIMIZATION a5,
		const D2D_VECTOR_2F* a6,
		D2D_VECTOR_2F* a7
	);

	float WINAPI CCustomBlur_DetermineOutputScale(
		float a1,
		float a2,
		D2D1_GAUSSIANBLUR_OPTIMIZATION a3
	);
}