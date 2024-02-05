/**
 * FileName: MDcompRender.cpp
 *
 * Copyright (C) 2024 Maplespe
 *
 * This file is part of MToolBox and DWMBlurGlass.
 * DWMBlurGlass is free software: you can redistribute it and/or modify it under the terms of the
 * GNU Lesser General Public License as published by the Free Software Foundation, either version 3
 * of the License, or any later version.
 *
 * DWMBlurGlass is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with Foobar.
 * If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/
#include "MDcompRender.h"
#include "winrt_impl.h"

namespace MDWMBlurGlass
{
	using namespace Mui;
	using namespace Render;

	bool MRender_DComp::InitRender(_m_uint width, _m_uint height) try
	{
		CoInitialize(nullptr);
		if (!m_Context && !m_Canvas)
		{
			check_hresult(GetD2DDevice()->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, &m_Context));
			m_Context->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
			m_Context->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
			m_Context->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

			com_ptr<IDXGIFactory2> dxgiFactory = nullptr;
			com_ptr<IDXGIAdapter> dxgiAdapter = nullptr;
			com_ptr<IDXGIDevice1> dxgiDevice{ GetDXGIDevice(), winrt::take_ownership_from_abi };

			check_hresult(dxgiDevice.get()->GetAdapter(dxgiAdapter.put()));
			check_hresult(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.put())));

			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
			swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			swapChainDesc.BufferCount = 3;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
			swapChainDesc.Width = width;
			swapChainDesc.Height = height;
			check_hresult(
				dxgiFactory->CreateSwapChainForComposition(dxgiDevice.get(), &swapChainDesc, nullptr, &m_swapChain)
			);
			check_hresult(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&m_backBuffer)));

			check_hresult(m_Context->CreateBitmapFromDxgiSurface(
				m_backBuffer,
				D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
				),
				&m_Display
			));

			//再创建一个画布 用于兼容GDI绘制
			auto base = static_cast<MRenderCmd*>(m_base);
			m_CanvasDef = base->CreateCanvas(width, height, true).cast<MCanvas_D2D>();
			//创建一个无关显示画布 给需要的控件临时使用
			m_CanvasTmp = base->CreateCanvas(width, height, true).cast<MCanvas_D2D>();
		}
		if (m_Context && m_Canvas)
		{
			SetCanvas(m_Canvas.get());
		}
		return true;
	}
	catch (...)
	{
		auto hr = winrt::to_hresult();
		return false;
	}

	HRESULT MRender_DComp::InitBackdrop() try
	{
		com_ptr<abi::System::IDispatcherQueueController> controller = nullptr;
		DispatcherQueueOptions options
		{
			sizeof(DispatcherQueueOptions),
			DQTYPE_THREAD_CURRENT,
			DQTAT_COM_NONE
		};
		check_hresult(CreateDispatcherQueueController(options, controller.put()));

		check_hresult(
			ABI::Windows::Foundation::ActivateInstance(
				Utils::abi_of(hstring(RuntimeClass_Windows_UI_Composition_Compositor)),
				m_compositor.put()
			)
		);

		com_ptr<ABI::Windows::System::IDispatcherQueue> queue = nullptr;
		controller->get_DispatcherQueue(queue.put());

		winrt::Windows::System::DispatcherQueueHandler callback([this]
		{
			com_ptr<abi::comp::ICompositionSurface> compositionSurface = nullptr;
			check_hresult(m_compositor.as<abi::comp::ICompositorInterop>()->CreateCompositionSurfaceForSwapChain(m_swapChain, compositionSurface.put()));
			check_hresult(m_compositor.as<abi::comp::ICompositorInterop>()->CreateGraphicsDevice(GetD2DDevice(), m_compositionDevice.put()));

			check_hresult(
				m_compositor.as<abi::desktop::ICompositorDesktopInterop>()->CreateDesktopWindowTarget(
					m_hWnd, FALSE, m_backdropTarget.put()
				)
			);

			com_ptr<abi::comp::IContainerVisual> layer = nullptr;
			check_hresult(
				m_compositor->CreateContainerVisual(layer.put())
			);
			check_hresult(
				m_compositor->CreateSpriteVisual(m_backdropVisual.put())
			);
			com_ptr<abi::comp::ISpriteVisual> renderLayer = nullptr;
			check_hresult(
				m_compositor->CreateSpriteVisual(renderLayer.put())
			);
			check_hresult(
				layer.as<abi::comp::IVisual2>()->put_RelativeSizeAdjustment({ 1.f, 1.f })
			);
			check_hresult(
				renderLayer.as<abi::comp::IVisual2>()->put_RelativeSizeAdjustment({ 1.f, 1.f })
			);
			check_hresult(
				m_backdropVisual.as<abi::comp::IVisual2>()->put_RelativeSizeAdjustment({ 1.f, 1.f })
			);

			com_ptr<abi::comp::IVisualCollection> collection = nullptr;
			layer->get_Children(collection.put());


			collection->InsertAtTop(m_backdropVisual.as<abi::comp::IVisual>().get());
			collection->InsertAtTop(renderLayer.as<abi::comp::IVisual>().get());

			check_hresult(
				m_compositor->CreateSurfaceBrush(m_surfaceBrush.put())
			);
			m_surfaceBrush->put_Surface(compositionSurface.get());

			renderLayer->put_Brush(m_surfaceBrush.as<abi::comp::ICompositionBrush>().get());

			check_hresult(
				m_backdropTarget.as<abi::comp::ICompositionTarget>()->put_Root(layer.as<abi::comp::IVisual>().get())
			);

			check_hresult(
				m_compositor.as<abi::comp::ICompositor3>()->CreateHostBackdropBrush(m_brush.put())
			);

			check_hresult(
				m_backdropVisual->put_Brush(
					m_brush.as<abi::comp::ICompositionBrush>().get()
				)
			);
		});

		boolean ret = false;
		queue->TryEnqueue(callback.as<ABI::Windows::System::IDispatcherQueueHandler>().get(), &ret);

		return S_OK;
	}
	catch (...)
	{
		return winrt::to_hresult();
	}
}