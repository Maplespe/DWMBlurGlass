#pragma once
#include "DCompBackdrop.hpp"
#include <Shlwapi.h>

namespace MDWMBlurGlassExt
{
	class CButtonGlowBackdrop : public implements<CButtonGlowBackdrop, IUnknown>, COverlayVisual
	{
		static inline wuc::CompositionDrawingSurface s_glowCloseTexture{ nullptr };
		static inline wuc::CompositionDrawingSurface s_glowOtherTexture{ nullptr };
		static inline com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner> s_dcompDevice{ nullptr };
		static inline wg::SizeInt32 s_glowCloseSize{};
		static inline wg::SizeInt32 s_glowOtherSize{};

		wuc::CompositionSurfaceBrush m_textureSurfaceBrush[2]{ { nullptr}, { nullptr } };
		wuc::CompositionNineGridBrush m_textureNineGridBrush[2]{ { nullptr}, { nullptr } };
		wuc::CompositionColorBrush m_opacityBrush{ nullptr };
		wuc::SpriteVisual m_spriteVisual[3]{ {nullptr}, {nullptr},{nullptr} };

		bool m_visible[3] = { false };
	public:
		CButtonGlowBackdrop(DWM::CVisual* visual) : COverlayVisual(visual)
		{
			CButtonGlowBackdrop::InitializeVisual();
		}
		~CButtonGlowBackdrop() override
		{
			CButtonGlowBackdrop::UninitializeVisual();
		}

		HRESULT InitializeVisual() override
		{
			InitializeInteropDevice(DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice());
			RETURN_IF_FAILED(COverlayVisual::InitializeVisual());
			EnsureTextureSurface();

			auto compositor{ m_dcompDevice.as<wuc::Compositor>() };
			m_spriteVisual[0] = compositor.CreateSpriteVisual();
			m_spriteVisual[1] = compositor.CreateSpriteVisual();
			m_spriteVisual[2] = compositor.CreateSpriteVisual();

			m_visualCollection.InsertAtTop(m_spriteVisual[2]);
			m_visualCollection.InsertAtTop(m_spriteVisual[1]);
			m_visualCollection.InsertAtTop(m_spriteVisual[0]);

			m_textureNineGridBrush[0] = compositor.CreateNineGridBrush();
			m_textureNineGridBrush[1] = compositor.CreateNineGridBrush();

			m_textureSurfaceBrush[0] = compositor.CreateSurfaceBrush(s_glowCloseTexture);
			m_textureSurfaceBrush[0].Stretch(wuc::CompositionStretch::Fill);

			m_textureSurfaceBrush[1] = compositor.CreateSurfaceBrush(s_glowOtherTexture);
			m_textureSurfaceBrush[1].Stretch(wuc::CompositionStretch::Fill);

			m_textureNineGridBrush[0].Source(m_textureSurfaceBrush[0]);
			m_textureNineGridBrush[0].SetInsets(15.f, 10.f, 15.f, 10.f);
			m_textureNineGridBrush[1].Source(m_textureSurfaceBrush[1]);
			m_textureNineGridBrush[1].SetInsets(15.f, 10.f, 15.f, 10.f);

			m_opacityBrush = compositor.CreateColorBrush();
			m_spriteVisual[0].Brush(m_opacityBrush);
			m_spriteVisual[1].Brush(m_opacityBrush);
			m_spriteVisual[2].Brush(m_opacityBrush);

			return S_OK;
		}

		void UninitializeVisual() override
		{
			m_opacityBrush = nullptr;
			m_spriteVisual[0] = nullptr;
			m_spriteVisual[1] = nullptr;
			m_spriteVisual[2] = nullptr;

			COverlayVisual::UninitializeVisual();
		}

		void Update(int index, POINT offset, SIZE size, float scale, bool show) try
		{
			EnsureTextureSurface();

			if (!s_glowCloseTexture || !s_glowOtherTexture)
				return;

			m_textureSurfaceBrush[0].Surface(s_glowCloseTexture);
			m_textureSurfaceBrush[1].Surface(s_glowOtherTexture);

			index--;

			//Question button
			if (index == -1)
				index = 0;

			if (index == 2)
			{
				m_spriteVisual[index].Offset(
					winrt::Windows::Foundation::Numerics::float3{ (float)offset.x - 9.5f, (float)offset.y - 7.f, 0.f }
				);
				m_spriteVisual[index].Size(winrt::Windows::Foundation::Numerics::float2
				{
					(float)size.cx + 19.5f + scale, (float)size.cy + 16.f + round(scale)
				});

			}
			else if (index == 1 || index == 0)
			{
				m_spriteVisual[index].Offset(
					winrt::Windows::Foundation::Numerics::float3{ (float)offset.x - 9.5f, (float)offset.y - 7.f, 0.f }
				);
				m_spriteVisual[index].Size(winrt::Windows::Foundation::Numerics::float2
				{
					(float)size.cx + 19.5f + scale, (float)size.cy + 16.f + round(scale)
				});
			}
			else
				return;

			auto newBrush = show ? m_textureNineGridBrush[index != 2].as<wuc::CompositionBrush>() : m_opacityBrush.as<wuc::CompositionBrush>();
			auto currentBrush = m_visible[index] ? m_textureNineGridBrush[index != 2].as<wuc::CompositionBrush>() : m_opacityBrush.as<wuc::CompositionBrush>();

			if (m_visible[index] != show)
			{
				auto compositor{ m_dcompDevice.as<wuc::Compositor>() };

				auto crossfadeBrush{ CreateCrossFadeBrush(compositor, currentBrush, newBrush) };

				m_visible[index] = show;
				m_spriteVisual[index].Brush(crossfadeBrush);

				crossfadeBrush.StartAnimation(
					L"Crossfade.Weight",
					CreateCrossFadeAnimation(
						compositor,
						0,
						std::chrono::milliseconds(60)
					)
				);
			}
		}
		CATCH_LOG_RETURN()
		
