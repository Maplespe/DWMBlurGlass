#pragma once
#include "CanvasEffect.hpp"

namespace MDWMBlurGlassExt
{
	class BlendEffect : public CanvasEffect
	{
	public:
		BlendEffect() : CanvasEffect{ CLSID_D2D1Blend }
		{
			SetBlendMode();
		}
		virtual ~BlendEffect() = default;

		void SetBlendMode(D2D1_BLEND_MODE blendMode = D2D1_BLEND_MODE_MULTIPLY)
		{
			SetProperty(D2D1_BLEND_PROP_MODE, BoxValue(blendMode));
		}
		void SetBackground(const winrt::Windows::Graphics::Effects::IGraphicsEffectSource& source)
		{
			SetInput(0, source);
		}
		void SetBackground(const winrt::Windows::UI::Composition::CompositionEffectSourceParameter& source)
		{
			SetInput(0, source);
		}

		void SetForeground(const winrt::Windows::Graphics::Effects::IGraphicsEffectSource& source)
		{
			SetInput(1, source);
		}
		void SetForeground(const winrt::Windows::UI::Composition::CompositionEffectSourceParameter& source)
		{
			SetInput(1, source);
		}
	};
}