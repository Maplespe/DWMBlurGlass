#pragma once
#include "DWMStruct.h"
#include "../Backdrops/GlassReflectionBackdrop.hpp"

namespace MDWMBlurGlassExt
{
	enum class CompositedBackdropKind : CHAR
	{
		None = -1,
		Legacy,
		Accent,
		SystemBackdrop
	};
	enum class CompositedBackdropType : CHAR
	{
		None = -1,
		Aero,
		Acrylic,
		Mica,
		Blur
	};

	class CGlassReflectionVisual;

	class CCompositedBackdropVisual : public implements<CCompositedBackdropVisual, DCompPrivate::ICompositedBackdropVisual>, CBackdropVisual
	{
		bool m_backdropDataChanged{ false };
		bool m_backdropBrushChanged{ false };
		bool m_activate{ false };
		bool m_visible{ true };
		bool m_clipApplied{ false };
		bool m_clipApplied2{ true };
		DWORD m_state{};
		DWORD m_color{};
		DWORD m_accentFlags{};
		std::chrono::steady_clock::time_point m_backdropTimeStamp{};

		CompositedBackdropKind m_kind{ CompositedBackdropKind::None };
		DWM::CTopLevelWindow* m_window{ nullptr };
		DWM::CWindowData* m_data{ nullptr };

		wuc::CompositionBrush m_backdropBrush{ nullptr };
		wuc::CompositionBrush m_previousBackdropBrush{ nullptr };
		wuc::ContainerVisual m_wucRootVisual{ nullptr };
		wuc::ContainerVisual m_containerVisual{ nullptr };
		wuc::SpriteVisual m_spriteVisual{ nullptr };

		bool m_isWaitingForAnimationComplete{ false };
		bool m_shouldPlayCrossFadeAnimation{ false };
		wuc::CompositionScopedBatch m_waitingForAnimationCompleteBatch{ nullptr };
		winrt::event_token m_waitingForAnimationCompleteToken{};

		com_ptr<CGlassReflectionVisual> m_reflectionVisual{ nullptr };

		wil::unique_hrgn m_gdiWindowRgn{ nullptr };
		wil::unique_hrgn m_clientBlurRgn{ nullptr };
		wil::unique_hrgn m_borderRgn{ nullptr };
		RECT m_captionRect{};
		RECT m_windowRect{};
		std::optional<RECT> m_accentRect{ std::nullopt };
		wil::unique_hrgn m_compositedRgn{ nullptr };
		wuc::CompositionPathGeometry m_pathGeometry{ nullptr };
	
		HRESULT InitializeVisual() override;
		void UninitializeVisual() override;

		void OnDeviceLost();
		void HandleChanges();
		void OnBackdropKindUpdated(CompositedBackdropKind kind);
		void OnBackdropRegionChanged(wil::unique_hrgn& newBackdropRegion);
		void OnBackdropBrushUpdated();
		void OnBackdropBrushChanged();
		void CancelAnimationCompleteWait();

		POINT GetClientAreaOffset() const;
		bool DoesBorderParticipateInBackdropRegion() const;
		wil::unique_hrgn CompositeNewBackdropRegion() const;

	public:
		CCompositedBackdropVisual(DWM::CTopLevelWindow* window);
		~CCompositedBackdropVisual() override;

		void SetClientBlurRegion(HRGN region) override;
		void SetCaptionRegion(HRGN region) override;
		void SetBorderRegion(HRGN region) override;
		void SetAccentRect(LPCRECT lprc) override;
		void SetGdiWindowRegion(HRGN region) override;
		void ValidateVisual() override;
		void UpdateNCBackground() override;
		auto GetuDwmVisual() const
		{
			return m_udwmVisual.get();
		}
		auto GetDCompVisual() const
		{
			return m_rootVisual;
		}
		bool CanBeTrimmed() override
		{
			if (m_kind != CompositedBackdropKind::Accent && (!m_visible || m_window->IsTrullyMinimized()))
			{
				return true;
			}
			if (m_accentRect.has_value())
			{
				return false;
			}
			if (m_gdiWindowRgn && m_data->GetAccentPolicy()->IsGdiRegionRespected())
			{
				return false;
			}

			return true;
		}
	};

	// temporary workaround for aero peek/live preview
	class CClonedPeekingBackdropVisual : public implements<CCompositedBackdropVisual, DCompPrivate::ICompositedBackdropVisual>, CBackdropVisual
	{
		DWM::CTopLevelWindow* m_window{ nullptr };
		DWM::CWindowData* m_data{ nullptr };
		wuc::ContainerVisual m_containerVisual{ nullptr };
		wuc::CompositionGeometricClip m_roundedClip{ nullptr };
		wuc::CompositionRoundedRectangleGeometry m_roundedGeometry{ nullptr };
		com_ptr<CGlassReflectionVisual> m_reflectionVisual{ nullptr };

		HRESULT InitializeVisual() override;

		void UninitializeVisual() override;

		void OnDeviceLost()
		{
			UninitializeVisual();
			InitializeInteropDevice(DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice());
			InitializeVisual();
		}
	public:
		CClonedPeekingBackdropVisual(DWM::CTopLevelWindow* source, DWM::CTopLevelWindow* target) :
			m_window{ target },
			m_data{ source->GetData() },
			m_reflectionVisual{ winrt::make_self<CGlassReflectionVisual>(source, m_data, true) },
			CBackdropVisual{ target->GetVisual() }
		{
			OnDeviceLost();
		}

		~CClonedPeekingBackdropVisual() override
		{
			CClonedPeekingBackdropVisual::UninitializeVisual();
		}

		void SetClientBlurRegion(HRGN /*region*/) override {}
		void SetCaptionRegion(HRGN /*region*/) override {}
		void SetBorderRegion(HRGN /*region*/) override {}
		void SetAccentRect(LPCRECT /*lprc*/) override {}
		void SetGdiWindowRegion(HRGN /*region*/) override {}
		void ValidateVisual() override {}
		void UpdateNCBackground() override {}
		bool CanBeTrimmed() override
		{
			return false;
		}
	};

	class CClonedCompositedBackdropVisual : public implements<CClonedCompositedBackdropVisual, DCompPrivate::ICompositedBackdropVisual>, CClonedBackdropVisual
	{
		com_ptr<CCompositedBackdropVisual> m_compositedBackdropVisual{ nullptr };
	public:
		CClonedCompositedBackdropVisual(DWM::CTopLevelWindow* window, const CCompositedBackdropVisual* compositedBackdropVisual) :
			CClonedBackdropVisual{ window->GetVisual(), compositedBackdropVisual->GetDCompVisual() }
		{
			copy_from_abi(m_compositedBackdropVisual, compositedBackdropVisual);
			OnDeviceLost();
		}

		~CClonedCompositedBackdropVisual() override
		{
			CClonedSpriteVisual::UninitializeVisual();
			m_compositedBackdropVisual = nullptr;
		}

		void SetClientBlurRegion(HRGN /*region*/) override {}
		void SetCaptionRegion(HRGN /*region*/) override {}
		void SetBorderRegion(HRGN /*region*/) override {}
		void SetAccentRect(LPCRECT /*lprc*/) override {}
		void SetGdiWindowRegion(HRGN /*region*/) override {}
		void ValidateVisual() override {}
		void UpdateNCBackground() override {}
		bool CanBeTrimmed() override
		{
			return false;
		}
	};
}