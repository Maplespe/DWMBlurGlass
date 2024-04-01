#pragma once
#include "CanvasEffect.hpp"

namespace AcrylicEverywhere
{
	class ColorMatrixEffect : public CanvasEffect
	{
	public:
		ColorMatrixEffect() : CanvasEffect{ CLSID_D2D1ColorMatrix }
		{
			SetAlphaMode();
		}
		virtual ~ColorMatrixEffect() = default;

		void SetColorMatrix(const D2D1_MATRIX_5X4_F& colorMatrix)
		{
			SetProperty(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, BoxValue(colorMatrix));
		}
		void SetAlphaMode(D2D1_COLORMATRIX_ALPHA_MODE alphaMode = D2D1_COLORMATRIX_ALPHA_MODE_PREMULTIPLIED)
		{
			SetProperty(D2D1_COLORMATRIX_PROP_ALPHA_MODE, BoxValue(alphaMode));
		}
	};
}