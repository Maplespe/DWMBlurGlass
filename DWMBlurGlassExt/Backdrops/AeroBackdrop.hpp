#pragma once
#include "DCompBackdrop.hpp"
#include "../Effects/CrossFadeEffect.hpp"
#include "../Effects/GaussianBlurEffect.hpp"
#include "../Effects/ColorSourceEffect.hpp"
#include "../Effects/OpacityEffect.hpp"
#include "../Effects/BlendEffect.hpp"
#include "../Effects/ExposureEffect.hpp"
#include "../Effects/CompositeEffect.hpp"
#include "../Effects/SaturationEffect.hpp"
#include "../Effects/TintEffect.hpp"

namespace MDWMBlurGlassExt
{
	struct CAeroResources : CDCompResources
	{
		winrt::Windows::UI::Color lightMode_Active_GlowColor{};
		winrt::Windows::UI::Color darkMode_Active_GlowColor{};
		winrt::Windows::UI::Color lightMode_Inactive_GlowColor{};
		winrt::Windows::UI::Color darkMode_Inactive_GlowColor{};

		float lightMode_Active_ColorBalance{};
		float lightMode_Inactive_ColorBalance{};
		float darkMode_Active_ColorBalance{};
		float darkMode_Inactive_ColorBalance{};

		float lightMode_Active_GlowBalance{};
		float lightMode_Inactive_GlowBalance{};
		float darkMode_Active_GlowBalance{};
		float darkMode_Inactive_GlowBalance{};

		float Active_BlurBalance{};
		float Inactive_BlurBalance{};

		float blurAmount{ 3.f };
		bool hostBackdrop{ false };

