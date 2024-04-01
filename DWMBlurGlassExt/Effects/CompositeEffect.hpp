#pragma once
#include "CanvasEffect.hpp"

namespace MDWMBlurGlassExt
{
	class CompositeEffect : public CanvasEffect
	{
	public:
		CompositeEffect() : CanvasEffect{ CLSID_D2D1Composite }
		{
			SetCompositeMode();
		}
		virtual ~CompositeEffect() = default;

		void SetCompositeMode(D2D1_COMPOSITE_MODE compositeMode = D2D1_COMPOSITE_MODE_SOURCE_OVER)
		{
			SetProperty(D2D1_COMPOSITE_PROP_MODE, BoxValue(compositeMode));
		}
	};

	class CompositeStepEffect : public CanvasEffect
	{
	public:
		CompositeStepEffect() : CanvasEffect{ CLSID_D2D1Composite }
		{
			SetCompositeMode();
		}
		virtual ~CompositeStepEffect() = default;

		void SetCompositeMode(D2D1_COMPOSITE_MODE compositeMode = D2D1_COMPOSITE_MODE_SOURCE_OVER)
		{
			SetProperty(D2D1_COMPOSITE_PROP_MODE, BoxValue(compositeMode));
		}
		void SetDestination(const winrt::Windows::Graphics::Effects::IGraphicsEffectSource& source)
		{
			SetInput(0, source);
		}
		void SetDestination(const winrt::Windows::UI::Composition::CompositionEffectSourceParameter& source)
		{
			SetInput(0, source);
		}
		void SetSource(const winrt::Windows::Graphics::Effects::IGraphicsEffectSource& source)
		{
			SetInput(1, source);
		}
		void SetSource(const winrt::Windows::UI::Composition::CompositionEffectSourceParameter& source)
		{
			SetInput(1, source);
		}
	};
}