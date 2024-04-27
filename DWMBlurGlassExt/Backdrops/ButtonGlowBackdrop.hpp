#pragma once
#include <ranges>
#include "../Section/CompositedBackdrop.hpp"
#include <Shlwapi.h>

namespace MDWMBlurGlassExt
{
	namespace comp = winrt::Windows::UI::Composition;

	struct CButtonGlowResource : CDCompResourcesBase
	{
		comp::CompositionDrawingSurface closebtnimg{ nullptr };
		comp::CompositionDrawingSurface otherbtnimg{ nullptr };

		void ReloadParameters() override
		{

		}

		comp::CompositionDrawingSurface LoadTexture(std::wstring_view path) try
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

			auto compositor{ interopDCompDevice.as<winrt::Windows::UI::Composition::Compositor>() };
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

			return drawingSurface;
		}
		catch (...)
		{
			return { nullptr };
		}

		HRESULT STDMETHODCALLTYPE EnsureTextureSurface() try
		{
			if (auto is_device_valid = [&]
				{
					if (!interopDCompDevice) return false;

					BOOL valid{ FALSE };
					THROW_IF_FAILED(
						interopDCompDevice.as<IDCompositionDevice>()->CheckDeviceState(
							&valid
						)
					);

					return valid == TRUE;
				}; !is_device_valid())
			{
				interopDCompDevice.copy_from(
					DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice()
				);
				ReloadParameters();

				closebtnimg = LoadTexture(L"data\\btnglow_close.png");
				otherbtnimg = LoadTexture(L"data\\btnglow_other.png");

				return S_OK;
			}

			return S_OK;
		}
		CATCH_RETURN()
	};

	struct CButtonGlowBackdrop : CSpriteVisualBase<false>
	{
		inline static CButtonGlowResource s_sharedResources{};

		winrt::com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner> interopDCompDevice{ nullptr };
		comp::CompositionSurfaceBrush textureSurfaceBrush[2]{ { nullptr}, { nullptr } };
		comp::CompositionNineGridBrush textureNineGridBrush[2]{ { nullptr}, { nullptr } };
		comp::CompositionColorBrush opacityBrush{ nullptr };
		comp::SpriteVisual spriteVisual[3]{ {nullptr}, {nullptr},{nullptr} };
		bool visible[3] = { false };


		CButtonGlowBackdrop(DWM::CVisual* visual) : CSpriteVisualBase(visual)
		{
			CButtonGlowBackdrop::InitializeVisual();
		}

		STDMETHOD(InitializeVisual)() override try
		{
			THROW_IF_FAILED(CSpriteVisualBase::InitializeVisual());
			THROW_IF_FAILED(s_sharedResources.EnsureTextureSurface());
			THROW_HR_IF_NULL(E_POINTER, s_sharedResources.closebtnimg);
			THROW_HR_IF_NULL(E_POINTER, s_sharedResources.otherbtnimg);

			auto compositor{ m_visualCollection.Compositor() };
			spriteVisual[0] = compositor.CreateSpriteVisual();
			spriteVisual[1] = compositor.CreateSpriteVisual();
			spriteVisual[2] = compositor.CreateSpriteVisual();

			m_visualCollection.InsertAtTop(spriteVisual[2]);
			m_visualCollection.InsertAtTop(spriteVisual[1]);
			m_visualCollection.InsertAtTop(spriteVisual[0]);

			//spriteVisual.Size(s_sharedResources.drawingSurface.Size());

			textureNineGridBrush[0] = compositor.CreateNineGridBrush();
			textureNineGridBrush[1] = compositor.CreateNineGridBrush();

			textureSurfaceBrush[0] = compositor.CreateSurfaceBrush(s_sharedResources.closebtnimg);
			textureSurfaceBrush[0].Stretch(winrt::Windows::UI::Composition::CompositionStretch::Fill);

			textureSurfaceBrush[1] = compositor.CreateSurfaceBrush(s_sharedResources.otherbtnimg);
			textureSurfaceBrush[1].Stretch(winrt::Windows::UI::Composition::CompositionStretch::Fill);
			//textureSurfaceBrush[1].HorizontalAlignmentRatio(0.f);
			//textureSurfaceBrush[1].VerticalAlignmentRatio(0.f);

			textureNineGridBrush[0].Source(textureSurfaceBrush[0]);
			textureNineGridBrush[0].SetInsets(15.f, 10.f, 15.f, 10.f);
			textureNineGridBrush[1].Source(textureSurfaceBrush[1]);
			textureNineGridBrush[1].SetInsets(15.f, 10.f, 15.f, 10.f);

			opacityBrush = compositor.CreateColorBrush();
			spriteVisual[0].Brush(opacityBrush);
			spriteVisual[1].Brush(opacityBrush);
			spriteVisual[2].Brush(opacityBrush);

			return S_OK;
		}
		CATCH_RETURN()

		HRESULT STDMETHODCALLTYPE Update(int index, POINT offset, SIZE size, float scale, bool show) try
		{
			THROW_IF_FAILED(s_sharedResources.EnsureTextureSurface());
			THROW_HR_IF_NULL(E_POINTER, s_sharedResources.closebtnimg);
			THROW_HR_IF_NULL(E_POINTER, s_sharedResources.otherbtnimg);
			if (interopDCompDevice != s_sharedResources.interopDCompDevice)
			{
				interopDCompDevice = s_sharedResources.interopDCompDevice;
				textureSurfaceBrush[0].Surface(s_sharedResources.closebtnimg);
				textureSurfaceBrush[1].Surface(s_sharedResources.otherbtnimg);
			}

			index--;

			if (index == 2)
			{
				spriteVisual[index].Offset(
					winrt::Windows::Foundation::Numerics::float3{ (float)offset.x - 10.f, (float)offset.y - 7.f, 0.f }
				);
				spriteVisual[index].Size(winrt::Windows::Foundation::Numerics::float2
					{
						(float)size.cx + 20.f + scale, (float)size.cy + 16.f + round(scale)
					});

			}
			else if (index == 1 || index == 0)
			{
				spriteVisual[index].Offset(
					winrt::Windows::Foundation::Numerics::float3{ (float)offset.x - 10.f, (float)offset.y - 7.f, 0.f }
				);
				spriteVisual[index].Size(winrt::Windows::Foundation::Numerics::float2
				{
					(float)size.cx + 20.f + scale, (float)size.cy + 16.f + round(scale)
				});
			}
			else
				return S_OK;

			auto newBrush = show ? textureNineGridBrush[index != 2].as<comp::CompositionBrush>() : opacityBrush.as<comp::CompositionBrush>();
			auto currentBrush = visible[index] ? textureNineGridBrush[index != 2].as<comp::CompositionBrush>() : opacityBrush.as<comp::CompositionBrush>();

			if (visible[index] != show)
			{
				auto compositor{ m_visualCollection.Compositor() };

				const auto crossfadeBrush
				{
					CDCompResources::CreateCrossFadeEffectBrush(
						compositor,
						currentBrush,
						newBrush
					)
				};

				visible[index] = show;
				spriteVisual[index].Brush(crossfadeBrush);

				crossfadeBrush.StartAnimation(
					L"Crossfade.Weight",
					CDCompResources::CreateCrossFadeAnimation(
						compositor,
						std::chrono::milliseconds(60)
					)
				);
			}

			return S_OK;
		}
		CATCH_RETURN()
	};

	class CButtonGlowManager
	{
	public:
		winrt::com_ptr<CButtonGlowBackdrop> GetOrCreateBackdrop(DWM::CTopLevelWindow* target, bool createIfNecessary = false)
		{
			auto it{ m_backdropMap.find(target) };

			if (createIfNecessary)
			{
				if (it == m_backdropMap.end())
				{
					auto visual = target->GetVisual();
					auto result{ m_backdropMap.emplace(target, winrt::make_self<CButtonGlowBackdrop>(visual)) };
					if (result.second == true)
					{
						it = result.first;
					}
				}
			}

			return it == m_backdropMap.end() ? nullptr : it->second;
		}

		winrt::com_ptr<CButtonGlowBackdrop> TryCloneBackdropForWindow(DWM::CTopLevelWindow* src, DWM::CTopLevelWindow* dest)
		{
			/*if (auto backdrop{ GetOrCreateBackdrop(src) }; backdrop)
			{
				auto it{ m_backdropMap.find(dest) };
				auto clonedBackdrop{ winrt::make_self<CButtonGlowBackdrop>(backdrop.get(), dest) };

				if (it == m_backdropMap.end())
				{
					auto result{ m_backdropMap.emplace(dest, clonedBackdrop) };
					if (result.second == true) { it = result.first; }
				}
				else
				{
					std::swap(
						clonedBackdrop,
						it->second
					);
				}

				return it == m_backdropMap.end() ? nullptr : it->second;
			}*/

			return nullptr;
		}

		void Remove(DWM::CTopLevelWindow* target)
		{
			auto it{ m_backdropMap.find(target) };

			if (it != m_backdropMap.end())
			{
				m_backdropMap.erase(it);
			}
		}

		void Shutdown()
		{
			m_backdropMap.clear();
		}

		void RefreshEffectConfig()
		{
			CButtonGlowBackdrop::s_sharedResources.interopDCompDevice = nullptr;

			for (auto& effect : m_backdropMap | std::views::values)
			{
				effect->interopDCompDevice = nullptr;
			}
		}
	private:
		std::unordered_map<DWM::CTopLevelWindow*, winrt::com_ptr<CButtonGlowBackdrop>> m_backdropMap{};
	};
}