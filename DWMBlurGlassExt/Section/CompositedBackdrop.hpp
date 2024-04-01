#pragma once
#include "DWMStruct.h"
#include "../Backdrops/uDwmBackdrop.hpp"
#include "../Backdrops/DCompBackdrop.hpp"
#include "../Backdrops/AcrylicBackdrop.hpp"
#include "../Backdrops/MicaBackdrop.hpp"
#include "../Backdrops/AeroBackdrop.hpp"
#include "../Backdrops/BlurBackdrop.hpp"
#include "../Backdrops/GlassReflectionBackdrop.hpp"
#include "VersionHelper.h"
#include "CommonDef.h"
#include <windowsx.h>

namespace MDWMBlurGlassExt
{
	struct CNCVisualManager
	{
		DWM::CTopLevelWindow* window;

		CNCVisualManager(DWM::CTopLevelWindow* topLevelWindow = nullptr) : window{topLevelWindow} {}
		void UpdateCloak(bool cloak)
		{
			if (!window)
			{
				return;
			}

			DWM::CVisual* legacy{ nullptr };
			DWM::CVisual* accent{ nullptr };
			DWM::CVisual* clientBlur{ nullptr };
			DWM::CVisual* systemBackdrop{ nullptr };
			DWM::CVisual* accentColor{ nullptr };

			if (cloak)
			{
				if ((legacy = window->GetLegacyVisual()))
				{
					legacy->Cloak(cloak);
				}
				if ((accent = window->GetAccent()))
				{
					accent->Cloak(cloak);
				}
				if ((clientBlur = window->GetClientBlurVisual()))
				{
					clientBlur->Cloak(cloak);
				}
				if ((systemBackdrop = window->GetSystemBackdropVisual()))
				{
					systemBackdrop->Cloak(cloak);
				}
				if ((accentColor = window->GetAccentColorVisual()))
				{
					accentColor->Cloak(cloak);
				}
			}
			else
			{
				if ((legacy = window->GetLegacyVisual()))
				{
					legacy->Cloak(cloak);
				}
				if ((accent = window->GetAccent()))
				{
					accent->Cloak(cloak);
				}
				if ((clientBlur = window->GetClientBlurVisual()))
				{
					clientBlur->Cloak(cloak);
				}
				if ((systemBackdrop = window->GetSystemBackdropVisual()))
				{
					systemBackdrop->Cloak(cloak);
				}
				if ((accentColor = window->GetAccentColorVisual()))
				{
					accentColor->Cloak(cloak);
				}
			}
		}
	};

	class CCompositedBackdrop : public winrt::implements<CCompositedBackdrop, IUnknown>
	{
	private:
		struct WindowContext
		{
			bool useDarkMode{ false };
			bool windowActivated{ false };
			enum class WindowState : UCHAR
			{
				Normal,
				Minimized,
				Maximized
			} windowState{ WindowState::Normal };
			bool updateNow{ true };
		} m_windowContext;

		MDWMBlurGlass::effectType m_type{ MDWMBlurGlass::effectType::None };
		bool m_enableGlassReflection{ false };
		bool m_enableBorders{ false };

		winrt::Windows::Foundation::Numerics::float3 m_offset{ 0.f, 0.f, 1.f };
		winrt::Windows::Foundation::Numerics::float2 m_size{ 0.f, 0.f };
		wil::unique_hrgn m_clipRgn{ nullptr };
		wil::unique_hrgn m_borderRgn{ CreateRectRgn(0, 0, 0, 0) };

		winrt::com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner> m_interopDCompDevice{ nullptr };
		winrt::com_ptr<DWM::CCanvasVisual> m_borderVisual{ nullptr };
		winrt::com_ptr<DWM::CSolidColorLegacyMilBrushProxy> m_milBrush{ nullptr };
		DWM::CTopLevelWindow* m_window{ nullptr };
		DWM::CTopLevelWindow* m_sourceWindow{ nullptr };
		DWM::CWindowData* m_windowData{ nullptr };
		std::unique_ptr<CDCompBackdrop> m_backdrop{ nullptr };
		std::unique_ptr<CGlassReflectionBackdrop> m_glassReflection{ nullptr };
		CSharedVisual m_sharedVisual{};
		CNCVisualManager m_ncVisualManager{ nullptr };
		winrt::com_ptr<CCompositedBackdrop> m_sourceBackdropForAnimationUse{};

