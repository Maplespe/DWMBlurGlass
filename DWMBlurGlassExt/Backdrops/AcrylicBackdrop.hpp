#pragma once
#include "DCompBackdrop.hpp"
#include "../Effects/ColorSourceEffect.hpp"
#include "../Effects/OpacityEffect.hpp"
#include "../Effects/BorderEffect.hpp"
#include "../Effects/BlendEffect.hpp"

namespace MDWMBlurGlassExt::AcrylicBackdrop
{
	double GetTintOpacityModifier(wu::Color tintColor)
	{
		// This method supresses the maximum allowable tint opacity depending on the luminosity and saturation of a color by 
		// compressing the range of allowable values - for example, a user-defined value of 100% will be mapped to 45% for pure 
		// white (100% luminosity), 85% for pure black (0% luminosity), and 90% for pure gray (50% luminosity).  The intensity of 
		// the effect increases linearly as luminosity deviates from 50%.  After this effect is calculated, we cancel it out
		// linearly as saturation increases from zero.

		constexpr double midPoint{ 0.50 }; // Mid point of HsvV range that these calculations are based on. This is here for easy tuning.

		constexpr double whiteMaxOpacity{ 0.45 }; // 100% luminosity
		constexpr double midPointMaxOpacity{ 0.90 }; // 50% luminosity
		constexpr double blackMaxOpacity{ 0.85 }; // 0% luminosity

		const auto rgb{ RgbFromColor(tintColor) };
		const auto hsv{ RgbToHsv(rgb) };

		double opacityModifier{ midPointMaxOpacity };

		if (hsv.v != midPoint)
		{
			// Determine maximum suppression amount
			double lowestMaxOpacity = midPointMaxOpacity;
			double maxDeviation = midPoint;

			if (hsv.v > midPoint)
			{
				lowestMaxOpacity = whiteMaxOpacity; // At white (100% hsvV)
				maxDeviation = 1 - maxDeviation;
			}
			else if (hsv.v < midPoint)
			{
				lowestMaxOpacity = blackMaxOpacity; // At black (0% hsvV)
			}

			double maxOpacitySuppression = midPointMaxOpacity - lowestMaxOpacity;

			// Determine normalized deviation from the midpoint
			const double deviation = std::abs(hsv.v - midPoint);
			const double normalizedDeviation = deviation / maxDeviation;

			// If we have saturation, reduce opacity suppression to allow that color to come through more
			if (hsv.s > 0)
			{
				// Dampen opacity suppression based on how much saturation there is
#pragma push_macro("max")
#undef max
				maxOpacitySuppression *= std::max(1 - (hsv.s * 2), 0.0);
#pragma pop_macro("max")
			}

			const double opacitySuppression = maxOpacitySuppression * normalizedDeviation;

			opacityModifier = midPointMaxOpacity - opacitySuppression;
		}

		return opacityModifier;
	}

	wu::Color GetEffectiveTintColor(wu::Color tintColor, float tintOpacity, std::optional<float> luminosityOpacity)
	{
		if (luminosityOpacity)
		{
			tintColor.A = static_cast<BYTE>(round(tintColor.A * tintOpacity));
		}
		else
		{
			const double tintOpacityModifier{ GetTintOpacityModifier(tintColor) };
			tintColor.A = static_cast<BYTE>(round(tintColor.A * tintOpacity * tintOpacityModifier));
		}

		return tintColor;
	}

	// The tintColor passed into this method should be the original, unmodified color created using user values for TintColor + TintOpacity
	wu::Color GetLuminosityColor(wu::Color tintColor, std::optional<double> luminosityOpacity)
	{
		const Rgb rgbTintColor = RgbFromColor(tintColor);

		// If luminosity opacity is specified, just use the values as is
		if (luminosityOpacity)
		{
			return ColorFromRgba(rgbTintColor, std::clamp(luminosityOpacity.value(), 0.0, 1.0));
		}
		else
		{
			// To create the Luminosity blend input color without luminosity opacity,
			// we're taking the TintColor input, converting to HSV, and clamping the V between these values
			const double minHsvV = 0.125;
			const double maxHsvV = 0.965;

			const Hsv hsvTintColor = RgbToHsv(rgbTintColor);

			const auto clampedHsvV = std::clamp(hsvTintColor.v, minHsvV, maxHsvV);

			const Hsv hsvLuminosityColor = Hsv(hsvTintColor.h, hsvTintColor.s, clampedHsvV);
			const Rgb rgbLuminosityColor = HsvToRgb(hsvLuminosityColor);

			// Now figure out luminosity opacity
			// Map original *tint* opacity to this range
			const double minLuminosityOpacity = 0.15;
			const double maxLuminosityOpacity = 1.03;

			const double luminosityOpacityRangeMax = maxLuminosityOpacity - minLuminosityOpacity;
			const double mappedTintOpacity = ((tintColor.A / 255.0) * luminosityOpacityRangeMax) + minLuminosityOpacity;

			// Finally, combine the luminosity opacity and the HsvV-clamped tint color
#pragma push_macro("min")
#undef min
			return ColorFromRgba(rgbLuminosityColor, std::min(mappedTintOpacity, 1.0));
#pragma pop_macro("min")
		}

	}

