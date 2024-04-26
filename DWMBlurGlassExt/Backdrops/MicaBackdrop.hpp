#pragma once
#include "DCompBackdrop.hpp"
#include "../Effects/CrossFadeEffect.hpp"
#include "../Effects/GaussianBlurEffect.hpp"
#include "../Effects/ColorSourceEffect.hpp"
#include "../Effects/OpacityEffect.hpp"
#include "../Effects/BlendEffect.hpp"
#include "../Common/VersionHelper.h"

namespace MDWMBlurGlassExt
{
	struct CMicaResources : CDCompResources
	{
		float lightMode_Active_TintOpacity{};
		float lightMode_Inactive_TintOpacity{};
		float darkMode_Active_TintOpacity{};
		float darkMode_Inactive_TintOpacity{};
		float lightMode_Active_LuminosityOpacity{};
		float lightMode_Inactive_LuminosityOpacity{};
		float darkMode_Active_LuminosityOpacity{};
		float darkMode_Inactive_LuminosityOpacity{};

		static winrt::Windows::UI::Composition::CompositionBrush CreateBrush(
			const winrt::Windows::UI::Composition::Compositor& compositor,
			const winrt::Windows::UI::Color& tintColor,
			const winrt::Windows::UI::Color& luminosityColor,
			float tintOpacity, 
			float luminosityOpacity
		) try
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
			luminosityColorEffect->SetColor(tintColor);

			auto luminosityOpacityEffect{ winrt::make_self<OpacityEffect>() };
			luminosityOpacityEffect->SetName(L"LuminosityOpacity");
			luminosityOpacityEffect->SetOpacity(luminosityOpacity);
			luminosityOpacityEffect->SetInput(*luminosityColorEffect);

			auto luminosityBlendEffect{ winrt::make_self<BlendEffect>() };
			luminosityBlendEffect->SetBlendMode(D2D1_BLEND_MODE_COLOR);
			luminosityBlendEffect->SetBackground(winrt::Windows::UI::Composition::CompositionEffectSourceParameter{ L"BlurredWallpaperBackdrop" });
			luminosityBlendEffect->SetForeground(*luminosityOpacityEffect);

			auto colorBlendEffect{ winrt::make_self<BlendEffect>() };
			colorBlendEffect->SetBlendMode(D2D1_BLEND_MODE_LUMINOSITY);
			colorBlendEffect->SetBackground(*luminosityBlendEffect);
			colorBlendEffect->SetForeground(*tintOpacityEffect);

			auto micaEffectBrush{ compositor.CreateEffectFactory(*colorBlendEffect).CreateBrush() };
			micaEffectBrush.SetSourceParameter(L"BlurredWallpaperBackdrop", compositor.TryCreateBlurredWallpaperBackdropBrush());

			return micaEffectBrush;
		}
		catch (...) { return nullptr; }

		void STDMETHODCALLTYPE ReloadParameters()
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

			lightMode_Active_TintOpacity = GetFloatAlpha(g_configData.activeBlendColor);
			lightMode_Inactive_TintOpacity = GetFloatAlpha(g_configData.inactiveBlendColor);
			darkMode_Active_TintOpacity = GetFloatAlpha(g_configData.activeBlendColorDark);
			darkMode_Inactive_TintOpacity = GetFloatAlpha(g_configData.inactiveBlendColorDark);

			lightMode_Active_LuminosityOpacity = g_configData.luminosityOpacity;
			lightMode_Inactive_LuminosityOpacity = g_configData.luminosityOpacity;
			darkMode_Active_LuminosityOpacity = g_configData.luminosityOpacity;
			darkMode_Inactive_LuminosityOpacity = g_configData.luminosityOpacity;
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
					lightMode_Active_Color,
					lightMode_Active_TintOpacity,
					lightMode_Active_LuminosityOpacity
				);
				lightMode_Inactive_Brush = CreateBrush(
					compositor,
					lightMode_Inactive_Color,
					lightMode_Inactive_Color,
					lightMode_Inactive_TintOpacity,
					lightMode_Inactive_LuminosityOpacity
				);
				darkMode_Active_Brush = CreateBrush(
					compositor,
					darkMode_Active_Color,
					darkMode_Active_Color,
					darkMode_Active_TintOpacity,
					darkMode_Active_LuminosityOpacity
				);
				darkMode_Inactive_Brush = CreateBrush(
					compositor,
					darkMode_Inactive_Color,
					darkMode_Inactive_Color,
					darkMode_Inactive_TintOpacity,
					darkMode_Inactive_LuminosityOpacity
				);
			}

			return CDCompResources::GetBrush(useDarkMode, windowActivated);
		}
		catch (...) { return nullptr; }
	};

	struct CMicaBackdrop : CDCompBackdrop
	{
		inline static CMicaResources s_sharedResources{};

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

	struct CAccentMicaResources : CDCompResourcesBase
	{

	};
	struct CAccentMicaBackdrop : CAccentDCompBackdrop
	{
		inline static CAccentMicaResources s_sharedResources{};
		bool usingLuminosity{ false };
		winrt::Windows::UI::Color currenTintColor{};

		HRESULT STDMETHODCALLTYPE UpdateBrush(const DWM::ACCENT_POLICY& policy) try
		{
			s_sharedResources.interopDCompDevice.copy_from(
				DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice()
			);
			auto compositor{ spriteVisual.Compositor() };
			auto useLuminosity
			{
				MDWMBlurGlass::os::buildNumber >= 22000 &&
				(policy.nFlags & 2) != 0 &&
				(
					(policy.nAccentState == 3 && MDWMBlurGlass::os::buildNumber > 22000) ||
					(policy.nAccentState == 4)
				)
			};
			auto tintColor{ FromAbgr(policy.nColor) };
			if (
				usingLuminosity != useLuminosity ||
				currenTintColor != tintColor ||
				interopDCompDevice != s_sharedResources.interopDCompDevice
			)
			{
				interopDCompDevice = s_sharedResources.interopDCompDevice;
				usingLuminosity = useLuminosity;
				currenTintColor = tintColor;

				spriteVisual.Brush(
					CMicaResources::CreateBrush(
						compositor,
						tintColor,
						useLuminosity ? winrt::Windows::UI::Color{ 255, tintColor.R, tintColor.G, tintColor.B } : tintColor,
						1.f,
						1.f
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