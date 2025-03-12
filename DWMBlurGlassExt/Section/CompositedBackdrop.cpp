#include "../Backdrops/DCompBackdrop.hpp"
#include "../Backdrops/BackdropFactory.hpp"
#include "CustomBackdropEffect.h"
#include "VersionHelper.h"
#include "CommonDef.h"
#include "../Helper/CanvasGeometry.hpp"
#include <windowsx.h>

namespace MDWMBlurGlassExt
{
	using namespace CommonDef;

	HRESULT CCompositedBackdropVisual::InitializeVisual()
	{
		RETURN_IF_FAILED(CBackdropVisual::InitializeVisual());

		auto compositor{ m_dcompDevice.as<wuc::Compositor>() };
		m_wucRootVisual = compositor.CreateContainerVisual();
		m_wucRootVisual.BorderMode(wuc::CompositionBorderMode::Soft);
		m_containerVisual = compositor.CreateContainerVisual();
		m_containerVisual.BorderMode(wuc::CompositionBorderMode::Hard);
		m_spriteVisual = compositor.CreateSpriteVisual();
		m_spriteVisual.RelativeSizeAdjustment({ 1.f, 1.f });

		// prepare backdrop brush
		OnBackdropBrushUpdated();

		// if the effect brush is compiled during the maximize/minimize animation
		// then it will NOT show normally, that's because the compilation is asynchronous
		m_reflectionVisual->InitializeVisual(compositor);
		m_wucRootVisual.Children().InsertAtTop(m_containerVisual);
		m_containerVisual.Children().InsertAtBottom(m_spriteVisual);
		m_containerVisual.Children().InsertAtTop(m_reflectionVisual->GetVisual());
		m_pathGeometry = compositor.CreatePathGeometry();

		com_ptr<ID2D1Factory> factory{ nullptr };
		DWM::CDesktopManager::s_pDesktopManagerInstance->GetD2DDevice()->GetFactory(factory.put());
		// since build 22621, Path propertie cannot be empty or the dwmcore will raise a null pointer exception
		m_pathGeometry.Path(wuc::CompositionPath{ Geometry::CanvasGeometry::CreateGeometryFromHRGN(factory.get(), m_compositedRgn.get()).as<wg::IGeometrySource2D>() });
		
		m_rootVisual.Clip(compositor.CreateGeometricClip(m_pathGeometry));
		m_visualCollection.InsertAtBottom(m_wucRootVisual);

		return S_OK;
	}

	void CCompositedBackdropVisual::UninitializeVisual()
	{
		CancelAnimationCompleteWait();

		m_pathGeometry = nullptr;
		m_spriteVisual = nullptr;
		m_wucRootVisual = nullptr;
		m_containerVisual = nullptr;
		m_backdropBrush = nullptr;
		m_previousBackdropBrush = nullptr;

		m_reflectionVisual->UninitializeVisual();
		CBackdropVisual::UninitializeVisual();
	}

	void CCompositedBackdropVisual::OnDeviceLost()
	{
		UninitializeVisual();
		InitializeInteropDevice(DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice());
		InitializeVisual();
		m_backdropDataChanged = true;
	}

	void CCompositedBackdropVisual::HandleChanges() try
	{
		wil::unique_hrgn compositedRgn{ CompositeNewBackdropRegion() };
		if (!EqualRgn(m_compositedRgn.get(), compositedRgn.get()))
		{
			OnBackdropRegionChanged(compositedRgn);
		}
		m_backdropDataChanged = false;
	}
	CATCH_LOG_RETURN()

	void CCompositedBackdropVisual::OnBackdropKindUpdated(CompositedBackdropKind kind)
	{
		if (m_kind != kind)
		{
			m_kind = kind;
			m_backdropDataChanged = true;
			m_previousBackdropBrush = nullptr;
			m_backdropBrush = nullptr;
			m_backdropBrushChanged = true;
		}
	}