		DWM::CTopLevelWindow* GetSourceWindow() const
		{
			return m_sourceWindow ? m_sourceWindow : m_window;
		}
		DWM::CTopLevelWindow* GetTargetWindow() const
		{
			return m_window;
		}
		void InitializeVisualConnection(bool disconnectPrevious)
		{
			THROW_IF_FAILED(
				DWM::CCanvasVisual::Create(
					m_borderVisual.put()
				)
			);
			m_interopDCompDevice.copy_from(
				DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice()
			);
			if (disconnectPrevious && GetTargetWindow())
			{
				THROW_IF_FAILED(
					GetTargetWindow()->GetVisual()->GetVisualCollection()->Remove(
						m_borderVisual.get()
					)
				);
				THROW_IF_FAILED(
					GetTargetWindow()->GetVisual()->GetVisualCollection()->Remove(
						m_sharedVisual.udwmVisual.get()
					)
				);
			}

			THROW_IF_FAILED(m_sharedVisual.InitializeDCompAnduDwmVisual(m_interopDCompDevice.get()));
			if (GetTargetWindow())
			{
				THROW_IF_FAILED(
					GetTargetWindow()->GetVisual()->GetVisualCollection()->InsertRelative(
						m_borderVisual.get(),
						nullptr,
						true,
						true
					)
				);
				m_borderVisual->AllowVisualTreeClone(false);
				THROW_IF_FAILED(
					GetTargetWindow()->GetVisual()->GetVisualCollection()->InsertRelative(
						m_sharedVisual.udwmVisual.get(),
						nullptr,
						true,
						true
					)
				);
			}
			m_sharedVisual.udwmVisual->AllowVisualTreeClone(false);

			if (disconnectPrevious)
			{
				m_backdrop.reset();
				m_glassReflection.reset();
				m_offset = { 0.f, 0.f, 1.f };
				m_size = { 0.f, 0.f };
			}
		}
	public:
		const CSharedVisual& GetSharedVisual() const
		{
			return m_sharedVisual;
		}
		void Reset()
		{
			m_interopDCompDevice = nullptr;
		}
		CCompositedBackdrop(DWM::CTopLevelWindow* window) : 
			m_window{ window },
			m_windowData{ window->GetData() }, 
			m_sharedVisual{},
			m_ncVisualManager{ window }
		{
			m_ncVisualManager.UpdateCloak(true);

			InitializeVisualConnection(false);
		}
		CCompositedBackdrop(
			const CCompositedBackdrop* backdrop, 
			DWM::CTopLevelWindow* dest
		) : 
			m_window{ dest },
			m_sourceWindow{ backdrop->m_window },
			m_windowData{ backdrop->m_windowData }, 
			m_sharedVisual{}
		{
			InitializeVisualConnection(false);
			
			bool allowNCBackgroundVisual{ backdrop->GetSourceWindow()->IsNCBackgroundVisualsCloneAllAllowed() };
			{
				UpdateEffectType(allowNCBackgroundVisual ? backdrop->m_type : MDWMBlurGlass::effectType::None);
				UpdateGlassReflectionState(backdrop->m_enableGlassReflection);
				UpdateBordersState(allowNCBackgroundVisual ? backdrop->m_enableBorders : false);
				try
				{
					THROW_IF_FAILED(UpdateClipRegion(allowNCBackgroundVisual ? backdrop->m_clipRgn.get() : nullptr));
					THROW_IF_FAILED(
						UpdateRootAndChildren(
							backdrop->m_offset,
							backdrop->m_size,
							backdrop->m_windowContext
						)
					);
					THROW_IF_FAILED(UpdateBorderVisual());
				}
				catch (...) { }
			}
		}
		CCompositedBackdrop(
			CCompositedBackdrop* backdrop
		) : CCompositedBackdrop{ backdrop, nullptr }
		{
			m_sourceBackdropForAnimationUse.copy_from(backdrop);
			m_sourceBackdropForAnimationUse->m_sharedVisual.udwmVisual->ConnectToParent(false);
			m_sourceBackdropForAnimationUse->m_sharedVisual.udwmVisual->Cloak(true);
		}
		~CCompositedBackdrop()
		{
			if (GetTargetWindow())
			{
				m_ncVisualManager.UpdateCloak(false);
				THROW_IF_FAILED(
					GetTargetWindow()->GetVisual()->GetVisualCollection()->Remove(
						m_sharedVisual.udwmVisual.get()
					)
				);
				THROW_IF_FAILED(
					GetTargetWindow()->GetVisual()->GetVisualCollection()->Remove(
						m_borderVisual.get()
					)
				);
			}
			if (m_sourceBackdropForAnimationUse)
			{
				m_sourceBackdropForAnimationUse->m_sharedVisual.udwmVisual->ConnectToParent(true);
				m_sourceBackdropForAnimationUse->m_sharedVisual.udwmVisual->Cloak(false);
			}
		}

