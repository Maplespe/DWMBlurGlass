#pragma once
#include "CanvasEffect.hpp"

namespace MDWMBlurGlassExt
{
	class OpacityEffect : public CanvasEffect
	{
	public:
		OpacityEffect() : CanvasEffect{ CLSID_D2D1Opacity }
		{
			SetOpacity();
		}
		virtual ~OpacityEffect() = default;

		void SetOpacity(float opacity = 1.0f)
		{
			SetProperty(D2D1_OPACITY_PROP_OPACITY, BoxValue(opacity));
		}
	};
}