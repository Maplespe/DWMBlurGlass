#pragma once
#include "DCompBackdrop.hpp"
#include <Shlwapi.h>

namespace MDWMBlurGlassExt
{
	struct CGlassReflectionResources : CDCompResourcesBase
	{
		winrt::Windows::UI::Composition::CompositionDrawingSurface drawingSurface{ nullptr };
		float parallaxIntensity{ 0.1f };
		float glassIntensity{ 0.8f };

		void ReloadParameters() override
		{
			glassIntensity = g_configData.glassIntensity;
		}
		HRESULT STDMETHODCALLTYPE EnsureGlassSurface() try
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
				winrt::Windows::UI::Composition::CompositionGraphicsDevice graphicsDevice{ nullptr };
				THROW_IF_FAILED(
					compositor.as<ABI::Windows::UI::Composition::ICompositorInterop>()->CreateGraphicsDevice(
						DWM::CDesktopManager::s_pDesktopManagerInstance->GetD2DDevice(),
						reinterpret_cast<ABI::Windows::UI::Composition::ICompositionGraphicsDevice**>(winrt::put_abi(graphicsDevice))
					)
				);

				WCHAR filePath[MAX_PATH + 1]{};
				GetModuleFileName(wil::GetModuleInstanceHandle(), filePath, MAX_PATH);
				PathCchRemoveFileSpec(filePath, MAX_PATH);
				PathCchAppend(filePath, MAX_PATH, L"data\\AeroPeek.png");

				wil::unique_hfile file{ CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0) };
				THROW_LAST_ERROR_IF(!file.is_valid());

				LARGE_INTEGER fileSize{};
				THROW_IF_WIN32_BOOL_FALSE(GetFileSizeEx(file.get(), &fileSize));

				auto buffer{ std::make_unique<BYTE[]>(static_cast<size_t>(fileSize.QuadPart)) };
				THROW_IF_WIN32_BOOL_FALSE(ReadFile(file.get(), buffer.get(), static_cast<DWORD>(fileSize.QuadPart), nullptr, nullptr));
				winrt::com_ptr<IStream> stream{ SHCreateMemStream(buffer.get(), static_cast<UINT>(fileSize.QuadPart)), winrt::take_ownership_from_abi };
				THROW_LAST_ERROR_IF_NULL(stream);

				winrt::com_ptr<IWICImagingFactory2> wicFactory{ nullptr };
				wicFactory.copy_from(DWM::CDesktopManager::s_pDesktopManagerInstance->GetWICFactory());
				winrt::com_ptr<IWICBitmapDecoder> wicDecoder{ nullptr };
				THROW_IF_FAILED(wicFactory->CreateDecoderFromStream(stream.get(), &GUID_VendorMicrosoft, WICDecodeMetadataCacheOnDemand, wicDecoder.put()));
				winrt::com_ptr<IWICBitmapFrameDecode> wicFrame{ nullptr };
				THROW_IF_FAILED(wicDecoder->GetFrame(0, wicFrame.put()));
				winrt::com_ptr<IWICFormatConverter> wicConverter{ nullptr };
				THROW_IF_FAILED(wicFactory->CreateFormatConverter(wicConverter.put()));
				winrt::com_ptr<IWICPalette> wicPalette{ nullptr };
				THROW_IF_FAILED(
					wicConverter->Initialize(
						wicFrame.get(),
						GUID_WICPixelFormat32bppPBGRA,
						WICBitmapDitherTypeNone,
						wicPalette.get(),
						0, WICBitmapPaletteTypeCustom
					)
				);
				winrt::com_ptr<IWICBitmap> wicBitmap{ nullptr };
				THROW_IF_FAILED(wicFactory->CreateBitmapFromSource(wicConverter.get(), WICBitmapCreateCacheOption::WICBitmapNoCache, wicBitmap.put()));

				UINT width{ 0 }, height{ 0 };
				THROW_IF_FAILED(wicBitmap->GetSize(&width, &height));
				drawingSurface = graphicsDevice.CreateDrawingSurface(
					{ static_cast<float>(width), static_cast<float>(height) },
					winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
					winrt::Windows::Graphics::DirectX::DirectXAlphaMode::Premultiplied
				);
				auto drawingSurfaceInterop{ drawingSurface.as<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop>() };
				POINT offset = { 0, 0 };
				winrt::com_ptr<ID2D1DeviceContext> d2dContext{ nullptr };
				THROW_IF_FAILED(
					drawingSurfaceInterop->BeginDraw(nullptr, IID_PPV_ARGS(d2dContext.put()), &offset)
				);
				d2dContext->Clear();
				winrt::com_ptr<ID2D1Bitmap1> d2dBitmap{ nullptr };
				d2dContext->CreateBitmapFromWicBitmap(
					wicBitmap.get(),
					D2D1::BitmapProperties1(
						D2D1_BITMAP_OPTIONS_NONE,
						D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
					),
					d2dBitmap.put()
				);
				d2dContext->DrawBitmap(d2dBitmap.get());
				THROW_IF_FAILED(
					drawingSurfaceInterop->EndDraw()
				);

