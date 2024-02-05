#pragma once
#include "CanvasEffect.hpp"

namespace MDWMBlurGlassExt
{
	class CrossFadeEffect : public CanvasEffect
	{
	public:
		CrossFadeEffect() : CanvasEffect{ CLSID_D2D1CrossFade }
		{
			m_namedProperties.emplace_back(
				L"Weight",
				D2D1_CROSSFADE_PROP_WEIGHT,
				NamedProperty::GRAPHICS_EFFECT_PROPERTY_MAPPING::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT
			);
			SetWeight();
		}
		virtual ~CrossFadeEffect() = default;

		void SetWeight(float weight = 0.5f)
		{
			SetProperty(D2D1_CROSSFADE_PROP_WEIGHT, BoxValue(weight));
		}
		void SetDestination(IGraphicsEffectSource* source)
		{
			SetInput(1, source);
		}
		void SetSource(IGraphicsEffectSource* source)
		{
			SetInput(0, source);
		}

		void SetDestination(const winrt::Windows::UI::Composition::CompositionEffectSourceParameter& source)
		{
			SetInput(1, source.as<IGraphicsEffectSource>().get());
		}
		void SetSource(const winrt::Windows::UI::Composition::CompositionEffectSourceParameter& source)
		{
			SetInput(0, source.as<IGraphicsEffectSource>().get());
		}

		void SetDestination(const winrt::com_ptr<IGraphicsEffectSource> source)
		{
			SetInput(1, source.get());
		}
		void SetSource(const winrt::com_ptr<IGraphicsEffectSource> source)
		{
			SetInput(0, source.get());
		}
	};
}