	void CCompositedBackdropVisual::OnBackdropRegionChanged(wil::unique_hrgn& newBackdropRegion)
	{
		m_compositedRgn = std::move(newBackdropRegion);

		bool isVisible{};
		RECT regionBox{};
		auto regionType{ GetRgnBox(m_compositedRgn.get(), &regionBox) };
		if (
			regionType == NULLREGION ||
			IsRectEmpty(&regionBox)
			)
		{
			isVisible = false;
		}
		else
		{
			isVisible = true;
		}

		if (m_visible != isVisible)
		{
			m_rootVisual.IsVisible(isVisible);
			m_visible = isVisible;
		}

		if (!m_visible)
		{
			return;
		}

		bool clipApplied{ false };
		if (regionType == SIMPLEREGION)
		{
			clipApplied = false;
		}
		if (regionType == COMPLEXREGION)
		{
			clipApplied = true;
		}
		if (m_clipApplied2 != clipApplied)
		{
			m_rootVisual.Clip(clipApplied ? m_dcompDevice.as<wuc::Compositor>().CreateGeometricClip(m_pathGeometry) : nullptr);
			m_clipApplied2 = clipApplied;
		}

		wfn::float3 offset{ static_cast<float>(regionBox.left), static_cast<float>(regionBox.top), 0.f };
		m_containerVisual.Offset(offset);
		m_containerVisual.Size({ static_cast<float>(max(wil::rect_width(regionBox), 0)), static_cast<float>(max(wil::rect_height(regionBox), 0)) });
		m_reflectionVisual->NotifyOffsetToWindow(offset);

		winrt::com_ptr<ID2D1Factory> factory{ nullptr };
		DWM::CDesktopManager::s_pDesktopManagerInstance->GetD2DDevice()->GetFactory(factory.put());
		auto canvasGeometry
		{
			Geometry::CanvasGeometry::CreateGeometryFromHRGN(
				factory.get(),
				m_compositedRgn.get()
			)
		};
		m_pathGeometry.Path(wuc::CompositionPath{ canvasGeometry.as<wg::IGeometrySource2D>() });
	}

	void CCompositedBackdropVisual::OnBackdropBrushUpdated()
	{
		m_backdropBrushChanged = false;
		m_shouldPlayCrossFadeAnimation = false;
		if (m_kind == CompositedBackdropKind::Accent)
		{
			auto policy{ m_data->GetAccentPolicy() };
			if (
				policy->nAccentState != m_state ||
				policy->nColor != m_color ||
				policy->nFlags != m_accentFlags
				)
			{
				m_state = policy->nAccentState;
				m_color = policy->nColor;
				m_accentFlags = policy->nFlags;
				m_backdropBrushChanged = true;
			}

			if (m_activate != true)
			{
				m_activate = true;
				m_backdropBrushChanged = true;
			}
		}
		else
		{
			auto active{ GetSrcWindow()->TreatAsActiveWindow()};
			if (active != m_activate)
			{
				m_shouldPlayCrossFadeAnimation = true;
				m_activate = active;
				m_backdropBrushChanged = true;
			}

			DWORD color;
			if (g_configData.useAccentColor)
			{
				BYTE alpha = BYTE(ULONG((active ? g_configData.activeBlendColor : g_configData.inactiveBlendColor) >> 24) & 0xff);
				color = RGB(GetRValue(g_accentColor), GetGValue(g_accentColor), GetBValue(g_accentColor));
				color = color | (alpha << 24);
			}
			else
			{
				if(m_data->IsUsingDarkMode())
					color = active ? g_configData.activeBlendColorDark : g_configData.inactiveBlendColorDark;
				else
					color = active ? g_configData.activeBlendColor : g_configData.inactiveBlendColor;
			}
			if (color != m_color)
			{
				m_shouldPlayCrossFadeAnimation = true;
				m_color = color;
				m_backdropBrushChanged = true;
			}
		}

		auto timeStamp{ BackdropFactory::GetBackdropBrushTimeStamp() };
		if (m_backdropTimeStamp != timeStamp)
		{
			m_backdropTimeStamp = timeStamp;
			m_backdropBrushChanged = true;
		}

		if (m_backdropBrushChanged || !m_backdropBrush)
		{
			m_backdropBrush = BackdropFactory::GetOrCreateBackdropBrush(
				m_dcompDevice.as<wuc::Compositor>(),
				m_color,
				m_activate,
				m_kind == CompositedBackdropKind::Accent ? m_data->GetAccentPolicy() : nullptr
			);
			OnBackdropBrushChanged();
		}
	}

