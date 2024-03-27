#pragma once
#include "DWMStruct.h"
#include "../Effects/CrossFadeEffect.hpp"
#include "ColorConversion.hpp"
#include "CommonDef.h"

namespace MDWMBlurGlassExt
{
	using namespace CommonDef;
	winrt::Windows::UI::Color MakeWinrtColor(COLORREF color, bool rgb = true)
	{
		return
		{
			rgb ? (BYTE)0xFF : BYTE((color >> 24) & 0xff),
			GetRValue(color),
			GetGValue(color),
			GetBValue(color),
		};
	}
	float GetFloatAlpha(COLORREF color)
	{
		return float(BYTE((color >> 24) & 0xff)) / 255.f;
	}

	struct CDCompResourcesBase
	{
		winrt::com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner> interopDCompDevice{ nullptr };
		virtual void ReloadParameters() {}
	};
	struct CDCompResources : CDCompResourcesBase
	{
		winrt::Windows::UI::Composition::CompositionBrush lightMode_Active_Brush{ nullptr };
		winrt::Windows::UI::Composition::CompositionBrush darkMode_Active_Brush{ nullptr };
		winrt::Windows::UI::Composition::CompositionBrush lightMode_Inactive_Brush{ nullptr };
		winrt::Windows::UI::Composition::CompositionBrush darkMode_Inactive_Brush{ nullptr };
		std::chrono::milliseconds crossfadeTime{ 87 };
		winrt::Windows::UI::Color lightMode_Active_Color{};
		winrt::Windows::UI::Color darkMode_Active_Color{};
		winrt::Windows::UI::Color lightMode_Inactive_Color{};
		winrt::Windows::UI::Color darkMode_Inactive_Color{};

		static winrt::Windows::UI::Composition::CompositionBrush CreateCrossFadeEffectBrush(
			const winrt::Windows::UI::Composition::Compositor& compositor,
			const winrt::Windows::UI::Composition::CompositionBrush& from,
			const winrt::Windows::UI::Composition::CompositionBrush& to
		)
		{
			auto crossFadeEffect{ winrt::make_self<CrossFadeEffect>() };
			crossFadeEffect->SetName(L"Crossfade");
			crossFadeEffect->SetSource(winrt::Windows::UI::Composition::CompositionEffectSourceParameter{ L"From" });
			crossFadeEffect->SetDestination(winrt::Windows::UI::Composition::CompositionEffectSourceParameter{ L"To" });
			crossFadeEffect->SetWeight(0);

			auto crossFadeEffectBrush{ compositor.CreateEffectFactory( *crossFadeEffect, {L"Crossfade.Weight"} ).CreateBrush() };
			crossFadeEffectBrush.Comment(L"Crossfade");
			crossFadeEffectBrush.SetSourceParameter(L"From", from);
			crossFadeEffectBrush.SetSourceParameter(L"To", to);
			return crossFadeEffectBrush;
		}
		static winrt::Windows::UI::Composition::ScalarKeyFrameAnimation CreateCrossFadeAnimation(
			const winrt::Windows::UI::Composition::Compositor& compositor,
			winrt::Windows::Foundation::TimeSpan const& crossfadeTime
		)
		{
			auto animation{ compositor.CreateScalarKeyFrameAnimation() };
			auto linearEasing{ compositor.CreateLinearEasingFunction() };
			animation.InsertKeyFrame(0.0f, 0.0f, linearEasing);
			animation.InsertKeyFrame(1.0f, 1.0f, linearEasing);
			animation.Duration(crossfadeTime);
			return animation;
		}
		virtual winrt::Windows::UI::Composition::CompositionBrush STDMETHODCALLTYPE GetBrush(bool useDarkMode, bool windowActivated)
		{
			if (useDarkMode)
			{
				if (windowActivated) { return darkMode_Active_Brush; }
				else { return darkMode_Inactive_Brush; }
			}
			else
			{
				if (windowActivated) { return lightMode_Active_Brush; }
				else { return lightMode_Inactive_Brush; }
			}

			return nullptr;
		}
	};

	struct CDCompBackdropBase
	{
		winrt::Windows::UI::Composition::SpriteVisual spriteVisual{ nullptr };

		STDMETHOD(Initialize)(
			winrt::Windows::UI::Composition::VisualCollection& visualCollection
		)
		{
			auto compositor{ visualCollection.Compositor() };
			spriteVisual = compositor.CreateSpriteVisual();
			spriteVisual.RelativeSizeAdjustment({ 1.f, 1.f });
			visualCollection.InsertAtTop(spriteVisual);

			return S_OK;
		}
		virtual ~CDCompBackdropBase() = default;
	};

	struct CDCompBackdrop : CDCompBackdropBase
	{
		winrt::Windows::UI::Composition::CompositionBrush currentBrush{ nullptr };
		winrt::Windows::UI::Color currentColor{};

		STDMETHOD(TryCrossFadeToNewBrush)(
			const winrt::Windows::UI::Composition::Compositor& compositor, 
			const winrt::Windows::UI::Composition::CompositionBrush& newBrush,
			std::chrono::milliseconds crossfadeTime
		) try
		{
			if (currentBrush != newBrush)
			{
				if (currentBrush && crossfadeTime.count())
				{
					const auto crossfadeBrush
					{
						CDCompResources::CreateCrossFadeEffectBrush(
							compositor,
							currentBrush,
							newBrush
						)
					};
					currentBrush = newBrush;
					spriteVisual.Brush(crossfadeBrush);

					crossfadeBrush.StartAnimation(
						L"Crossfade.Weight",
						CDCompResources::CreateCrossFadeAnimation(
							compositor,
							crossfadeTime
						)
					);
				}
				else
				{
					currentBrush = newBrush;
					spriteVisual.Brush(currentBrush);
				}
			}

			return S_OK;
		}
		CATCH_RETURN()

		STDMETHOD(UpdateColorizationColor)(
			bool useDarkMode,
			bool windowActivated
		) PURE;
		STDMETHOD(Update)(
			bool useDarkMode,
			bool windowActivated
		) PURE;
	};
	struct CAccentDCompBackdrop : CDCompBackdropBase
	{
		winrt::com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner> interopDCompDevice{ nullptr };
		inline static winrt::Windows::UI::Color FromAbgr(DWORD gradientColor)
		{
			auto abgr{ reinterpret_cast<const UCHAR*>(&gradientColor) };
			return
			{
				abgr[3],
				abgr[0],
				abgr[1],
				abgr[2]
			};
		}
		STDMETHOD(Update)(const DWM::ACCENT_POLICY& policy) PURE;
	};
}