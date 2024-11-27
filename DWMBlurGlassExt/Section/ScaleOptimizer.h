/**
 * FileName: ScaleOptimizer.h
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

namespace MDWMBlurGlassExt::ScaleOptimizer
{
	void Attach();
	void Detach();
	void Refresh();

	float STDMETHODCALLTYPE CCustomBlur_DetermineOutputScale(
		float size,
		float blurAmount,
		D2D1_GAUSSIANBLUR_OPTIMIZATION optimization
	);

	void STDMETHODCALLTYPE CBlurRenderingGraph_DeterminePreScale(
		const DWM::Core::EffectInput& input1,
		const DWM::Core::EffectInput& input2,
		D2D1_GAUSSIANBLUR_OPTIMIZATION optimization,
		const D2D1_VECTOR_2F& blurAmount,
		D2D1_VECTOR_2F* scaleAmount
	);

	void STDMETHODCALLTYPE CBlurRenderingGraph_DeterminePreScale_Build22000(
		const DWM::Core::CDrawingContext* drawingContext,
		const DWM::Core::EffectInput& input1,
		const DWM::Core::EffectInput& input2,
		D2D1_GAUSSIANBLUR_OPTIMIZATION optimization,
		const D2D1_VECTOR_2F& blurAmount,
		D2D1_VECTOR_2F* scaleAmount
	);

	HRESULT STDMETHODCALLTYPE CCustomBlur_Draw(
		DWM::Core::CCustomBlur* This,
		DWM::Core::CDrawingContext* drawingContext,
		const D2D1_RECT_F& destinationRect,
		const D2D1_POINT_2F* point,
		D2D1_INTERPOLATION_MODE interpolationMode,
		D2D1_COMPOSITE_MODE compositeMode
	);
}