	void CCompositedBackdropVisual::OnBackdropBrushChanged()
	{
		if (m_previousBackdropBrush == m_backdropBrush)
			return;

		if (!m_previousBackdropBrush)
		{
			m_shouldPlayCrossFadeAnimation = false;
		}
		CancelAnimationCompleteWait();

		if (m_shouldPlayCrossFadeAnimation && g_configData.crossFade)
		{
			auto compositor{ m_backdropBrush.Compositor() };
			auto crossfadeBrush{ CreateCrossFadeBrush(compositor, m_previousBackdropBrush, m_backdropBrush) };

			winrt::Windows::Foundation::TimeSpan crossfadeTime{ std::chrono::milliseconds{ g_configData.crossfadeTime } };
			// the completed handler won't be called under windows 10 2004, idk why...
			if (os::buildNumber >= 19041)
			{
				m_isWaitingForAnimationComplete = true;
				ThisModule_AddRef();

				m_waitingForAnimationCompleteBatch = compositor.CreateScopedBatch(wuc::CompositionBatchTypes::Animation);
				{
					auto strongThis{ get_strong() };
					auto handler = [strongThis](auto sender, auto args)
						{
							strongThis->CancelAnimationCompleteWait();
							strongThis->m_spriteVisual.Brush(strongThis->m_backdropBrush);
						};

					crossfadeBrush.StartAnimation(L"Crossfade.Weight", CreateCrossFadeAnimation(compositor,
						1, crossfadeTime));

					m_waitingForAnimationCompleteToken = m_waitingForAnimationCompleteBatch.Completed(handler);
					m_waitingForAnimationCompleteBatch.End();
				}
			}
			else
			{
				crossfadeBrush.StartAnimation(L"Crossfade.Weight", CreateCrossFadeAnimation(compositor,
					1, crossfadeTime));
			}

			m_spriteVisual.Brush(crossfadeBrush);
		}
		else
		{
			m_spriteVisual.Brush(m_backdropBrush);
		}
		m_previousBackdropBrush = m_backdropBrush;
	}

	void CCompositedBackdropVisual::CancelAnimationCompleteWait()
	{
		if (m_waitingForAnimationCompleteBatch)
		{
			m_waitingForAnimationCompleteBatch.Completed(m_waitingForAnimationCompleteToken);
			m_waitingForAnimationCompleteBatch = nullptr;
			m_waitingForAnimationCompleteToken = {};
		}
		if (m_isWaitingForAnimationComplete)
		{
			m_isWaitingForAnimationComplete = false;
			ThisModule_Release();
		}
	}

	POINT CCompositedBackdropVisual::GetClientAreaOffset() const
	{
		// GetClientBlurVisual() somtimes cannot work in ClonedVisual
		// so here we use the original offset get from CTopLevelWindow::UpdateClientBlur
		auto margins{ GetSrcWindow()->GetClientAreaContainerParentVisual()->GetMargins()};
		return { margins->cxLeftWidth, margins->cyTopHeight };
	}

	bool CCompositedBackdropVisual::DoesBorderParticipateInBackdropRegion() const
	{
		return g_configData.extendBorder || m_data->IsFrameExtendedIntoClientAreaLRB();
	}

	wil::unique_hrgn CCompositedBackdropVisual::CompositeNewBackdropRegion() const
	{
		wil::unique_hrgn compositedRgn{ CreateRectRgn(0, 0, 0, 0) };
		wil::unique_hrgn nonClientRgn{ CreateRectRgn(0, 0, 0, 0) };
		wil::unique_hrgn realClientBlurRgn{ CreateRectRgn(0, 0, 0, 0) };
		wil::unique_hrgn captionRgn{ CreateRectRgnIndirect(&m_captionRect) };
		wil::unique_hrgn windowRgn{ CreateRectRgnIndirect(&m_windowRect) };

		auto includedBorder{ DoesBorderParticipateInBackdropRegion() };
		CombineRgn(nonClientRgn.get(), captionRgn.get(), includedBorder ? m_borderRgn.get() : nullptr, includedBorder ? RGN_OR : RGN_COPY);
		// DwmEnableBlurBehind
		if (m_clientBlurRgn)
		{
			CopyRgn(realClientBlurRgn.get(), m_clientBlurRgn.get());
			auto clientOffset{ GetClientAreaOffset() };
			OffsetRgn(realClientBlurRgn.get(), clientOffset.x, clientOffset.y);
			CombineRgn(compositedRgn.get(), compositedRgn.get(), realClientBlurRgn.get(), RGN_OR);
		}
		if (m_kind != CompositedBackdropKind::Accent)
		{
			CombineRgn(compositedRgn.get(), compositedRgn.get(), nonClientRgn.get(), RGN_OR);
			if (m_kind == CompositedBackdropKind::SystemBackdrop)
			{
				CombineRgn(compositedRgn.get(), compositedRgn.get(), windowRgn.get(), RGN_OR);
			}
		}
		else
		{
			CombineRgn(compositedRgn.get(), compositedRgn.get(), windowRgn.get(), RGN_OR);
			if (m_accentRect)
			{
				wil::unique_hrgn accentRgn{ CreateRectRgnIndirect(&m_accentRect.value()) };
				CombineRgn(compositedRgn.get(), compositedRgn.get(), accentRgn.get(), RGN_AND);
			}

			if (
				m_gdiWindowRgn &&
				m_data->GetAccentPolicy()->IsGdiRegionRespected()
				)
			{
				CombineRgn(compositedRgn.get(), compositedRgn.get(), m_gdiWindowRgn.get(), RGN_AND);
			}
		}
		CombineRgn(compositedRgn.get(), compositedRgn.get(), windowRgn.get(), RGN_AND);

		return compositedRgn;
	}

