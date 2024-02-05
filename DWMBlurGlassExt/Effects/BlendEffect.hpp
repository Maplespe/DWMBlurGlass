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
		void SetBackground(ABI::Windows::Graphics::Effects::IGraphicsEffectSource* source)
		{
			SetInput(0, source);
		}
		void SetBackground(const winrt::com_ptr<ABI::Windows::Graphics::Effects::IGraphicsEffectSource> source)
		{
			SetInput(0, source.get());
		}
		void SetBackground(const winrt::Windows::UI::Composition::CompositionEffectSourceParameter& source)
		{
			SetInput(0, source.as<IGraphicsEffectSource>().get());
		}

		void SetForeground(ABI::Windows::Graphics::Effects::IGraphicsEffectSource* source)
		{
			SetInput(1, source);
		}
		void SetForeground(const winrt::com_ptr<ABI::Windows::Graphics::Effects::IGraphicsEffectSource> source)
		{
			SetInput(1, source.get());
		}
		void SetForeground(const winrt::Windows::UI::Composition::CompositionEffectSourceParameter& source)
		{
			SetInput(1, source.as<IGraphicsEffectSource>().get());
		}
	};
}