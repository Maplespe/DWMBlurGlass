#pragma once
#include "DWMStruct.h"
#include <wil.h>

namespace MDWMBlurGlassExt
{
	namespace wu = winrt::Windows::UI;
	namespace wuc = winrt::Windows::UI::Composition;
	namespace wg = winrt::Windows::Graphics;
	namespace wfn = winrt::Windows::Foundation::Numerics;
	namespace wge = winrt::Windows::Graphics::Effects;
	namespace wgd = winrt::Windows::Graphics::DirectX;

	using namespace MDWMBlurGlass;

	template <bool insertAtBack>
	class CSpriteVisual
	{
	protected:
		DWM::CVisual* m_parentVisual{ nullptr };
		com_ptr<DWM::CVisual> m_udwmVisual{ nullptr };

		wuc::ContainerVisual m_rootVisual{ nullptr };
		com_ptr<DCompPrivate::InteropCompositionTarget> m_visualTarget{ nullptr };
		com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner> m_dcompDevice{ nullptr };

		wuc::VisualCollection m_visualCollection{ nullptr };

		void InitializeInteropDevice(DCompPrivate::IDCompositionDesktopDevicePartner* interopDevice)
		{
			m_dcompDevice.copy_from(interopDevice);
		}
		virtual HRESULT InitializeVisual()
		{
			auto compositor = m_dcompDevice.as<wuc::Compositor>();
			m_rootVisual = compositor.CreateContainerVisual();
			m_visualCollection = m_rootVisual.Children();

			// create shared target
			RETURN_IF_FAILED(
				m_dcompDevice->CreateSharedResource(
					IID_PPV_ARGS(m_visualTarget.put())
				)
			);
			RETURN_IF_FAILED(
				m_visualTarget.as<DCompPrivate::IVisualTargetPartner>()->SetRoot(
					m_rootVisual.as<ABI::Windows::UI::Composition::IVisual>().get()
				)
			);
			RETURN_IF_FAILED(m_dcompDevice->Commit());

			// interop with udwm and dwmcore
			wil::unique_handle resourceHandle{ nullptr };
			RETURN_IF_FAILED(
				m_dcompDevice->OpenSharedResourceHandle(m_visualTarget.get(), resourceHandle.put())
			);

			if (os::buildNumber < 18362)
			{
				UINT handleIndex{ 0 };
				RETURN_IF_FAILED(
					DWM::CDesktopManager::s_pDesktopManagerInstance->GetCompositor()->GetChannel()->DuplicateSharedResource(
						resourceHandle.get(),
						38,
						&handleIndex
					)
				);
				RETURN_IF_FAILED(DWM::CVisual::WrapExistingResource(DWM::CDesktopManager::s_pDesktopManagerInstance->GetCompositor()->GetChannel(), handleIndex, m_udwmVisual.put()));
			}
			else if (os::buildNumber < 19041)
			{
				UINT handleIndex{ 0 };
				RETURN_IF_FAILED(
					DWM::CDesktopManager::s_pDesktopManagerInstance->GetCompositor()->GetChannel()->DuplicateSharedResource(
						resourceHandle.get(),
						39,
						&handleIndex
					)
				);
				RETURN_IF_FAILED(DWM::CVisual::WrapExistingResource(handleIndex, m_udwmVisual.put()));
			}
			else
			{
				RETURN_IF_FAILED(DWM::CVisual::CreateFromSharedHandle(resourceHandle.get(), m_udwmVisual.put()));
			}
			m_udwmVisual->AllowVisualTreeClone(false);

			if (m_parentVisual)
			{
				RETURN_IF_FAILED(
					m_parentVisual->GetVisualCollection()->InsertRelative(
						m_udwmVisual.get(),
						nullptr,
						insertAtBack,
						true
					)
				);
			}

			return S_OK;
		}
		virtual void UninitializeVisual()
		{
			if (m_visualCollection)
			{
				m_visualCollection.RemoveAll();
			}
			if (m_parentVisual)
			{
				if (m_udwmVisual)
				{
					m_parentVisual->GetVisualCollection()->Remove(
						m_udwmVisual.get()
					);
				}

				m_udwmVisual = nullptr;
			}
			if (m_rootVisual)
			{
				m_visualCollection.RemoveAll();
				m_rootVisual = nullptr;
			}
			m_visualTarget = nullptr;
		}

		CSpriteVisual(DWM::CVisual* parentVisual) : m_parentVisual{ parentVisual } {}
		virtual ~CSpriteVisual() { CSpriteVisual::UninitializeVisual(); }
	};
	using CBackdropVisual = CSpriteVisual<true>;
	using COverlayVisual = CSpriteVisual<false>;

	template <bool insertAtBack>
	class CClonedSpriteVisual : CSpriteVisual<insertAtBack>
	{
	protected:
		wuc::RedirectVisual m_redirectVisual{ nullptr };
		wuc::Visual m_sourceVisual{ nullptr };

		HRESULT InitializeVisual() override
		{
			RETURN_IF_FAILED(CSpriteVisual<insertAtBack>::InitializeVisual());
			auto compositor{ this->m_dcompDevice.template as<wuc::Compositor>() };
			m_redirectVisual = compositor.CreateRedirectVisual(m_sourceVisual);
			this->m_visualCollection.InsertAtBottom(m_redirectVisual);

			return S_OK;
		}
		void UninitializeVisual() override
		{
			if (this->m_visualCollection)
			{
				this->m_visualCollection.RemoveAll();
			}
			m_redirectVisual = nullptr;
			CSpriteVisual<insertAtBack>::UninitializeVisual();
		}
		void OnDeviceLost()
		{
			UninitializeVisual();
			this->InitializeInteropDevice(DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice());
			InitializeVisual();
		}

		CClonedSpriteVisual(DWM::CVisual* parentVisual, const wuc::Visual& sourceVisual) :
			CSpriteVisual<insertAtBack>{ parentVisual },
			m_sourceVisual{ sourceVisual }
		{
		}

		~CClonedSpriteVisual() override { CClonedSpriteVisual::UninitializeVisual(); }
	};
	using CClonedBackdropVisual = CClonedSpriteVisual<true>;
	using CClonedOverlayVisual = CClonedSpriteVisual<false>;

	FORCEINLINE bool CheckDeviceState(const com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner>& dcompDevice)
	{
		if (dcompDevice.get() != DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice())
		{
			return false;
		}

		return true;
	}
}