		CDCompBackdrop* GetMainBackdrop() const
		{
			return m_backdrop.get();
		}

		void UpdateEffectType(MDWMBlurGlass::effectType type)
		{
			if (m_type != type)
			{
				m_type = type;
				if (
					m_sharedVisual.layerVisual &&
					m_backdrop &&
					m_backdrop->spriteVisual
				)
				{
					auto visualCollection{ m_sharedVisual.layerVisual.Children() };
					visualCollection.Remove(m_backdrop->spriteVisual);
					m_backdrop.reset();
				}
			}
		}
		void UpdateGlassReflectionState(bool enable)
		{
			if (m_enableGlassReflection != enable)
			{
				m_enableGlassReflection = enable;
				if (
					m_sharedVisual.layerVisual &&
					m_glassReflection &&
					m_glassReflection->spriteVisual
				)
				{
					auto visualCollection{ m_sharedVisual.layerVisual.Children() };
					visualCollection.Remove(m_glassReflection->spriteVisual);
					m_glassReflection.reset();
				}
			}
		}
		void UpdateBordersState(bool enable)
		{
			if (m_enableBorders != enable)
			{
				m_enableBorders = enable;

				m_milBrush = nullptr;
				THROW_IF_FAILED(m_borderVisual->ClearInstructions());
			}
		}

		HRESULT UpdateClipRegion(HRGN hrgn) try
		{
			if (hrgn && !EqualRgn(hrgn, m_clipRgn.get()))
			{
				winrt::com_ptr<DWM::CRgnGeometryProxy> geometry{nullptr};
				THROW_IF_FAILED(
					DWM::ResourceHelper::CreateGeometryFromHRGN(
						hrgn,
						geometry.put()
					)
				);
				m_sharedVisual.udwmVisual->GetVisualProxy()->SetClip(geometry.get());

				m_clipRgn.reset(CreateRectRgn(0, 0, 0, 0));
				CopyRgn(m_clipRgn.get(), hrgn);
			}
			if (!hrgn && m_clipRgn)
			{
				m_sharedVisual.udwmVisual->GetVisualProxy()->SetClip(nullptr);
				m_clipRgn.reset();
			}

			return S_OK;
		}
		CATCH_RETURN()

		HRESULT UpdateRootAndChildren(
			const winrt::Windows::Foundation::Numerics::float3& offset,
			const winrt::Windows::Foundation::Numerics::float2& size,
			const WindowContext& context
		) try
		{
			auto is_visual_empty = [&]
			{
				return
					(
						(size.x <= 0.f || size.y <= 0.f) ||
						(
							context.windowState == WindowContext::WindowState::Minimized &&
							m_windowContext.windowState == WindowContext::WindowState::Minimized
						) ||
						(m_clipRgn && EqualRgn(m_clipRgn.get(), wil::unique_hrgn{CreateRectRgn(0, 0, 0, 0)}.get()))
					);
			};
			auto update_visual_info = [&]
			{
				if (
					m_offset != offset || 
					m_size != size || 
					memcmp(&context, &m_windowContext, sizeof(WindowContext)) != 0
				)
				{
					m_offset = offset;
					m_size = size;
					m_windowContext = context;

					m_sharedVisual.layerVisual.Offset(m_offset);
					m_sharedVisual.layerVisual.Size(m_size);
				}

				THROW_IF_FAILED(UpdateBackdrop());
				THROW_IF_FAILED(UpdateGlassReflection());
				THROW_IF_FAILED(UpdateBorderVisual());
				m_ncVisualManager.UpdateCloak(true);
			};

			if (m_sharedVisual.layerVisual.IsVisible())
			{
				if (is_visual_empty())
				{
					m_sharedVisual.layerVisual.IsVisible(false);
				}
				update_visual_info();
			}
			else if (!is_visual_empty())
			{
				m_sharedVisual.layerVisual.IsVisible(true);
				update_visual_info();
			}

			return S_OK;
		}
		CATCH_RETURN()