		static comp::CompositionDrawingSurface LoadTexture(std::wstring_view path) try
		{
			WCHAR filePath[MAX_PATH + 1]{};
			GetModuleFileName(wil::GetModuleInstanceHandle(), filePath, MAX_PATH);
			PathCchRemoveFileSpec(filePath, MAX_PATH);
			PathCchAppend(filePath, MAX_PATH, path.data());
		
			wil::unique_hfile file{ CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0) };
			THROW_LAST_ERROR_IF(!file.is_valid());
		
			LARGE_INTEGER fileSize{};
			THROW_IF_WIN32_BOOL_FALSE(GetFileSizeEx(file.get(), &fileSize));
		
			auto buffer{ std::make_unique<BYTE[]>(static_cast<size_t>(fileSize.QuadPart)) };
			THROW_IF_WIN32_BOOL_FALSE(ReadFile(file.get(), buffer.get(), static_cast<DWORD>(fileSize.QuadPart), nullptr, nullptr));
			com_ptr<IStream> stream{ SHCreateMemStream(buffer.get(), static_cast<UINT>(fileSize.QuadPart)), winrt::take_ownership_from_abi };
			THROW_LAST_ERROR_IF_NULL(stream);
		
			com_ptr<IWICImagingFactory2> wicFactory{ nullptr };
			wicFactory.copy_from(DWM::CDesktopManager::s_pDesktopManagerInstance->GetWICFactory());
			com_ptr<IWICBitmapDecoder> wicDecoder{ nullptr };
			THROW_IF_FAILED(wicFactory->CreateDecoderFromStream(stream.get(), &GUID_VendorMicrosoft, WICDecodeMetadataCacheOnDemand, wicDecoder.put()));
			com_ptr<IWICBitmapFrameDecode> wicFrame{ nullptr };
			THROW_IF_FAILED(wicDecoder->GetFrame(0, wicFrame.put()));
			com_ptr<IWICFormatConverter> wicConverter{ nullptr };
			THROW_IF_FAILED(wicFactory->CreateFormatConverter(wicConverter.put()));
			com_ptr<IWICPalette> wicPalette{ nullptr };
			THROW_IF_FAILED(
				wicConverter->Initialize(
					wicFrame.get(),
					GUID_WICPixelFormat32bppPBGRA,
					WICBitmapDitherTypeNone,
					wicPalette.get(),
					0, WICBitmapPaletteTypeCustom
				)
			);
			com_ptr<IWICBitmap> wicBitmap{ nullptr };
			THROW_IF_FAILED(wicFactory->CreateBitmapFromSource(wicConverter.get(), WICBitmapCreateCacheOption::WICBitmapNoCache, wicBitmap.put()));
		
			auto compositor{ s_dcompDevice.as<wuc::Compositor>() };
			winrt::Windows::UI::Composition::CompositionGraphicsDevice graphicsDevice{ nullptr };
			THROW_IF_FAILED(
				compositor.as<ABI::Windows::UI::Composition::ICompositorInterop>()->CreateGraphicsDevice(
					DWM::CDesktopManager::s_pDesktopManagerInstance->GetD2DDevice(),
					reinterpret_cast<ABI::Windows::UI::Composition::ICompositionGraphicsDevice**>(winrt::put_abi(graphicsDevice))
				)
			);
		
			UINT width{ 0 }, height{ 0 };
			THROW_IF_FAILED(wicBitmap->GetSize(&width, &height));
			auto drawingSurface = graphicsDevice.CreateDrawingSurface(
				{ static_cast<float>(width), static_cast<float>(height) },
				winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
				winrt::Windows::Graphics::DirectX::DirectXAlphaMode::Premultiplied
			);
			auto drawingSurfaceInterop{ drawingSurface.as<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop>() };
			POINT offset = { 0, 0 };
			com_ptr<ID2D1DeviceContext> d2dContext{ nullptr };
			THROW_IF_FAILED(
				drawingSurfaceInterop->BeginDraw(nullptr, IID_PPV_ARGS(d2dContext.put()), &offset)
			);
			d2dContext->Clear();
			com_ptr<ID2D1Bitmap1> d2dBitmap{ nullptr };
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
		
			return drawingSurface;
		}
		CATCH_LOG_RETURN_HR(nullptr)

		static void EnsureTextureSurface()
		{
			if (!s_dcompDevice)
			{
				s_dcompDevice.copy_from(DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice());
			}
			if (!CheckDeviceState(s_dcompDevice))
			{
				s_glowCloseTexture = nullptr;
				s_glowOtherTexture = nullptr;
			}
			if(!s_glowCloseTexture)
				s_glowCloseTexture = LoadTexture(Utils::GetCurrentDir() + L"\\data\\btnglow_close.png");
			if(!s_glowOtherTexture)
				s_glowOtherTexture = LoadTexture(Utils::GetCurrentDir() + L"\\data\\btnglow_other.png");
		}
		
		static void Shutdown()
		{
			s_glowCloseTexture = nullptr;
			s_glowOtherTexture = nullptr;
			s_dcompDevice = nullptr;
		}
	};
}
