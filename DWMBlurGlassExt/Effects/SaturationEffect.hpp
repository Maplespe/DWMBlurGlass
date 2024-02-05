#pragma once
#include "CanvasEffect.hpp"

namespace MDWMBlurGlassExt
{
	class SaturationEffect : public CanvasEffect
	{
	public:
		SaturationEffect() : CanvasEffect{ CLSID_D2D1Saturation }
		{
			SetSaturation();
		}
		virtual ~SaturationEffect() = default;

		void SetSaturation(float saturation = 0.5f)
		{
			SetProperty(D2D1_SATURATION_PROP_SATURATION, BoxValue(saturation));
		}
	};
}