		HRESULT UpdateBackdrop() try
		{
			auto visualCollection{ m_sharedVisual.layerVisual.Children() };
			if (!m_backdrop && m_type != MDWMBlurGlass::effectType::None)
			{
				switch (m_type)
				{
					case MDWMBlurGlass::effectType::Acrylic:
					{
						m_backdrop.reset(new CAcrylicBackdrop);
						THROW_IF_FAILED(
							m_backdrop->Initialize(
								visualCollection
							)
						);
						break;
					}
					case MDWMBlurGlass::effectType::Mica:
					{
						m_backdrop.reset(new CMicaBackdrop);
						THROW_IF_FAILED(
							m_backdrop->Initialize(
								visualCollection
							)
						);
						break;
					}
					case MDWMBlurGlass::effectType::Aero:
					{
						m_backdrop.reset(new CAeroBackdrop);
						THROW_IF_FAILED(
							m_backdrop->Initialize(
								visualCollection
							)
						);
						break;
					}
					case MDWMBlurGlass::effectType::Blur:
					{
						m_backdrop.reset(new CBlurBackdrop);
						THROW_IF_FAILED(
							m_backdrop->Initialize(
								visualCollection
							)
						);
						break;
					}
					default:
						break;
				}
			}
			if (m_backdrop)
			{
				THROW_IF_FAILED(
					m_backdrop->Update(
						m_windowContext.useDarkMode,
						m_windowContext.windowActivated
					)
				);
			}

			return S_OK;
		}
		CATCH_RETURN()

		HRESULT UpdateGlassReflection() try
		{
			auto visualCollection{ m_sharedVisual.layerVisual.Children() };
			if (!m_glassReflection && m_enableGlassReflection)
			{
				m_glassReflection.reset(new CGlassReflectionBackdrop);
				THROW_IF_FAILED(
					m_glassReflection->Initialize(visualCollection)
				);
			}
			if (m_glassReflection)
			{
				THROW_IF_FAILED(
					m_glassReflection->Update(
						GetSourceWindow()
					)
				);
			}

			return S_OK;
		}
		CATCH_RETURN()

		HRESULT UpdateBorderVisual() try
		{
			if (m_enableBorders)
			{
				RECT borderRect
				{
					static_cast<LONG>(m_offset.x),
					static_cast<LONG>(m_offset.y),
					static_cast<LONG>(m_offset.x) + static_cast<LONG>(m_size.x),
					static_cast<LONG>(m_offset.y) + static_cast<LONG>(m_size.y)
				};
				wil::unique_hrgn borderRgn{ CreateRectRgnIndirect(&borderRect) };
				{
					borderRect.left += 1;
					borderRect.top += 1;
					borderRect.right -= 1;
					borderRect.bottom -= 1;
				}
				wil::unique_hrgn innerRgn{ CreateRectRgnIndirect(&borderRect) };
				CombineRgn(borderRgn.get(), borderRgn.get(), innerRgn.get(), RGN_XOR);

				auto milBrushPtr{ GetSourceWindow()->GetBorderMilBrush() };
				if (
					(
						DWM::CDesktopManager::s_pDesktopManagerInstance->IsVanillaTheme() &&
						MDWMBlurGlass::os::buildNumber < 22000
					) &&
					milBrushPtr &&
					(
						m_milBrush.get() != *milBrushPtr ||
						!EqualRgn(borderRgn.get(), m_borderRgn.get())
					) &&
					m_windowContext.windowState != WindowContext::WindowState::Maximized
				)
				{
					THROW_IF_FAILED(m_borderVisual->ClearInstructions());
					m_milBrush.copy_from(
						*GetSourceWindow()->GetBorderMilBrush()
					);
					CopyRgn(m_borderRgn.get(), borderRgn.get());

					winrt::com_ptr<DWM::CRgnGeometryProxy> geometry{ nullptr };
					THROW_IF_FAILED(
						DWM::ResourceHelper::CreateGeometryFromHRGN(
							m_borderRgn.get(),
							geometry.put()
						)
					);
					if (
						m_milBrush &&
						geometry
					)
					{
						winrt::com_ptr<DWM::CDrawGeometryInstruction> instruction{ nullptr };
						THROW_IF_FAILED(
							DWM::CDrawGeometryInstruction::Create(
								m_milBrush.get(),
								geometry.get(),
								instruction.put()
							)
						);
						THROW_IF_FAILED(
							m_borderVisual->AddInstruction(
								instruction.get()
							)
						);
					}
				}
				else if (m_windowContext.windowState == WindowContext::WindowState::Maximized)
				{
					THROW_IF_FAILED(m_borderVisual->ClearInstructions());
				}
			}
			
			return S_OK;
		}
		CATCH_RETURN()

