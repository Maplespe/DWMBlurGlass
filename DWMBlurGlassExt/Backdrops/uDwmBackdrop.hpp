#pragma once
#include "DWMStruct.h"
#include <wil.h>

namespace MDWMBlurGlassExt
{
	struct CSharedVisual
	{
		winrt::com_ptr<DWM::CVisual> udwmVisual{ nullptr };
		winrt::com_ptr<IDCompositionVisual2> dcompVisual{ nullptr };
		winrt::com_ptr<DCompPrivate::InteropCompositionTarget> interopCompositionTarget{ nullptr };
		winrt::Windows::UI::Composition::LayerVisual layerVisual{ nullptr };

		HRESULT STDMETHODCALLTYPE InitializeDCompAnduDwmVisual(DCompPrivate::IDCompositionDesktopDevicePartner* requestedInteropDCompDevice) try
		{
			winrt::com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner> interopDCompDevice{ nullptr };
			interopDCompDevice.copy_from(requestedInteropDCompDevice);

			THROW_IF_FAILED(
				interopDCompDevice->CreateSharedResource(
					IID_PPV_ARGS(interopCompositionTarget.put())
				)
			);

			auto interopCompositor{ interopDCompDevice.as<winrt::Windows::UI::Composition::Compositor>() };

			THROW_IF_FAILED(interopDCompDevice->CreateVisual(dcompVisual.put()));
			THROW_IF_FAILED(
				dcompVisual->SetCompositeMode(DCOMPOSITION_COMPOSITE_MODE_SOURCE_OVER)
			);
			THROW_IF_FAILED(
				dcompVisual->SetBitmapInterpolationMode(DCOMPOSITION_BITMAP_INTERPOLATION_MODE_LINEAR)
			);
			THROW_IF_FAILED(
				dcompVisual->SetBorderMode(DCOMPOSITION_BORDER_MODE_SOFT)
			);

			layerVisual = interopCompositor.CreateLayerVisual();
			auto interopVisual{ dcompVisual.as<DCompPrivate::IDCompositionVisualPartnerWinRTInterop>() };
			interopVisual->GetVisualCollection().InsertAtBottom(layerVisual);

			THROW_IF_FAILED(interopCompositionTarget->SetRoot(dcompVisual.get()));
			THROW_IF_FAILED(interopDCompDevice->Commit());

			wil::unique_handle resourceHandle{ nullptr };
			THROW_IF_FAILED(
				interopDCompDevice->OpenSharedResourceHandle(interopCompositionTarget.get(), resourceHandle.put())
			);

			THROW_IF_FAILED(DWM::CVisual::CreateFromSharedHandle(resourceHandle.get(), udwmVisual.put()));

			return S_OK;
		}
		CATCH_RETURN()
	};
}