				return S_OK;
			}

			return S_OK;
		}
		CATCH_RETURN()
	};

	struct CGlassReflectionBackdrop : CDCompBackdropBase
	{
		inline static CGlassReflectionResources s_sharedResources{};

		winrt::com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner> interopDCompDevice{ nullptr };
		winrt::Windows::Foundation::Numerics::float2 fixedOffset{};
		RECT currentWindowRect{};
		RECT currentMonitorRect{};
		HMONITOR currentMonitor{ nullptr };
		winrt::Windows::UI::Composition::CompositionSurfaceBrush glassSurfaceBrush{ nullptr };

		STDMETHOD(Initialize)(
			winrt::Windows::UI::Composition::VisualCollection& visualCollection
		) override try
		{
			THROW_IF_FAILED(CDCompBackdropBase::Initialize(visualCollection));
			THROW_IF_FAILED(s_sharedResources.EnsureGlassSurface());
			THROW_HR_IF_NULL(E_POINTER, s_sharedResources.drawingSurface);
			glassSurfaceBrush = spriteVisual.Compositor().CreateSurfaceBrush(s_sharedResources.drawingSurface);
			glassSurfaceBrush.Stretch(winrt::Windows::UI::Composition::CompositionStretch::None);
			glassSurfaceBrush.HorizontalAlignmentRatio(0.f);
			glassSurfaceBrush.VerticalAlignmentRatio(0.f);
			spriteVisual.Brush(glassSurfaceBrush);
			spriteVisual.Opacity(s_sharedResources.glassIntensity);

			return S_OK;
		}
		CATCH_RETURN()

		HRESULT STDMETHODCALLTYPE Update(DWM::CTopLevelWindow* window) try
		{
			THROW_IF_FAILED(s_sharedResources.EnsureGlassSurface());
			THROW_HR_IF_NULL(E_POINTER, s_sharedResources.drawingSurface);
			if (interopDCompDevice != s_sharedResources.interopDCompDevice)
			{
				interopDCompDevice = s_sharedResources.interopDCompDevice;
				glassSurfaceBrush.Surface(s_sharedResources.drawingSurface);
				spriteVisual.Opacity(s_sharedResources.glassIntensity);
			}

			RECT windowRect{};
			THROW_HR_IF_NULL(E_INVALIDARG, window->GetActualWindowRect(&windowRect, false, true, false));

			HWND hwnd{ window->GetData()->GetHWND() };
			HMONITOR monitor{ MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST) };
			THROW_LAST_ERROR_IF_NULL(monitor);

			RECT monitorRect{};
			MONITORINFO mi{ sizeof(MONITORINFO) };
			THROW_IF_WIN32_BOOL_FALSE(GetMonitorInfoW(monitor, &mi));

			auto surfaceSize{ s_sharedResources.drawingSurface.SizeInt32() };
			if (currentMonitor != monitor || !EqualRect(&currentMonitorRect, &mi.rcMonitor))
			{
				auto scaleFactor
				{
					[&]()
					{
						float factor{1.f};

						auto scaledWidth{ static_cast<float>(surfaceSize.Width) * static_cast<float>(wil::rect_height(mi.rcMonitor)) * (1 + s_sharedResources.parallaxIntensity) / static_cast<float>(surfaceSize.Height) };
						factor = scaledWidth / static_cast<float>(surfaceSize.Width);

						if (scaledWidth < static_cast<float>(wil::rect_width(mi.rcMonitor)) * (1 + s_sharedResources.parallaxIntensity))
						{
							scaledWidth = static_cast<float>(wil::rect_width(mi.rcMonitor)) * (1 + s_sharedResources.parallaxIntensity);
							factor = scaledWidth / static_cast<float>(surfaceSize.Width);
						}

						return factor;
					} ()
				};
				glassSurfaceBrush.Scale(
					winrt::Windows::Foundation::Numerics::float2{ scaleFactor, scaleFactor }
				);
				winrt::Windows::Foundation::Numerics::float2 scaledSize
				{
					static_cast<float>(surfaceSize.Width) * scaleFactor,
					static_cast<float>(surfaceSize.Height) * scaleFactor
				};

				fixedOffset =
				{
					(static_cast<float>(wil::rect_width(mi.rcMonitor)) * (1.f + s_sharedResources.parallaxIntensity) - scaledSize.x) / 2.f,
					(static_cast<float>(wil::rect_height(mi.rcMonitor)) * (1.f + s_sharedResources.parallaxIntensity) - scaledSize.y) / 2.f
				};
				currentMonitor = monitor;
				currentMonitorRect = mi.rcMonitor;
			}
			if ((currentWindowRect.left != windowRect.left) || (currentWindowRect.top != windowRect.top))
			{
				MARGINS margins{};
				window->GetBorderMargins(&margins);
				glassSurfaceBrush.Offset(
					winrt::Windows::Foundation::Numerics::float2
					{
						-static_cast<float>(windowRect.left - mi.rcMonitor.left) * (1.f + s_sharedResources.parallaxIntensity) +
						//static_cast<float>(IsMaximized(hwnd) ? margins.cxLeftWidth : 0.f) +
						fixedOffset.x,
						-static_cast<float>(windowRect.top - mi.rcMonitor.top) * (1.f + s_sharedResources.parallaxIntensity) +
						//static_cast<float>(IsMaximized(hwnd) ? margins.cyTopHeight : 0.f) +
						fixedOffset.y
					}
				);

				currentWindowRect = windowRect;
			}

			return S_OK;
		}
		CATCH_RETURN()
	};
}