		WindowContext GetCachedWindowContext() const
		{
			return m_windowContext;
		}
		WindowContext QueryCurrentWindowContext()
		{
			HWND hwnd{ m_windowData->GetHWND() };

			WindowContext windowContext
			{
				m_windowData->IsUsingDarkMode(),
				GetSourceWindow()->TreatAsActiveWindow(),
				WindowContext::WindowState::Normal,
				false
			};
			if (IsMaximized(hwnd)) { windowContext.windowState = WindowContext::WindowState::Maximized; }
			else if (IsMinimized(hwnd)) { windowContext.windowState = WindowContext::WindowState::Minimized; }

			return windowContext;
		}

		HRESULT Update() try
		{
			auto is_device_valid = [&]()
			{
				if (!m_interopDCompDevice) return false;

				BOOL valid{ FALSE };
				THROW_IF_FAILED(
					m_interopDCompDevice.as<IDCompositionDevice>()->CheckDeviceState(
						&valid
					)
				);

				return valid == TRUE;
			};
			if (!is_device_valid())
			{
				InitializeVisualConnection(true);
				UpdateClipRegion(wil::unique_hrgn{ m_clipRgn.release() }.get());
			}

			HWND hwnd{ m_windowData->GetHWND() };
			const auto windowContext{ QueryCurrentWindowContext() };

			RECT windowRect{}, borderRect{};
			THROW_HR_IF_NULL(E_INVALIDARG, GetSourceWindow()->GetActualWindowRect(&windowRect, false, true, true));
			THROW_HR_IF_NULL(E_INVALIDARG, GetSourceWindow()->GetActualWindowRect(&borderRect, false, true, false));
			MARGINS margins{};
			GetSourceWindow()->GetBorderMargins(&margins);
			winrt::Windows::Foundation::Numerics::float3 offset
			{
				static_cast<float>(!IsMaximized(hwnd) ? (windowRect.left - borderRect.left) : margins.cxLeftWidth),
				static_cast<float>(!IsMaximized(hwnd) ? (windowRect.top - borderRect.top) : margins.cyTopHeight),
				1.f
			};
			winrt::Windows::Foundation::Numerics::float2 size
			{
				IsMinimized(hwnd) ? m_size.x : static_cast<float>(wil::rect_width(windowRect)),
				IsMinimized(hwnd) ? m_size.y : static_cast<float>(wil::rect_height(windowRect))
			};

			UpdateRootAndChildren(
				offset,
				size,
				windowContext
			);

			return S_OK;
		}
		CATCH_RETURN()
	};

	class CCompositedAccentBackdrop : public winrt::implements<CCompositedAccentBackdrop, IUnknown>
	{
	private:
		struct WindowContext
		{
			enum class WindowState : UCHAR
			{
				Normal,
				Minimized,
				Maximized
			} windowState{ WindowState::Normal };
			bool updateNow{ true };
		} m_windowContext;

		MDWMBlurGlass::effectType m_type{ MDWMBlurGlass::effectType::None };
		bool m_enableGlassReflection{ false };

		winrt::Windows::Foundation::Numerics::float3 m_offset{ 0.f, 0.f, 1.f };
		winrt::Windows::Foundation::Numerics::float2 m_size{ 0.f, 0.f };
		RECT m_accentBlurRect{};
		DWM::ACCENT_POLICY m_accentPolicy{ DWM::ACCENT_INVALID_STATE };

