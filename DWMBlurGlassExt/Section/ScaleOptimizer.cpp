/**
 * FileName: ScaleOptimizer.cpp
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
#include "ScaleOptimizer.h"
#include "CommonDef.h"

namespace MDWMBlurGlassExt::ScaleOptimizer
{
	using namespace MDWMBlurGlass;
	using namespace DWM;
	using namespace CommonDef;

	std::atomic_bool g_startup = false;

	MinHook g_funCCustomBlur_DetermineOutputScale
	{
		"CCustomBlur::DetermineOutputScale",
		CCustomBlur_DetermineOutputScale
	};

	MinHook g_funCBlurRenderingGraph_DeterminePreScale
	{
		"CBlurRenderingGraph::DeterminePreScale",
		CBlurRenderingGraph_DeterminePreScale
	};

	MinHook g_funCBlurRenderingGraph_DeterminePreScale22000
	{
		"CBlurRenderingGraph::DeterminePreScale",
		CBlurRenderingGraph_DeterminePreScale_Build22000
	};

	MinHook g_funCCustomBlur_Draw
	{
		"CCustomBlur::Draw",
		CCustomBlur_Draw
	};

	void Attach()
	{
		if (g_startup) return;
		g_startup = true;

		if (os::buildNumber < 22000)
		{
			g_funCCustomBlur_DetermineOutputScale.Attach();
			g_funCCustomBlur_Draw.Attach();
		}
		else if (os::buildNumber < 22621)
			g_funCBlurRenderingGraph_DeterminePreScale22000.Attach();
		else
			g_funCBlurRenderingGraph_DeterminePreScale.Attach();
	}

	void Detach()
	{
		if (!g_startup) return;

		if (os::buildNumber < 22000)
		{
			g_funCCustomBlur_DetermineOutputScale.Detach();
			g_funCCustomBlur_Draw.Detach();
		}
		else if (os::buildNumber < 22621)
			g_funCBlurRenderingGraph_DeterminePreScale22000.Detach();
		else
			g_funCBlurRenderingGraph_DeterminePreScale.Detach();

		g_startup = false;
	}

	void Refresh()
	{
		if (g_configData.blurmethod == blurMethod::CustomBlur && g_configData.scaleOptimizer && !g_startup)
			Attach();
		else if ((!g_configData.scaleOptimizer || g_configData.blurmethod != blurMethod::CustomBlur) && g_startup)
			Detach();
	}

	float CCustomBlur_DetermineOutputScale(float size, float blurAmount, D2D1_GAUSSIANBLUR_OPTIMIZATION optimization)
	{
		auto result{ g_funCCustomBlur_DetermineOutputScale.call_org(size, blurAmount, optimization) };
		return result * 0.5f;
	}

	void CBlurRenderingGraph_DeterminePreScale(const Core::EffectInput& input1,
		const Core::EffectInput& input2, D2D1_GAUSSIANBLUR_OPTIMIZATION optimization,
		const D2D1_VECTOR_2F& blurAmount, D2D1_VECTOR_2F* scaleAmount)
	{
		g_funCBlurRenderingGraph_DeterminePreScale.call_org(input1, input2, optimization, blurAmount, scaleAmount);
		if (scaleAmount)
		{
			scaleAmount->x *= 0.5f;
			scaleAmount->y *= 0.5f;
		}
	}

	void CBlurRenderingGraph_DeterminePreScale_Build22000(const Core::CDrawingContext* drawingContext,
		const Core::EffectInput& input1, const Core::EffectInput& input2,
		D2D1_GAUSSIANBLUR_OPTIMIZATION optimization, const D2D1_VECTOR_2F& blurAmount, D2D1_VECTOR_2F* scaleAmount)
	{
		g_funCBlurRenderingGraph_DeterminePreScale22000.call_org(drawingContext, input1, input2, optimization, blurAmount, scaleAmount);
		if (scaleAmount)
		{
			scaleAmount->x *= 0.5f;
			scaleAmount->y *= 0.5f;
		}
	}

	HRESULT CCustomBlur_Draw(DWM::Core::CCustomBlur* This, DWM::Core::CDrawingContext* drawingContext,
		const D2D1_RECT_F& destinationRect, const D2D1_POINT_2F* point, D2D1_INTERPOLATION_MODE interpolationMode,
		D2D1_COMPOSITE_MODE compositeMode)
	{
		This->GetDeviceContext()->DrawImage(
			This->GetDirectionalBlurYEffect(),
			nullptr,
			&destinationRect,
			D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
			D2D1_COMPOSITE_MODE_BOUNDED_SOURCE_COPY
		);

		return S_OK;
	}
}
