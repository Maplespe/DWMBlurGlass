#pragma once
#include "DWMStruct.h"
#include "../Effects/CrossFadeEffect.hpp"
#include "ColorConversion.hpp"
#include "CommonDef.h"
#include "../Effects/GaussianBlurEffect.hpp"

namespace MDWMBlurGlassExt
{
	using namespace CommonDef;

	inline winrt::Windows::UI::Color MakeWinrtColor(COLORREF color, bool rgb = false)
	{
		return
		{
			rgb ? (BYTE)0xFF : BYTE((color >> 24) & 0xff),
			GetRValue(color),
			GetGValue(color),
			GetBValue(color),
		};
	}

	inline float GetFloatAlpha(COLORREF color)
	{
		return float(BYTE((color >> 24) & 0xff)) / 255.f;
	}

	FORCEINLINE wge::IGraphicsEffectSource CreateBlurredBackdrop(float blurAmount)
	{
		if (!blurAmount)
		{
			return wuc::CompositionEffectSourceParameter{ L"Backdrop" };
		}

		auto gaussianBlurEffect{ winrt::make_self<GaussianBlurEffect>() };
		gaussianBlurEffect->SetName(L"Blur");
		gaussianBlurEffect->SetBorderMode(D2D1_BORDER_MODE_HARD);
		gaussianBlurEffect->SetBlurAmount(blurAmount);
		gaussianBlurEffect->SetOptimizationMode(D2D1_GAUSSIANBLUR_OPTIMIZATION_SPEED);
		gaussianBlurEffect->SetInput(wuc::CompositionEffectSourceParameter{ L"Backdrop" });
		return *gaussianBlurEffect;
	}

	FORCEINLINE wuc::CompositionBrush CreateCrossFadeBrush(
		const wuc::Compositor& compositor,
		const wuc::CompositionBrush& from,
		const wuc::CompositionBrush& to
	)
	{
		auto crossFadeEffect{ winrt::make_self<CrossFadeEffect>() };
		crossFadeEffect->SetName(L"Crossfade");
		crossFadeEffect->SetSource(wuc::CompositionEffectSourceParameter{ L"From" });
		crossFadeEffect->SetDestination(wuc::CompositionEffectSourceParameter{ L"To" });
		crossFadeEffect->SetWeight(0);

		auto crossFadeEffectBrush{ compositor.CreateEffectFactory(*crossFadeEffect, {L"Crossfade.Weight"}).CreateBrush() };
		crossFadeEffectBrush.Comment(L"Crossfade");
		crossFadeEffectBrush.SetSourceParameter(L"From", from);
		crossFadeEffectBrush.SetSourceParameter(L"To", to);
		return crossFadeEffectBrush;
	}

	FORCEINLINE wuc::ScalarKeyFrameAnimation CreateCrossFadeAnimation(
		const wuc::Compositor& compositor,
		char easingFunction,
		winrt::Windows::Foundation::TimeSpan const& crossfadeTime
	)
	{
		auto animation{ compositor.CreateScalarKeyFrameAnimation() };
		auto easing{ easingFunction == 1 ? static_cast<wuc::CompositionEasingFunction>(
			compositor.CreateCubicBezierEasingFunction({ 0.5f, 0.0f }, { 0.0f, 0.9f })) : static_cast<wuc::CompositionEasingFunction>(compositor.CreateLinearEasingFunction()) 
		};
		animation.InsertKeyFrame(0.0f, 0.0f);
		animation.InsertKeyFrame(1.0f, 1.0f, easing);
		animation.Duration(crossfadeTime);
		return animation;
	}

	FORCEINLINE void ThisModule_AddRef()
	{
		HMODULE thisModule{ nullptr };
		GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, L"", &thisModule);
	}

	FORCEINLINE void ThisModule_Release(bool async = true)
	{
		if (async)
		{
			static wil::unique_threadpool_work s_freeModuleRefWork{ CreateThreadpoolWork([](PTP_CALLBACK_INSTANCE instance, PVOID, PTP_WORK) { DwmFlush(); FreeLibraryWhenCallbackReturns(instance, wil::GetModuleInstanceHandle());  }, nullptr, nullptr) };
			SubmitThreadpoolWork(s_freeModuleRefWork.get());
		}
		else
		{
			FreeLibraryAndExitThread(wil::GetModuleInstanceHandle(), 0);
		}
	}
}