#pragma once
#include "DCompBackdrop.hpp"
#include "../Effects/CrossFadeEffect.hpp"
#include "../Effects/GaussianBlurEffect.hpp"
#include "../Effects/ColorSourceEffect.hpp"
#include "../Effects/OpacityEffect.hpp"
#include "../Effects/BlendEffect.hpp"
#include "../Effects/ExposureEffect.hpp"
#include "../Effects/CompositeEffect.hpp"

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
			// New Aero backdrop recipe by @ALTaleX531 (https://github.com/ALTaleX531/MDWMBlurGlassExt), @aubymori (normal layer fixes)
			// @kfh83 for porting to DWMBlurGlass and minor modifications

			auto colorEffect{ winrt::make_self<ColorSourceEffect>() };
			colorEffect->SetName(L"MainColor");
			colorEffect->SetColor(mainColor);

			auto colorOpacityEffect{ winrt::make_self<OpacityEffect>() };
			colorOpacityEffect->SetName(L"MainColorOpacity");
			colorOpacityEffect->SetInput(*colorEffect);
			colorOpacityEffect->SetOpacity(colorBalance);

			auto glowColorEffect{ winrt::make_self<ColorSourceEffect>() };
			glowColorEffect->SetName(L"GlowColor");
			glowColorEffect->SetColor(glowColor);

			auto glowOpacityEffect{ winrt::make_self<OpacityEffect>() };
			glowOpacityEffect->SetName(L"GlowColorOpacity");
			glowOpacityEffect->SetInput(*glowColorEffect);
			glowOpacityEffect->SetOpacity(glowBalance);

			auto blurredBackdropBalanceEffect{ winrt::make_self<ExposureEffect>() };
			blurredBackdropBalanceEffect->SetName(L"BlurBalance");
			blurredBackdropBalanceEffect->SetExposureAmount(blurBalance);

			if (hostBackdrop)
			{
				blurredBackdropBalanceEffect->SetInput(winrt::Windows::UI::Composition::CompositionEffectSourceParameter{ L"Backdrop" });
			}
			else
			{
				auto gaussianBlurEffect{ winrt::make_self<GaussianBlurEffect>() };
				gaussianBlurEffect->SetName(L"Blur");
				gaussianBlurEffect->SetBorderMode(D2D1_BORDER_MODE_HARD);
				gaussianBlurEffect->SetBlurAmount(blurAmount);
				gaussianBlurEffect->SetInput(winrt::Windows::UI::Composition::CompositionEffectSourceParameter{ L"Backdrop" });
				blurredBackdropBalanceEffect->SetInput(*gaussianBlurEffect);
			}

			auto glowBlendEffect{ winrt::make_self<BlendEffect>() };
			glowBlendEffect->SetBlendMode(D2D1_BLEND_MODE_MULTIPLY);
			glowBlendEffect->SetBackground(*blurredBackdropBalanceEffect);
			glowBlendEffect->SetForeground(*glowOpacityEffect);

			auto glowBalanceEffect{ winrt::make_self<ExposureEffect>() };
			glowBalanceEffect->SetName(L"GlowBalance");
			glowBalanceEffect->SetExposureAmount(glowBalance / 10.f);
			glowBalanceEffect->SetInput(*glowBlendEffect);

			auto compositeEffect{ winrt::make_self<CompositeStepEffect>() };
			compositeEffect->SetCompositeMode(D2D1_COMPOSITE_MODE_SOURCE_OVER);
			compositeEffect->SetName(L"Composite");
			compositeEffect->SetDestination(*glowBalanceEffect);
			compositeEffect->SetSource(*colorOpacityEffect);

			auto colorBalanceEffect{ winrt::make_self<ExposureEffect>() };
			colorBalanceEffect->SetName(L"ColorBalance");
			colorBalanceEffect->SetExposureAmount(colorBalance / 10.f);
			colorBalanceEffect->SetInput(*compositeEffect);

			auto effectBrush{ compositor.CreateEffectFactory(*colorBalanceEffect).CreateBrush() };
			if (hostBackdrop)
			{
				effectBrush.SetSourceParameter(L"Backdrop", compositor.CreateHostBackdropBrush());
			}
			else
			{
				effectBrush.SetSourceParameter(L"Backdrop", compositor.CreateBackdropBrush());
			}

			return effectBrush;
		}
		catch (...) { return nullptr; }

		void STDMETHODCALLTYPE ReloadParameters()
		{
			crossfadeTime = std::chrono::milliseconds{ g_configData.crossfadeTime };

			darkMode_Active_Color = MakeWinrtColor(g_configData.activeBlendColorDark);
			darkMode_Inactive_Color = MakeWinrtColor(g_configData.inactiveBlendColorDark);
			lightMode_Active_Color = MakeWinrtColor(g_configData.activeBlendColor);
			lightMode_Inactive_Color = MakeWinrtColor(g_configData.inactiveBlendColor);

			darkMode_Active_GlowColor = darkMode_Active_Color;
			darkMode_Inactive_GlowColor = darkMode_Inactive_Color;
			lightMode_Active_GlowColor = lightMode_Active_Color;
			lightMode_Inactive_GlowColor = lightMode_Inactive_Color;

			lightMode_Active_ColorBalance = g_configData.activeColorBalance;
			lightMode_Inactive_ColorBalance = g_configData.inactiveColorBalance;
			darkMode_Active_ColorBalance = g_configData.activeColorBalance;
			darkMode_Inactive_ColorBalance = g_configData.inactiveColorBalance;

			lightMode_Active_GlowBalance = GetFloatAlpha(g_configData.activeBlendColor);
			lightMode_Inactive_GlowBalance = GetFloatAlpha(g_configData.inactiveBlendColor);
			darkMode_Active_GlowBalance = GetFloatAlpha(g_configData.activeBlendColorDark);
			darkMode_Inactive_GlowBalance = GetFloatAlpha(g_configData.inactiveBlendColorDark);

			Active_BlurBalance = g_configData.activeBlurBalance;
			Inactive_BlurBalance = g_configData.inactiveBlurBalance;

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

	struct CAccentAeroResources : CDCompResourcesBase
	{

	};
	struct CAccentAeroBackdrop : CAccentDCompBackdrop
	{
		inline static CAccentAeroResources s_sharedResources{};
		winrt::Windows::UI::Color currenGlowColor{};

		HRESULT STDMETHODCALLTYPE UpdateBrush(const DWM::ACCENT_POLICY& policy) try
		{
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
						static_cast<float>(currenGlowColor.A) / 255.f,
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