		static winrt::Windows::UI::Composition::CompositionBrush CreateBrush(
			const winrt::Windows::UI::Composition::Compositor& compositor,
			const winrt::Windows::UI::Color& mainColor,
			const winrt::Windows::UI::Color& glowColor,
			float colorBalance,
			float glowBalance,
			float blurAmount,
			float blurBalance,
			bool hostBackdrop
		) try
		{
			// the current recipe is modified from @kfh83, @TorutheRedFox, @aubymori
			auto fallbackTintSource{ winrt::make_self<ColorSourceEffect>() };
			fallbackTintSource->SetColor(winrt::Windows::UI::Color
			{
				255,
				static_cast<UCHAR>(min(blurBalance + 0.1f, 1.f) * 255.f),
				static_cast<UCHAR>(min(blurBalance + 0.1f, 1.f) * 255.f),
				static_cast<UCHAR>(min(blurBalance + 0.1f, 1.f) * 255.f),
			});

			auto blackOrTransparentSource{ winrt::make_self<TintEffect>() };
			blackOrTransparentSource->SetInput(winrt::Windows::UI::Composition::CompositionEffectSourceParameter{ L"Backdrop" });
			blackOrTransparentSource->SetColor(D2D1::ColorF(D2D1::ColorF::Black));

			auto colorEffect{ winrt::make_self<ColorSourceEffect>() };
			colorEffect->SetName(L"MainColor");
			colorEffect->SetColor(mainColor);

			auto colorOpacityEffect{ winrt::make_self<OpacityEffect>() };
			colorOpacityEffect->SetName(L"MainColorOpacity");
			colorOpacityEffect->SetInput(*colorEffect);
			colorOpacityEffect->SetOpacity(colorBalance);

			auto backdropBalanceEffect{ winrt::make_self<OpacityEffect>() };
			backdropBalanceEffect->SetName(L"BlurBalance");
			backdropBalanceEffect->SetOpacity(blurBalance);
			backdropBalanceEffect->SetInput(winrt::Windows::UI::Composition::CompositionEffectSourceParameter{ L"Backdrop" });

			auto actualBackdropEffect{ winrt::make_self<CompositeStepEffect>() };
			actualBackdropEffect->SetCompositeMode(D2D1_COMPOSITE_MODE_PLUS);
			actualBackdropEffect->SetDestination(*blackOrTransparentSource);
			actualBackdropEffect->SetSource(*backdropBalanceEffect);

			auto desaturatedBackdrop{ winrt::make_self<SaturationEffect>() };
			desaturatedBackdrop->SetSaturation(0.f);
			desaturatedBackdrop->SetInput(winrt::Windows::UI::Composition::CompositionEffectSourceParameter{ L"Backdrop" });

			// make animation feel better...
			auto backdropNotTransparentPromised{ winrt::make_self<CompositeStepEffect>() };
			backdropNotTransparentPromised->SetCompositeMode(D2D1_COMPOSITE_MODE_SOURCE_OVER);
			backdropNotTransparentPromised->SetDestination(*fallbackTintSource);
			backdropNotTransparentPromised->SetSource(*desaturatedBackdrop);

			// if the glowColor is black, then it will produce a completely transparent surface
			auto tintEffect{ winrt::make_self<TintEffect>() };
			tintEffect->SetInput(*backdropNotTransparentPromised);
			tintEffect->SetColor(winrt::Windows::UI::Color{ static_cast<UCHAR>(static_cast<float>(glowColor.A) * glowBalance), glowColor.R, glowColor.G, glowColor.B });

			auto backdropWithAfterGlow{ winrt::make_self<CompositeStepEffect>() };
			backdropWithAfterGlow->SetCompositeMode(D2D1_COMPOSITE_MODE_PLUS);
			backdropWithAfterGlow->SetDestination(*actualBackdropEffect);
			backdropWithAfterGlow->SetSource(*tintEffect);

			auto compositeEffect{ winrt::make_self<CompositeStepEffect>() };
			compositeEffect->SetCompositeMode(D2D1_COMPOSITE_MODE_PLUS);
			compositeEffect->SetDestination(*backdropWithAfterGlow);
			compositeEffect->SetSource(*colorOpacityEffect);

			auto gaussianBlurEffect{ winrt::make_self<GaussianBlurEffect>() };
			gaussianBlurEffect->SetName(L"Blur");
			gaussianBlurEffect->SetBorderMode(D2D1_BORDER_MODE_HARD);
			gaussianBlurEffect->SetBlurAmount(blurAmount);
			gaussianBlurEffect->SetInput(*compositeEffect);

			auto effectBrush{ compositor.CreateEffectFactory(*gaussianBlurEffect).CreateBrush() };
			effectBrush.SetSourceParameter(L"Backdrop", compositor.CreateBackdropBrush());

			return effectBrush;
		}
		catch (...) { return nullptr; }

