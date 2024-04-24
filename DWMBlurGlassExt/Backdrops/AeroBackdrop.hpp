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
			// New Aero backdrop recive v2, thx 2 @aubymori and @wackyideas for some help

			// define the colors used

			// before you ask why i have 3 of the color black, ask Direct2D first why i can't just reuse this color

			winrt::Windows::UI::Color black{ 255,0,0,0 }; 
			winrt::Windows::UI::Color black2{ 255,0,0,0 }; 
			winrt::Windows::UI::Color black3{ 255,0,0,0 }; 

			auto ColorizationColor{ winrt::make_self<ColorSourceEffect>() };
			ColorizationColor->SetName(L"MainColor");
			ColorizationColor->SetColor(mainColor);

			auto ColorizationAfterglow{ winrt::make_self<ColorSourceEffect>() };
			ColorizationAfterglow->SetName(L"GlowColor");
			ColorizationAfterglow->SetColor(glowColor);

			// again this is disgusting and i hate that i have to use it, but Direct2D will NOT let me use brushes & colors

			auto BalanceBackdrop{ winrt::make_self<ColorSourceEffect>() }; // used to blend against the layers thus acting as the balance
			BalanceBackdrop->SetColor(black);

			auto BalanceBackdrop2{ winrt::make_self<ColorSourceEffect>() }; // used to blend against the layers thus acting as the balance
			BalanceBackdrop2->SetColor(black2);

			auto BalanceBackdrop3{ winrt::make_self<ColorSourceEffect>() }; // used to blend against the layers thus acting as the balance
			BalanceBackdrop3->SetColor(black3);

			// gaussian blur backdrop

			// apparently i CANNOT REUSE brushes... so here goes 2 gaussian blur brushes !!!

			auto AfterglowBlur{ winrt::make_self<GaussianBlurEffect>() };
			AfterglowBlur->SetBorderMode(D2D1_BORDER_MODE_HARD);
			AfterglowBlur->SetBlurAmount(blurAmount);
			AfterglowBlur->SetInput(winrt::Windows::UI::Composition::CompositionEffectSourceParameter{ L"Backdrop" });

			auto RegularBlur{ winrt::make_self<GaussianBlurEffect>() };
			RegularBlur->SetBorderMode(D2D1_BORDER_MODE_HARD);
			RegularBlur->SetBlurAmount(blurAmount);
			RegularBlur->SetInput(winrt::Windows::UI::Composition::CompositionEffectSourceParameter{ L"Backdrop" });

			// ColorizationColor -- render semi-transparent color over black bg

			auto ColorizationColorBalance{ winrt::make_self<OpacityEffect>() };
			ColorizationColorBalance->SetInput(*ColorizationColor);
			ColorizationColorBalance->SetOpacity(colorBalance);

			auto ColorizationColorRender{ winrt::make_self<CompositeStepEffect>() };
			ColorizationColorRender->SetCompositeMode(D2D1_COMPOSITE_MODE_SOURCE_OVER);
			ColorizationColorRender->SetDestination(*BalanceBackdrop);
			ColorizationColorRender->SetSource(*ColorizationColorBalance);

			// ColorizationAfterglow -- multiply over b&w bg, over black bg

			auto CAGSetSaturation{ winrt::make_self<SaturationEffect>() }; 
			CAGSetSaturation->SetInput(*AfterglowBlur);
			CAGSetSaturation->SetSaturation(0);

			auto ColorizationAfterglowBlend{ winrt::make_self<BlendEffect>() };
			ColorizationAfterglowBlend->SetBlendMode(D2D1_BLEND_MODE_MULTIPLY);
			ColorizationAfterglowBlend->SetBackground(*CAGSetSaturation);
			ColorizationAfterglowBlend->SetForeground(*ColorizationAfterglow);

			auto ColorizationAfterglowBalance{ winrt::make_self<OpacityEffect>() };
			ColorizationAfterglowBalance->SetInput(*ColorizationAfterglowBlend);
			ColorizationAfterglowBalance->SetOpacity(glowBalance);

			auto ColorizationAfterglowRender{ winrt::make_self<CompositeStepEffect>() };
			ColorizationAfterglowRender->SetCompositeMode(D2D1_COMPOSITE_MODE_SOURCE_OVER);
			ColorizationAfterglowRender->SetDestination(*BalanceBackdrop2);
			ColorizationAfterglowRender->SetSource(*ColorizationAfterglowBalance); 

			// ColorizationBlurBalance -- gaussian blur over black bg

			auto ColorizationBlurBalance{ winrt::make_self<OpacityEffect>() };
			ColorizationBlurBalance->SetName(L"gaussianBlurEffect");
			ColorizationBlurBalance->SetInput(*RegularBlur);
			ColorizationBlurBalance->SetOpacity(blurBalance);

			auto ColorizationBlurBalanceRender{ winrt::make_self<CompositeStepEffect>() };
			ColorizationBlurBalanceRender->SetCompositeMode(D2D1_COMPOSITE_MODE_SOURCE_OVER);
			ColorizationBlurBalanceRender->SetName(L"ColorizationBlurBalanceRender");
			ColorizationBlurBalanceRender->SetDestination(*BalanceBackdrop3);
			ColorizationBlurBalanceRender->SetSource(*ColorizationBlurBalance); 

			// blend everything together with ADDITIVE BLENDING... it's not color dodge folks!!!

			auto ColorOverBlur{ winrt::make_self<CompositeStepEffect>() };
			ColorOverBlur->SetCompositeMode(D2D1_COMPOSITE_MODE_PLUS);
			ColorOverBlur->SetDestination(*ColorizationAfterglowRender);
			ColorOverBlur->SetSource(*ColorizationColorRender);

			auto BlendOverBlur{ winrt::make_self<CompositeStepEffect>() };
			BlendOverBlur->SetCompositeMode(D2D1_COMPOSITE_MODE_PLUS);
			BlendOverBlur->SetDestination(*ColorizationBlurBalanceRender);
			BlendOverBlur->SetSource(*ColorOverBlur);

			auto effectBrush{ compositor.CreateEffectFactory(*BlendOverBlur).CreateBrush() };
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

			// please altalex keep this as is or atleast try to keep this behavior

			lightMode_Active_ColorBalance = g_configData.ColorizationColorBalance / 100.0f;
			lightMode_Inactive_ColorBalance = lightMode_Active_ColorBalance * 0.4f;
			darkMode_Active_ColorBalance = g_configData.ColorizationColorBalance / 100.0f;
			darkMode_Inactive_ColorBalance = darkMode_Active_ColorBalance * 0.4f;

			lightMode_Active_GlowBalance = g_configData.ColorizationAfterglowBalance / 100.0f;
			lightMode_Inactive_GlowBalance = g_configData.ColorizationAfterglowBalance / 100.0f;
			darkMode_Active_GlowBalance = g_configData.ColorizationAfterglowBalance / 100.0f;
			darkMode_Inactive_GlowBalance = g_configData.ColorizationAfterglowBalance / 100.0f;

			Active_BlurBalance = g_configData.ColorizationBlurBalance / 100.0f;
			Inactive_BlurBalance = (Active_BlurBalance * 0.4) + 0.60;

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
