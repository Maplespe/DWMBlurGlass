#pragma once
#include "CanvasEffect.hpp"

namespace MDWMBlurGlassExt
{
	class ExposureEffect : public CanvasEffect
	{
	public:
		ExposureEffect() : CanvasEffect{ CLSID_D2D1Exposure }
		{
			SetExposure();
		}
		virtual ~ExposureEffect() = default;

		void SetExposure(float exposure = 1.f)
		{
			SetProperty(D2D1_EXPOSURE_PROP_EXPOSURE_VALUE, BoxValue(exposure));
		}
	};
}