	wu::Color GetEffectiveLuminosityColor(wu::Color tintColor, float tintOpacity, std::optional<float> luminosityOpacity)
	{
		// Purposely leaving out tint opacity modifier here because GetLuminosityColor needs the *original* tint opacity set by the user.
		tintColor.A = static_cast<uint8_t>(round(tintColor.A * tintOpacity));

		return GetLuminosityColor(tintColor, luminosityOpacity);
	}

	wuc::CompositionBrush CreateBrush(
		const wuc::Compositor& compositor,
		const wuc::CompositionSurfaceBrush& noiceBrush,
		const wu::Color& tintColor,
		const wu::Color& luminosityColor,
		float blurAmount,
		float noiceAmount
	)
	{
		if (tintColor.A == 255)
		{
			return compositor.CreateColorBrush(tintColor);
		}

		auto tintColorEffect{ winrt::make_self<ColorSourceEffect>() };
		tintColorEffect->SetName(L"TintColor");
		tintColorEffect->SetColor(tintColor);

		auto luminosityColorEffect{ winrt::make_self<ColorSourceEffect>() };
		luminosityColorEffect->SetName(L"LuminosityColor");
		luminosityColorEffect->SetColor(luminosityColor);

		auto luminosityBlendEffect{ winrt::make_self<BlendEffect>() };
		// NOTE: There is currently a bug where the names of BlendEffectMode::Luminosity and BlendEffectMode::Color are flipped->
		// This should be changed to Luminosity when/if the bug is fixed->
		luminosityBlendEffect->SetBlendMode(D2D1_BLEND_MODE_COLOR);
		luminosityBlendEffect->SetBackground(CreateBlurredBackdrop(blurAmount));
		luminosityBlendEffect->SetForeground(*luminosityColorEffect);

		auto colorBlendEffect{ winrt::make_self<BlendEffect>() };
		// NOTE: There is currently a bug where the names of BlendEffectMode::Luminosity and BlendEffectMode::Color are flipped->
		// This should be changed to Color when/if the bug is fixed->
		colorBlendEffect->SetBlendMode(D2D1_BLEND_MODE_LUMINOSITY);
		colorBlendEffect->SetBackground(*luminosityBlendEffect);
		colorBlendEffect->SetForeground(*tintColorEffect);

		auto noiseBorderEffect{ winrt::make_self<BorderEffect>() };
		noiseBorderEffect->SetExtendX(D2D1_BORDER_EDGE_MODE_WRAP);
		noiseBorderEffect->SetExtendY(D2D1_BORDER_EDGE_MODE_WRAP);
		wu::Composition::CompositionEffectSourceParameter noiseEffectSourceParameter{ L"Noise" };
		noiseBorderEffect->SetInput(noiseEffectSourceParameter);

		auto noiseOpacityEffect{ winrt::make_self<OpacityEffect>() };
		noiseOpacityEffect->SetName(L"NoiseOpacity");
		noiseOpacityEffect->SetOpacity(noiceAmount);
		noiseOpacityEffect->SetInput(*noiseBorderEffect);

		auto blendEffectOuter{ winrt::make_self<BlendEffect>() };
		blendEffectOuter->SetBlendMode(D2D1_BLEND_MODE_MULTIPLY);
		blendEffectOuter->SetBackground(*colorBlendEffect);
		blendEffectOuter->SetForeground(*noiseOpacityEffect);

		auto effectBrush{ compositor.CreateEffectFactory(*blendEffectOuter).CreateBrush() };
		effectBrush.SetSourceParameter(L"Noise", noiceBrush);
		effectBrush.SetSourceParameter(L"Backdrop", compositor.CreateBackdropBrush());

		return effectBrush;
	}
	static winrt::Windows::UI::Composition::CompositionSurfaceBrush CreateNoiceSurfaceBrush(
		const winrt::Windows::UI::Composition::Compositor& compositor,
		ID2D1Device* d2dDevice
	) try
	{
		winrt::Windows::UI::Composition::CompositionGraphicsDevice graphicsDevice{ nullptr };
		THROW_IF_FAILED(
			compositor.as<ABI::Windows::UI::Composition::ICompositorInterop>()->CreateGraphicsDevice(
				d2dDevice,
				reinterpret_cast<ABI::Windows::UI::Composition::ICompositionGraphicsDevice**>(winrt::put_abi(graphicsDevice))
			)
		);
		auto compositionSurface
		{
			graphicsDevice.CreateDrawingSurface(
				{ 256.f, 256.f },
				winrt::Windows::Graphics::DirectX::DirectXPixelFormat::R16G16B16A16Float,
				winrt::Windows::Graphics::DirectX::DirectXAlphaMode::Premultiplied
			)
		};
		auto noiceBrush{ compositor.CreateSurfaceBrush(compositionSurface) };

		wil::unique_hmodule wuxcModule{ LoadLibraryExW(L"Windows.UI.Xaml.Controls.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE) };
		THROW_LAST_ERROR_IF_NULL(wuxcModule);
		auto resourceHandle{ FindResourceW(wuxcModule.get(), MAKEINTRESOURCE(2000), RT_RCDATA) };
		THROW_LAST_ERROR_IF_NULL(resourceHandle);
		auto globalHandle{ LoadResource(wuxcModule.get(), resourceHandle) };
		THROW_LAST_ERROR_IF_NULL(globalHandle);
		auto cleanUp = wil::scope_exit([&]
			{
				if (globalHandle)
				{
					UnlockResource(globalHandle);
					FreeResource(globalHandle);
				}
			});
		DWORD resourceSize{ SizeofResource(wuxcModule.get(), resourceHandle) };
		THROW_LAST_ERROR_IF(resourceSize == 0);
		auto resourceAddress{ reinterpret_cast<PBYTE>(LockResource(globalHandle)) };
		winrt::com_ptr<IStream> stream{ SHCreateMemStream(resourceAddress, resourceSize), winrt::take_ownership_from_abi };
		THROW_LAST_ERROR_IF_NULL(stream);

		winrt::com_ptr<IWICImagingFactory2> wicFactory{ nullptr };
		wicFactory.copy_from(DWM::CDesktopManager::s_pDesktopManagerInstance->GetWICFactory());
		winrt::com_ptr<IWICBitmapDecoder> wicDecoder{ nullptr };
		THROW_IF_FAILED(wicFactory->CreateDecoderFromStream(stream.get(), &GUID_VendorMicrosoft, WICDecodeMetadataCacheOnDemand, wicDecoder.put()));
		winrt::com_ptr<IWICBitmapFrameDecode> wicFrame{ nullptr };
		THROW_IF_FAILED(wicDecoder->GetFrame(0, wicFrame.put()));
		winrt::com_ptr<IWICFormatConverter> wicConverter{ nullptr };
		THROW_IF_FAILED(wicFactory->CreateFormatConverter(wicConverter.put()));
		winrt::com_ptr<IWICPalette> wicPalette{ nullptr };
		THROW_IF_FAILED(
			wicConverter->Initialize(
				wicFrame.get(),
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				wicPalette.get(),
				0, WICBitmapPaletteTypeCustom
			)
		);
		winrt::com_ptr<IWICBitmap> wicBitmap{ nullptr };
		THROW_IF_FAILED(wicFactory->CreateBitmapFromSource(wicConverter.get(), WICBitmapCreateCacheOption::WICBitmapNoCache, wicBitmap.put()));

		auto drawingSurfaceInterop{ compositionSurface.as<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop>() };
		POINT offset = { 0, 0 };
		winrt::com_ptr<ID2D1DeviceContext> d2dContext{ nullptr };
		THROW_IF_FAILED(
			drawingSurfaceInterop->BeginDraw(nullptr, IID_PPV_ARGS(d2dContext.put()), &offset)
		);
		d2dContext->Clear();
		winrt::com_ptr<ID2D1Bitmap1> d2dBitmap{ nullptr };
		d2dContext->CreateBitmapFromWicBitmap(
			wicBitmap.get(),
			D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_NONE,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
			),
			d2dBitmap.put()
		);
		d2dContext->DrawBitmap(d2dBitmap.get());
		THROW_IF_FAILED(
			drawingSurfaceInterop->EndDraw()
		);

		return noiceBrush;
	}
	catch (...) { return nullptr; }
}