	CCompositedBackdropVisual::CCompositedBackdropVisual(DWM::CTopLevelWindow* window) :
		m_window{ window },
		m_data{ window->GetData() },
		m_reflectionVisual{ winrt::make_self<CGlassReflectionVisual>(m_window, m_data) },
		CBackdropVisual{ window->GetVisual() }
	{
		m_borderRgn.reset(CreateRectRgn(0, 0, 0, 0));
		m_compositedRgn.reset(CreateRectRgn(0, 0, 0, 0));

		OnDeviceLost();
		OnBackdropKindUpdated(CustomBackdrop::GetActualBackdropKind(window));
		if (m_kind == CompositedBackdropKind::Accent)
		{
			wil::unique_hrgn clipRgn{ CreateRectRgn(0, 0, 0, 0) };
			if (GetWindowRgn(m_data->GetHWND(), clipRgn.get()) != ERROR)
			{
				CCompositedBackdropVisual::SetGdiWindowRegion(clipRgn.get());
			}
			else
			{
				CCompositedBackdropVisual::SetGdiWindowRegion(nullptr);
			}
		}
	}

	CCompositedBackdropVisual::~CCompositedBackdropVisual()
	{
		CCompositedBackdropVisual::UninitializeVisual();
	}

	void CCompositedBackdropVisual::SetClientBlurRegion(HRGN region)
	{
		if (!m_clientBlurRgn)
		{
			m_clientBlurRgn.reset(CreateRectRgn(0, 0, 0, 0));
		}
		CopyRgn(m_clientBlurRgn.get(), region);
		m_backdropDataChanged = true;
	}

	void CCompositedBackdropVisual::SetCaptionRegion(HRGN region)
	{
		GetRgnBox(region, &m_captionRect);
		m_backdropDataChanged = true;
	}

	void CCompositedBackdropVisual::SetBorderRegion(HRGN region)
	{
		CopyRgn(m_borderRgn.get(), region);
		m_backdropDataChanged = true;
	}

	void CCompositedBackdropVisual::SetAccentRect(LPCRECT lprc)
	{
		if (lprc)
		{
			m_accentRect = *lprc;
			m_gdiWindowRgn.reset();
		}
		else if (m_accentRect.has_value())
		{
			m_accentRect = std::nullopt;
			if (m_kind == CompositedBackdropKind::Accent)
			{
				wil::unique_hrgn clipRgn{ CreateRectRgn(0, 0, 0, 0) };
				if (GetWindowRgn(m_data->GetHWND(), clipRgn.get()) != ERROR)
				{
					m_gdiWindowRgn.reset(clipRgn.release());
				}
				else
				{
					m_gdiWindowRgn.reset();
				}
			}
		}
		m_backdropDataChanged = true;
	}

	void CCompositedBackdropVisual::SetGdiWindowRegion(HRGN region)
	{
		if (region)
		{
			if (!m_gdiWindowRgn)
			{
				m_gdiWindowRgn.reset(CreateRectRgn(0, 0, 0, 0));
			}
			CopyRgn(m_gdiWindowRgn.get(), region);
			m_accentRect = std::nullopt;
		}
		else if (m_gdiWindowRgn)
		{
			m_gdiWindowRgn.reset();
			m_accentRect = std::nullopt;
		}
		m_backdropDataChanged = true;
	}

	void CCompositedBackdropVisual::ValidateVisual()
	{
		if (GetSrcWindow()->IsTrullyMinimized())
			return;

		if (m_visible)
		{
			if (!CheckDeviceState(m_dcompDevice))
			{
				OnDeviceLost();
			}
		}

		OnBackdropKindUpdated(CustomBackdrop::GetActualBackdropKind(GetSrcWindow()));
		if (m_backdropDataChanged) { HandleChanges(); }
		if (m_visible)
		{
			OnBackdropBrushUpdated();
			m_reflectionVisual->ValidateVisual();
		}
	}