		void STDMETHODCALLTYPE ReloadParameters() override
		{
			crossfadeTime = std::chrono::milliseconds{ g_configData.crossfadeTime };

			if (g_configData.useAccentColor)
			{
				darkMode_Active_Color = MakeWinrtColor(g_accentColor);
				darkMode_Inactive_Color = darkMode_Active_Color;
				lightMode_Active_Color = darkMode_Inactive_Color;
				lightMode_Inactive_Color = lightMode_Active_Color;
			}
			else
			{
				darkMode_Active_Color = MakeWinrtColor(g_configData.activeBlendColorDark);
				darkMode_Inactive_Color = MakeWinrtColor(g_configData.inactiveBlendColorDark);
				lightMode_Active_Color = MakeWinrtColor(g_configData.activeBlendColor);
				lightMode_Inactive_Color = MakeWinrtColor(g_configData.inactiveBlendColor);
			}

			darkMode_Active_GlowColor = darkMode_Active_Color;
			darkMode_Inactive_GlowColor = darkMode_Inactive_Color;
			lightMode_Active_GlowColor = lightMode_Active_Color;
			lightMode_Inactive_GlowColor = lightMode_Inactive_Color;

			// please altalex keep this as is or atleast try to keep this behavior

			lightMode_Active_ColorBalance = g_configData.aeroColorBalance;
			lightMode_Inactive_ColorBalance = lightMode_Active_ColorBalance * 0.4f;
			darkMode_Active_ColorBalance = g_configData.aeroColorBalance;
			darkMode_Inactive_ColorBalance = darkMode_Active_ColorBalance * 0.4f;

			lightMode_Active_GlowBalance = g_configData.aeroAfterglowBalance;
			lightMode_Inactive_GlowBalance = g_configData.aeroAfterglowBalance;
			darkMode_Active_GlowBalance = g_configData.aeroAfterglowBalance;
			darkMode_Inactive_GlowBalance = g_configData.aeroAfterglowBalance;

			Active_BlurBalance = g_configData.aeroBlurBalance;
			Inactive_BlurBalance = (Active_BlurBalance * 0.4f) + 0.6f;

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
					lightMode_Active_GlowColor,
					lightMode_Active_ColorBalance,
					lightMode_Active_GlowBalance,
					blurAmount,
					Active_BlurBalance,
					hostBackdrop
				);
				lightMode_Inactive_Brush = CreateBrush(
					compositor,
					lightMode_Inactive_Color,
					lightMode_Inactive_GlowColor,
					lightMode_Inactive_ColorBalance,
					lightMode_Inactive_GlowBalance,
					blurAmount,
					Inactive_BlurBalance,
					hostBackdrop
				);
				darkMode_Active_Brush = CreateBrush(
					compositor,
					darkMode_Active_Color,
					darkMode_Active_GlowColor,
					darkMode_Active_ColorBalance,
					darkMode_Active_GlowBalance,
					blurAmount,
					Active_BlurBalance,
					hostBackdrop
				);
				darkMode_Inactive_Brush = CreateBrush(
					compositor,
					darkMode_Inactive_Color,
					darkMode_Inactive_GlowColor,
					darkMode_Inactive_ColorBalance,
					darkMode_Inactive_GlowBalance,
					blurAmount,
					Inactive_BlurBalance,
					hostBackdrop
				);
			}

			return CDCompResources::GetBrush(useDarkMode, windowActivated);
		}
		catch (...) { return nullptr; }
	};

	struct CAeroBackdrop : CDCompBackdrop
	{
		inline static CAeroResources s_sharedResources{};

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
		) override try
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

	struct CAccentAeroResources : CDCompResourcesBase
	{

	};
	struct CAccentAeroBackdrop : CAccentDCompBackdrop
	{
		inline static CAccentAeroResources s_sharedResources{};
		winrt::Windows::UI::Color currenGlowColor{};

		HRESULT STDMETHODCALLTYPE UpdateBrush(const DWM::ACCENT_POLICY& policy) try
		{
			s_sharedResources.ReloadParameters();
			s_sharedResources.interopDCompDevice.copy_from(
				DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice()
			);
			auto compositor{ spriteVisual.Compositor() };
			auto glowColor{ FromAbgr(policy.nColor) };
			if (
				currenGlowColor != glowColor ||
				interopDCompDevice != s_sharedResources.interopDCompDevice
			)
			{
				interopDCompDevice = s_sharedResources.interopDCompDevice;
				currenGlowColor = glowColor;

				spriteVisual.Brush(
					CAeroResources::CreateBrush(
						compositor,
						CAeroBackdrop::s_sharedResources.lightMode_Active_Color,
						{ 255, currenGlowColor.R, currenGlowColor.G, currenGlowColor.B },
						CAeroBackdrop::s_sharedResources.lightMode_Active_ColorBalance,
						CAeroBackdrop::s_sharedResources.lightMode_Active_GlowBalance,
						CAeroBackdrop::s_sharedResources.blurAmount,
						CAeroBackdrop::s_sharedResources.Active_BlurBalance,
						CAeroBackdrop::s_sharedResources.hostBackdrop
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
