#pragma once
#include "CanvasEffect.hpp"

namespace MDWMBlurGlassExt
{
	class ColorSourceEffect : public CanvasEffect
	{
	public:
		ColorSourceEffect() : CanvasEffect{ CLSID_D2D1Flood }
		{
			SetColor();
		}
		virtual ~ColorSourceEffect() = default;

		void SetColor(const D2D1_COLOR_F& color = { 0.f, 0.f, 0.f, 1.f })
		{
			float value[] = { color.r, color.g, color.b, color.a };
			SetProperty(D2D1_FLOOD_PROP_COLOR, BoxValue(value));
		}
		void SetColor(const winrt::Windows::UI::Color& color)
		{
			SetColor(
				D2D1_COLOR_F
				{
					static_cast<float>(color.R) / 255.f,
					static_cast<float>(color.G) / 255.f,
					static_cast<float>(color.B) / 255.f,
					static_cast<float>(color.A) / 255.f
				}
			);
		}
	};
}