		winrt::com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner> m_interopDCompDevice{ nullptr };
		DWM::CTopLevelWindow* m_window{ nullptr };
		DWM::CTopLevelWindow* m_sourceWindow{ nullptr };
		winrt::com_ptr<DWM::CBaseGeometryProxy> m_clipRegion{ nullptr };

		DWM::CWindowData* m_windowData{ nullptr };
		std::unique_ptr<CAccentDCompBackdrop> m_backdrop{ nullptr };
		std::unique_ptr<CGlassReflectionBackdrop> m_glassReflection{ nullptr };
		CSharedVisual m_sharedVisual{};
		CNCVisualManager m_ncVisualManager{ nullptr };
		winrt::com_ptr<CCompositedAccentBackdrop> m_sourceBackdropForAnimationUse{};

		DWM::CTopLevelWindow* GetSourceWindow() const
		{
			return m_sourceWindow ? m_sourceWindow : m_window;
		}
		DWM::CTopLevelWindow* GetTargetWindow() const
		{
			return m_window;
		}
		void InitializeVisualConnection(bool disconnectPrevious)
		{
			m_interopDCompDevice.copy_from(
				DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice()
			);

			if (disconnectPrevious && GetTargetWindow())
			{
				THROW_IF_FAILED(
					GetTargetWindow()->GetVisual()->GetVisualCollection()->Remove(
						m_sharedVisual.udwmVisual.get()
					)
				);
			}
			THROW_IF_FAILED(m_sharedVisual.InitializeDCompAnduDwmVisual(m_interopDCompDevice.get()));

			auto compositor{ m_interopDCompDevice.as<winrt::Windows::UI::Composition::Compositor>() };
			// TO-DO: Add accent dropshadow
			// ...

			if (GetTargetWindow())
			{
				THROW_IF_FAILED(
					GetTargetWindow()->GetVisual()->GetVisualCollection()->InsertRelative(
						m_sharedVisual.udwmVisual.get(),
						nullptr,
						true,
						true
					)
				);
			}
			m_sharedVisual.udwmVisual->AllowVisualTreeClone(false);
			if (disconnectPrevious)
			{
				m_backdrop.reset();
				m_glassReflection.reset();
				m_offset = { 0.f, 0.f, 1.f };
				m_size = { 0.f, 0.f };
				m_accentPolicy = { DWM::ACCENT_INVALID_STATE };
			}
		}
		bool IsContainShadow()
		{
			return 
				(
					(m_accentPolicy.nFlags & (1 << 5)) ||
					(m_accentPolicy.nFlags & (1 << 6)) ||
					(m_accentPolicy.nFlags & (1 << 7)) ||
					(m_accentPolicy.nFlags & (1 << 8))
				);
		}
	public:
		const CSharedVisual& GetSharedVisual() const
		{
			return m_sharedVisual;
		}
		void Reset()
		{
			m_interopDCompDevice = nullptr;
		}
		CCompositedAccentBackdrop(DWM::CTopLevelWindow* window) :
			m_window{ window },
			m_windowData{ window->GetData() },
			m_sharedVisual{},
			m_ncVisualManager{ window }
		{
			m_ncVisualManager.UpdateCloak(true);
			InitializeVisualConnection(false);
		}
		CCompositedAccentBackdrop(const CCompositedAccentBackdrop* backdrop, DWM::CTopLevelWindow* dest) :
			m_window{ dest },
			m_sourceWindow{ backdrop->m_window },
			m_windowData{ backdrop->m_windowData },
			m_sharedVisual{}
		{
			InitializeVisualConnection(false);

			bool allowAccentVisual{ backdrop->GetSourceWindow()->GetAccent()->IsCloneAllowed() || backdrop->GetSourceWindow()->IsNCBackgroundVisualsCloneAllAllowed() };
			{
				UpdateEffectType(allowAccentVisual ? backdrop->m_type : MDWMBlurGlass::effectType::None);
				UpdateGlassReflectionState(backdrop->m_enableGlassReflection);
				THROW_IF_FAILED(UpdateClipRegion());
				THROW_IF_FAILED(
					UpdateRootAndChildren(
						backdrop->m_offset,
						backdrop->m_size,
						backdrop->m_windowContext
					)
				);
			}
		}
		CCompositedAccentBackdrop(
			CCompositedAccentBackdrop* backdrop
		) : CCompositedAccentBackdrop{ backdrop, nullptr }
		{
			m_sourceBackdropForAnimationUse.copy_from(backdrop);
			m_sourceBackdropForAnimationUse->m_sharedVisual.udwmVisual->ConnectToParent(false);
			m_sourceBackdropForAnimationUse->m_sharedVisual.udwmVisual->Cloak(true);
		}
		~CCompositedAccentBackdrop()
		{
			if (GetTargetWindow())
			{
				m_ncVisualManager.UpdateCloak(false);
				THROW_IF_FAILED(
					GetTargetWindow()->GetVisual()->GetVisualCollection()->Remove(
						m_sharedVisual.udwmVisual.get()
					)
				);
			}
			if (m_sourceBackdropForAnimationUse)
			{
				m_sourceBackdropForAnimationUse->m_sharedVisual.udwmVisual->ConnectToParent(true);
				m_sourceBackdropForAnimationUse->m_sharedVisual.udwmVisual->Cloak(false);
			}
		}