	void CCompositedBackdropVisual::UpdateNCBackground()
	{
		if (GetSrcWindow()->IsTrullyMinimized())
		{
			return;
		}

		RECT borderRect{};
		THROW_HR_IF_NULL(E_INVALIDARG, GetSrcWindow()->GetActualWindowRect(&borderRect, true, true, false));

		if (!EqualRect(&m_windowRect, &borderRect))
		{
			m_windowRect = borderRect;
			m_backdropDataChanged = true;
		}
	}

	HRESULT CClonedPeekingBackdropVisual::InitializeVisual()
	{
		RETURN_IF_FAILED(CBackdropVisual::InitializeVisual());

		auto compositor{ m_dcompDevice.as<wuc::Compositor>() };
		m_reflectionVisual->InitializeVisual(compositor);
		m_containerVisual = compositor.CreateContainerVisual();
		m_roundedGeometry = compositor.CreateRoundedRectangleGeometry();
		m_roundedClip = compositor.CreateGeometricClip(m_roundedGeometry);
		{
			HWND hwnd{ m_data->GetHWND() };
			RECT windowRect{}, borderRect{};
			auto window{ m_data->GetWindow() };
			window->GetActualWindowRect(&windowRect, false, true, true);
			window->GetActualWindowRect(&borderRect, false, true, false);
			MARGINS margins{};
			//window->GetBorderMargins(&margins);
			winrt::Windows::Foundation::Numerics::float3 offset
			{
				static_cast<float>(0.f),
				static_cast<float>(!IsMaximized(hwnd) ? 0.f : margins.cyTopHeight),
				1.f
			};
			winrt::Windows::Foundation::Numerics::float2 size
			{
				static_cast<float>(wil::rect_width(borderRect) + (IsMaximized(hwnd) ? margins.cxRightWidth + margins.cxLeftWidth : 0)),
				static_cast<float>(wil::rect_height(borderRect))
			};
			m_containerVisual.Offset(offset);
			m_containerVisual.Size(size);
			m_roundedGeometry.Size(size);
			m_roundedGeometry.CornerRadius(wfn::float2{ (float)g_configData.extendRound, (float)g_configData.extendRound });
			m_reflectionVisual->NotifyOffsetToWindow(offset);
		}
		m_containerVisual.Clip(m_roundedClip);
		m_containerVisual.Children().InsertAtTop(m_reflectionVisual->GetVisual());
		m_reflectionVisual->ValidateVisual();
		m_visualCollection.InsertAtBottom(m_containerVisual);

		return S_OK;
	}

	void CClonedPeekingBackdropVisual::UninitializeVisual()
	{
		m_roundedClip = nullptr;
		m_roundedGeometry = nullptr;
		m_containerVisual = nullptr;

		m_reflectionVisual->UninitializeVisual();
		CBackdropVisual::UninitializeVisual();
	}

	HRESULT CClonedCompositedBackdropVisual::InitializeVisual()
	{
		RETURN_IF_FAILED(CBackdropVisual::InitializeVisual());

		auto compositor{ m_dcompDevice.as<wuc::Compositor>() };
		m_reflectionVisual->InitializeVisual(compositor);
		m_containerVisual = compositor.CreateContainerVisual();
		m_spriteVisual = compositor.CreateSpriteVisual();
		m_spriteVisual.RelativeSizeAdjustment({ 1.f, 1.f });

		m_reflectionVisual->NotifyOffsetToWindow(m_source->m_reflectionVisual->GetOffset());
		m_reflectionVisual->ValidateVisual();

		RECT regionBox{};
		GetRgnBox(m_source->m_compositedRgn.get(), &regionBox);

		wfn::float3 offset{ static_cast<float>(regionBox.left), static_cast<float>(regionBox.top), 0.f };
		m_containerVisual.Offset(offset);
		m_containerVisual.Size({ static_cast<float>(max(wil::rect_width(regionBox), 0)), static_cast<float>(max(wil::rect_height(regionBox), 0)) });

		m_containerVisual.Clip(compositor.CreateGeometricClip(m_source->m_pathGeometry));
		m_containerVisual.Children().InsertAtTop(m_reflectionVisual->GetVisual());

		m_spriteVisual.Brush(m_source->m_spriteVisual.Brush());
		m_containerVisual.Children().InsertAtBottom(m_spriteVisual);
		m_visualCollection.InsertAtBottom(m_containerVisual);

		return S_OK;
	}

	void CClonedCompositedBackdropVisual::UninitializeVisual()
	{
		m_spriteVisual = nullptr;
		m_containerVisual = nullptr;
		m_source = nullptr;
		m_reflectionVisual->UninitializeVisual();
		CBackdropVisual::UninitializeVisual();
	}
}
