#pragma once
#include "../Effects/CrossFadeEffect.hpp"
#include "../Effects/GaussianBlurEffect.hpp"
#include "../Effects/ColorSourceEffect.hpp"
#include "../Effects/OpacityEffect.hpp"

namespace MDWMBlurGlassExt::BlurBackdrop
{
	wuc::CompositionBrush CreateBrush(
		const wuc::Compositor& compositor,
		const wu::Color& tintColor,
		float tintOpacity,
		float blurAmount
	)
	{
		if (static_cast<float>(tintColor.A) * tintOpacity == 255.f)
		{
			return compositor.CreateColorBrush(tintColor);
		}

		if (!blurAmount)
		{
			return compositor.CreateColorBrush({ static_cast<BYTE>(static_cast<float>(tintColor.A) * tintOpacity), tintColor.R, tintColor.G, tintColor.B });
		}

		auto gaussianBlurEffect{ winrt::make_self<GaussianBlurEffect>() };
		gaussianBlurEffect->SetName(L"Blur");
		gaussianBlurEffect->SetBorderMode(D2D1_BORDER_MODE_HARD);
		gaussianBlurEffect->SetBlurAmount(blurAmount);
		gaussianBlurEffect->SetOptimizationMode(D2D1_GAUSSIANBLUR_OPTIMIZATION_SPEED);
		gaussianBlurEffect->SetInput(wuc::CompositionEffectSourceParameter{ L"Backdrop" });
		if (static_cast<float>(tintColor.A) * tintOpacity == 0.f)
		{
			auto effectBrush{ compositor.CreateEffectFactory(*gaussianBlurEffect).CreateBrush() };
			effectBrush.SetSourceParameter(L"Backdrop", compositor.CreateBackdropBrush());
			return effectBrush;
		}

		auto tintColorEffect{ winrt::make_self<ColorSourceEffect>() };
		tintColorEffect->SetName(L"TintColor");
		tintColorEffect->SetColor(tintColor);

		auto tintOpacityEffect{ winrt::make_self<OpacityEffect>() };
		tintOpacityEffect->SetName(L"TintOpacity");
		tintOpacityEffect->SetOpacity(tintOpacity);
		tintOpacityEffect->SetInput(*tintColorEffect);

		auto compositeStepEffect{ winrt::make_self<CompositeStepEffect>() };
		compositeStepEffect->SetDestination(*gaussianBlurEffect);
		compositeStepEffect->SetSource(*tintOpacityEffect);

			blurAmount = g_configData.customBlurAmount;
		}
		winrt::Windows::UI::Composition::CompositionBrush STDMETHODCALLTYPE GetBrush(bool useDarkMode, bool windowActivated) override try
		{
			auto is_device_valid = [&]()
			{
				if (!interopDCompDevice) return false;

				BOOL valid{ FALSE };
				THROW_IF_FAILED(
					interopDCompDevice.as<IDCompositionDevice>()->CheckDeviceState(
						&valid
					)
				);

				return valid == TRUE;
			};
			if (!is_device_valid())
			{
				interopDCompDevice.copy_from(
					DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice()
				);
				ReloadParameters();
				auto compositor{ interopDCompDevice.as<winrt::Windows::UI::Composition::Compositor>() };

				lightMode_Active_Brush = CreateBrush(
					compositor,
					lightMode_Active_Color,
					lightMode_Active_TintOpacity,
					blurAmount,
					hostBackdrop
				);
				lightMode_Inactive_Brush = CreateBrush(
					compositor,
					lightMode_Inactive_Color,
					lightMode_Inactive_TintOpacity,
					blurAmount,
					hostBackdrop
				);
				darkMode_Active_Brush = CreateBrush(
					compositor,
					darkMode_Active_Color,
					darkMode_Active_TintOpacity,
					blurAmount,
					hostBackdrop
				);
				darkMode_Inactive_Brush = CreateBrush(
					compositor,
					darkMode_Inactive_Color,
					darkMode_Inactive_TintOpacity,
					blurAmount,
					hostBackdrop
				);
			}

			return CDCompResources::GetBrush(useDarkMode, windowActivated);
		}
		catch (...) { return nullptr; }
	};

	struct CBlurBackdrop : CDCompBackdrop
	{
		inline static CBlurResources s_sharedResources{};

		STDMETHOD(UpdateColorizationColor)(
			bool useDarkMode,
			bool windowActivated
			) override
		{
			if (useDarkMode)
			{
				if (windowActivated) { currentColor = s_sharedResources.darkMode_Active_Color; }
				else { currentColor = s_sharedResources.darkMode_Inactive_Color; }
			}
			else
			{
				if (windowActivated) { currentColor = s_sharedResources.lightMode_Active_Color; }
				else { currentColor = s_sharedResources.lightMode_Inactive_Color; }
			}

			return S_OK;
		}
		HRESULT STDMETHODCALLTYPE Update(
			bool useDarkMode,
			bool windowActivated
		) try
		{
			THROW_IF_FAILED(UpdateColorizationColor(useDarkMode, windowActivated));
			THROW_IF_FAILED(
				TryCrossFadeToNewBrush(
					spriteVisual.Compositor(),
					s_sharedResources.GetBrush(useDarkMode, windowActivated),
					s_sharedResources.crossfadeTime
				)
			);

			return S_OK;
		}
		CATCH_RETURN()
	};

	struct CAccentBlurResources : CDCompResourcesBase
	{

	};
	struct CAccentBlurBackdrop : CAccentDCompBackdrop
	{
		inline static CAccentBlurResources s_sharedResources{};
		winrt::Windows::UI::Color currenTintColor{};

		HRESULT STDMETHODCALLTYPE UpdateBrush(const DWM::ACCENT_POLICY& policy) try
		{
			s_sharedResources.ReloadParameters();
			s_sharedResources.interopDCompDevice.copy_from(
				DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice()
			);
			auto compositor{ spriteVisual.Compositor() };
			auto tintColor{ FromAbgr(policy.nColor) };
			if (
				currenTintColor != tintColor ||
				interopDCompDevice != s_sharedResources.interopDCompDevice
			)
			{
				interopDCompDevice = s_sharedResources.interopDCompDevice;
				currenTintColor = tintColor;

				spriteVisual.Brush(
					CBlurResources::CreateBrush(
						compositor,
						tintColor,
						1.f,
						CBlurBackdrop::s_sharedResources.blurAmount,
						CBlurBackdrop::s_sharedResources.hostBackdrop
					)
				);
			}

			return S_OK;
		}
		CATCH_RETURN()

		HRESULT STDMETHODCALLTYPE Update(const DWM::ACCENT_POLICY& policy) override try
		{
			THROW_IF_FAILED(UpdateBrush(policy));

			return S_OK;
		}
		CATCH_RETURN()
	};
}