		void UpdateEffectType(MDWMBlurGlass::effectType type)
		{
			if (m_type != type)
			{
				m_type = type;
				if (
					m_sharedVisual.layerVisual &&
					m_backdrop &&
					m_backdrop->spriteVisual
				)
				{
					auto visualCollection{ m_sharedVisual.layerVisual.Children() };
					visualCollection.Remove(m_backdrop->spriteVisual);
					m_backdrop.reset();
				}
			}
		}
		void UpdateGlassReflectionState(bool enable)
		{
			if (m_enableGlassReflection != enable)
			{
				m_enableGlassReflection = enable;
				if (
					m_sharedVisual.layerVisual &&
					m_glassReflection &&
					m_glassReflection->spriteVisual
				)
				{
					auto visualCollection{ m_sharedVisual.layerVisual.Children() };
					visualCollection.Remove(m_glassReflection->spriteVisual);
					m_glassReflection.reset();
				}
			}
		}

		HRESULT UpdateClipRegion()
		{
			auto geometry{ GetSourceWindow()->GetAccent()->GetClipGeometry() };
			if (m_clipRegion.get() != geometry)
			{
				m_clipRegion.copy_from(geometry);
				m_sharedVisual.udwmVisual->GetVisualProxy()->SetClip(geometry);
			}
			return S_OK;
		}

		HRESULT UpdateRootAndChildren(
			const winrt::Windows::Foundation::Numerics::float3& offset,
			const winrt::Windows::Foundation::Numerics::float2& size,
			const WindowContext& context
		) try
		{
			auto is_visual_empty = [&]
			{
				return
					(
						(size.x <= 0.f || size.y <= 0.f) ||
						(
							context.windowState == WindowContext::WindowState::Minimized &&
							m_windowContext.windowState == WindowContext::WindowState::Minimized
						)
					);
			};
			auto update_visual_info = [&]
			{
				if (
					m_offset != offset ||
					m_size != size ||
					memcmp(&context, &m_windowContext, sizeof(WindowContext)) != 0
				)
				{
					m_offset = offset;
					m_size = size;
					m_windowContext = context;

					m_sharedVisual.layerVisual.Offset(m_offset);
					m_sharedVisual.layerVisual.Size(m_size);
				}

				THROW_IF_FAILED(UpdateGlassReflection());
				THROW_IF_FAILED(UpdateAccentPolicy());
				m_ncVisualManager.UpdateCloak(true);
			};

			if (m_sharedVisual.layerVisual.IsVisible())
			{
				if (is_visual_empty())
				{
					m_sharedVisual.layerVisual.IsVisible(false);
				}
				update_visual_info();
			}
			else if (!is_visual_empty())
			{
				m_sharedVisual.layerVisual.IsVisible(true);
				update_visual_info();
			}

			return S_OK;
		}
		CATCH_RETURN()

