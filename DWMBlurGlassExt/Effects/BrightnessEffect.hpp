#pragma once
#include "ColorMatrixEffect.hpp"

namespace AcrylicEverywhere
{
	class BrightnessEffect : public ColorMatrixEffect
	{
	public:
		BrightnessEffect() : ColorMatrixEffect{}
		{
			SetBrightnessAmount();
		}
		virtual ~BrightnessEffect() = default;

		void SetBrightnessAmount(float r = 1.f, float g = 1.f, float b = 1.f)
		{
			SetColorMatrix(
				D2D1::Matrix5x4F{
					r, 0.f, 0.f, 0.f,
					0.f, g, 0.f, 0.f,
					0.f, 0.f, b, 0.f,
					0.f, 0.f, 0.f, 1.f,
					0.f, 0.f, 0.f, 0.f
				}
			);
		}
	};
}