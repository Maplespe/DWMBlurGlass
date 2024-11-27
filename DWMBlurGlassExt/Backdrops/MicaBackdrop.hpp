#pragma once
#include "../Effects/CrossFadeEffect.hpp"
#include "../Effects/ColorSourceEffect.hpp"
#include "../Effects/OpacityEffect.hpp"
#include "../Effects/BlendEffect.hpp"

namespace MDWMBlurGlassExt::MicaBackdrop
{
	wuc::CompositionBrush CreateBrush(
		const wuc::Compositor& compositor,
		const wu::Color& tintColor,
		const wu::Color& luminosityColor,
		float tintOpacity,
		float luminosityOpacity
	)
	{
		if (static_cast<float>(tintColor.A) * tintOpacity == 255.f)
		{
			return compositor.CreateColorBrush(tintColor);
		}

		auto tintColorEffect{ winrt::make_self<ColorSourceEffect>() };
		tintColorEffect->SetName(L"TintColor");
		tintColorEffect->SetColor(tintColor);

		auto tintOpacityEffect{ winrt::make_self<OpacityEffect>() };
		tintOpacityEffect->SetName(L"TintOpacity");
		tintOpacityEffect->SetOpacity(tintOpacity);
		tintOpacityEffect->SetInput(*tintColorEffect);

		auto luminosityColorEffect{ winrt::make_self<ColorSourceEffect>() };
		luminosityColorEffect->SetColor(luminosityColor);

		auto luminosityOpacityEffect{ winrt::make_self<OpacityEffect>() };
		luminosityOpacityEffect->SetName(L"LuminosityOpacity");
		luminosityOpacityEffect->SetOpacity(luminosityOpacity);
		luminosityOpacityEffect->SetInput(*luminosityColorEffect);

		auto luminosityBlendEffect{ winrt::make_self<BlendEffect>() };
		luminosityBlendEffect->SetBlendMode(D2D1_BLEND_MODE_COLOR);
		luminosityBlendEffect->SetBackground(wuc::CompositionEffectSourceParameter{ L"BlurredWallpaperBackdrop" });
		luminosityBlendEffect->SetForeground(*luminosityOpacityEffect);

		auto colorBlendEffect{ winrt::make_self<BlendEffect>() };
		colorBlendEffect->SetBlendMode(D2D1_BLEND_MODE_LUMINOSITY);
		colorBlendEffect->SetBackground(*luminosityBlendEffect);
		colorBlendEffect->SetForeground(*tintOpacityEffect);

		auto effectBrush{ compositor.CreateEffectFactory(*colorBlendEffect).CreateBrush() };
		effectBrush.SetSourceParameter(L"BlurredWallpaperBackdrop", compositor.TryCreateBlurredWallpaperBackdropBrush());

		return effectBrush;
	}
}