		HRESULT UpdateBackdrop() try
		{
			auto visualCollection{ m_sharedVisual.layerVisual.Children() };
			if (!m_backdrop && m_type != MDWMBlurGlass::effectType::None)
			{
				switch (m_type)
				{
				case MDWMBlurGlass::effectType::Acrylic:
				{
					m_backdrop.reset(new CAccentAcrylicBackdrop);
					THROW_IF_FAILED(
						m_backdrop->Initialize(
							visualCollection
						)
					);
					break;
				}
				case MDWMBlurGlass::effectType::Mica:
				{
					m_backdrop.reset(new CAccentMicaBackdrop);
					THROW_IF_FAILED(
						m_backdrop->Initialize(
							visualCollection
						)
					);
					break;
				}
				case MDWMBlurGlass::effectType::Aero:
				{
					m_backdrop.reset(new CAccentAeroBackdrop);
					THROW_IF_FAILED(
						m_backdrop->Initialize(
							visualCollection
						)
					);
					break;
				}
				case MDWMBlurGlass::effectType::Blur:
				{
					m_backdrop.reset(new CAccentBlurBackdrop);
					THROW_IF_FAILED(
						m_backdrop->Initialize(
							visualCollection
						)
					);
					break;
				}
				default:
					break;
				}
			}
			if (m_backdrop)
			{
				THROW_IF_FAILED(
					m_backdrop->Update(
						m_accentPolicy
					)
				);
			}

			return S_OK;
		}
		CATCH_RETURN()

		HRESULT UpdateGlassReflection() try
		{
			auto visualCollection{ m_sharedVisual.layerVisual.Children() };
			if (!m_glassReflection && m_enableGlassReflection)
			{
				m_glassReflection.reset(new CGlassReflectionBackdrop);
				THROW_IF_FAILED(
					m_glassReflection->Initialize(visualCollection)
				);
			}
			if (m_glassReflection)
			{
				THROW_IF_FAILED(
					m_glassReflection->Update(
						GetSourceWindow()
					)
				);
			}

			return S_OK;
		}
		CATCH_RETURN()

		HRESULT UpdateAccentPolicy()
		{
			if (memcmp(&m_accentPolicy, m_windowData->GetAccentPolicy(), sizeof(DWM::ACCENT_POLICY)) != 0)
			{
				m_accentPolicy = *m_windowData->GetAccentPolicy();
				RETURN_IF_FAILED(UpdateBackdrop());
			}
			return S_OK;
		}

		WindowContext QueryCurrentWindowContext()
		{
			HWND hwnd{ m_windowData->GetHWND() };

			WindowContext windowContext
			{
				WindowContext::WindowState::Normal,
				false
			};
			if (IsMaximized(hwnd)) { windowContext.windowState = WindowContext::WindowState::Maximized; }
			else if (IsMinimized(hwnd)) { windowContext.windowState = WindowContext::WindowState::Minimized; }

			return windowContext;
		}

		HRESULT Update() try
		{
			auto is_device_valid = [&]()
			{
				if (!m_interopDCompDevice) return false;

				BOOL valid{ FALSE };
				THROW_IF_FAILED(
					m_interopDCompDevice.as<IDCompositionDevice>()->CheckDeviceState(
						&valid
					)
				);

				return valid == TRUE;
			};
			if (!is_device_valid())
			{
				InitializeVisualConnection(true);
				THROW_IF_FAILED(UpdateClipRegion());
			}

			HWND hwnd{ m_windowData->GetHWND() };
			const auto windowContext{ QueryCurrentWindowContext() };

			RECT windowRect{}, borderRect{};
			THROW_HR_IF_NULL(E_INVALIDARG, GetSourceWindow()->GetActualWindowRect(&windowRect, false, true, true));
			THROW_HR_IF_NULL(E_INVALIDARG, GetSourceWindow()->GetActualWindowRect(&borderRect, false, true, false));
			MARGINS margins{};
			GetSourceWindow()->GetBorderMargins(&margins);
			winrt::Windows::Foundation::Numerics::float3 offset
			{
				static_cast<float>(0.f),
				static_cast<float>(!IsMaximized(hwnd) ? 0.f : margins.cyTopHeight),
				1.f
			};
			winrt::Windows::Foundation::Numerics::float2 size
			{
				IsMinimized(hwnd) ? m_size.x : static_cast<float>(wil::rect_width(borderRect) + (IsMaximized(hwnd) ? margins.cxRightWidth + margins.cxLeftWidth : 0)),
				IsMinimized(hwnd) ? m_size.y : static_cast<float>(wil::rect_height(borderRect))
			};

			THROW_IF_FAILED(UpdateClipRegion());
			THROW_IF_FAILED(
				UpdateRootAndChildren(
					offset,
					size,
					windowContext
				)
			);

			return S_OK;
		}
		CATCH_RETURN()
	};
}