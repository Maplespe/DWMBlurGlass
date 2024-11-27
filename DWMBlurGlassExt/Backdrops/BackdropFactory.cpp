#include "BackdropFactory.hpp"
#include "DcompBackdrop.hpp"
#include <Shlwapi.h>

#include "AeroBackdrop.hpp"
#include "BlurBackdrop.hpp"
#include "AcrylicBackdrop.hpp"
#include "MicaBackdrop.hpp"

namespace MDWMBlurGlassExt
{
	effectType g_type{ effectType::Blur };

	wuc::CompositionDrawingSurface g_materialTextureSurface{ nullptr };
	wuc::CompositionSurfaceBrush g_materialTextureBrush{ nullptr };

	std::chrono::steady_clock::time_point g_currentTimeStamp{};
	std::unordered_map<DWORD, wuc::CompositionBrush> g_backdropActiveBrushMap{};
	std::unordered_map<DWORD, wuc::CompositionBrush> g_backdropInactiveBrushMap{};

	wuc::CompositionSurfaceBrush CreateMaterialTextureBrush()
	{
		com_ptr<IStream> stream{ nullptr };
		com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner> dcompDevice{ nullptr };
		winrt::copy_from_abi(dcompDevice, DWM::CDesktopManager::s_pDesktopManagerInstance->GetDCompositionInteropDevice());
		auto compositor{ dcompDevice.as<wuc::Compositor>() };

		wil::unique_hmodule wuxcModule{ LoadLibraryExW(L"Windows.UI.Xaml.Controls.dll", nullptr,
				LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE) };
		THROW_LAST_ERROR_IF_NULL(wuxcModule);
		auto resourceHandle{ FindResourceW(wuxcModule.get(), MAKEINTRESOURCE(2000), RT_RCDATA) };
		THROW_LAST_ERROR_IF_NULL(resourceHandle);
		auto globalHandle{ LoadResource(wuxcModule.get(), resourceHandle) };
		THROW_LAST_ERROR_IF_NULL(globalHandle);
		auto cleanUp = wil::scope_exit([&]
		{
			if (globalHandle)
			{
				UnlockResource(globalHandle);
				FreeResource(globalHandle);
			}
		});
		DWORD resourceSize{ SizeofResource(wuxcModule.get(), resourceHandle) };
		THROW_LAST_ERROR_IF(resourceSize == 0);
		auto resourceAddress{ reinterpret_cast<PBYTE>(LockResource(globalHandle)) };
		stream = { SHCreateMemStream(resourceAddress, resourceSize), winrt::take_ownership_from_abi };
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
		com_ptr<IWICBitmap> wicBitmap{ nullptr };
		THROW_IF_FAILED(wicFactory->CreateBitmapFromSource(wicConverter.get(), WICBitmapCreateCacheOption::WICBitmapNoCache, wicBitmap.put()));

		UINT width{ 0 }, height{ 0 };
		THROW_IF_FAILED(wicBitmap->GetSize(&width, &height));

		if (!g_materialTextureSurface)
		{
			wuc::CompositionGraphicsDevice graphicsDevice{ nullptr };
			THROW_IF_FAILED(
				compositor.as<ABI::Windows::UI::Composition::ICompositorInterop>()->CreateGraphicsDevice(
					DWM::CDesktopManager::s_pDesktopManagerInstance->GetD2DDevice(),
					reinterpret_cast<ABI::Windows::UI::Composition::ICompositionGraphicsDevice**>(winrt::put_abi(graphicsDevice))
				)
			);
			g_materialTextureSurface = graphicsDevice.CreateDrawingSurface(
				{ static_cast<float>(width), static_cast<float>(height) },
				wgd::DirectXPixelFormat::R16G16B16A16Float,
				wgd::DirectXAlphaMode::Premultiplied
			);
		}
		else
		{
			g_materialTextureSurface.Resize(
				{
					static_cast<int>(width),
					static_cast<int>(height)
				}
			);
		}

		auto drawingSurfaceInterop{ g_materialTextureSurface.as<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop>() };
		POINT offset{ 0, 0 };
		winrt::com_ptr<ID2D1DeviceContext> d2dContext{ nullptr };
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

		return compositor.CreateSurfaceBrush(g_materialTextureSurface);
	}

	wuc::CompositionBrush BackdropFactory::GetOrCreateBackdropBrush(
		const wuc::Compositor& compositor,
		DWORD color,
		bool active,
		DWM::ACCENT_POLICY* policy
	)
	{
		// these backdrops has no difference other than color
		if (g_type != effectType::Aero)
		{
			active = false;
		}

		auto& map{ active ? g_backdropActiveBrushMap : g_backdropInactiveBrushMap };
		auto it{ map.find(color) };
		// find cached brush
		if (it != map.end())
		{
			if (it->second.Compositor() == compositor)
			{
				return it->second;
			}
			else
			{
				Shutdown();
			}
		}

		wuc::CompositionBrush brush{ nullptr };
		if (policy)
		{
			color = policy->nColor | 0xFF000000;
			if (policy->nAccentState == 3 && !(policy->nFlags & 2) && os::buildNumber < 22621)
			{
				color = 0;
			}

			if (
				policy->nAccentState == 4 &&
				policy->nColor == 0
				)
			{
				brush = compositor.CreateColorBrush({});
			}
			if (policy->nAccentState == 2)
			{
				brush = compositor.CreateColorBrush(MakeWinrtColor(policy->nColor));
			}
			if (policy->nAccentState == 1)
			{
				brush = compositor.CreateColorBrush(MakeWinrtColor(color));
			}

			if (brush)
			{
				return brush;
			}
		}

		auto winrtColor{ MakeWinrtColor(color) };
		auto glassOpacity{ policy ? static_cast<float>(policy->nColor >> 24 & 0xFF) / 255.f : 1.f };
		switch (g_type)
		{
		case effectType::Blur:
		{
			brush = BlurBackdrop::CreateBrush(
				compositor,
				winrtColor,
				glassOpacity,
				g_configData.customBlurAmount
			);
			break;
		}
		case effectType::Aero:
		{
			brush = AeroBackdrop::CreateBrush(
				compositor,
				winrtColor,
				winrtColor,
				policy ? static_cast<float>(policy->nColor >> 24 & 0xFF) / 255.f : 
				(active ? g_configData.aeroColorBalance : g_configData.aeroColorBalance * 0.4),
				g_configData.aeroAfterglowBalance,
				active ? g_configData.aeroBlurBalance : 0.4f * g_configData.aeroBlurBalance + 0.6f,
				g_configData.customBlurAmount
			);
			break;
		}
		case effectType::Acrylic:
		{
			auto useLuminosity
			{
				policy &&
				os::buildNumber >= 22000 &&
				(policy->nFlags & 2) != 0 &&
				(
					(policy->nAccentState == 3 && os::buildNumber > 22000) ||
					(policy->nAccentState == 4)
				)
			};
			auto luminosity{ policy ? (useLuminosity ? std::optional{ 1.03f } : std::nullopt) : g_configData.luminosityOpacity };
			brush = AcrylicBackdrop::CreateBrush(
				compositor,
				g_materialTextureBrush,
				AcrylicBackdrop::GetEffectiveTintColor(winrtColor, glassOpacity, luminosity),
				AcrylicBackdrop::GetEffectiveLuminosityColor(winrtColor, glassOpacity, luminosity),
				g_configData.customBlurAmount,
				1.f
			);
			break;
		}
		case effectType::Mica:
		{
			brush = MicaBackdrop::CreateBrush(
				compositor,
				winrtColor,
				winrtColor,
				glassOpacity,
				g_configData.luminosityOpacity
			);
			break;
		}
		default:
			brush = compositor.CreateColorBrush(MakeWinrtColor(policy ? policy->nColor : color));
			break;
		}

		if (!policy)
		{
			map.insert_or_assign(color, brush);
		}

		return brush;
	}

	std::chrono::steady_clock::time_point BackdropFactory::GetBackdropBrushTimeStamp()
	{
		return g_currentTimeStamp;
	}

	void BackdropFactory::Shutdown()
	{
		g_backdropActiveBrushMap.clear();
		g_backdropInactiveBrushMap.clear();
		g_materialTextureBrush = nullptr;
		g_materialTextureSurface = nullptr;
	}

	void BackdropFactory::RefreshConfig()
	{
		if (!g_materialTextureBrush)
		{
			g_materialTextureBrush = CreateMaterialTextureBrush();
		}

		g_type = g_configData.effectType;
		// mica is not available in windows 10
		if (os::buildNumber < 22000 && g_type == effectType::Mica)
		{
			g_type = effectType::Blur;
		}

		g_backdropActiveBrushMap.clear();
		g_backdropInactiveBrushMap.clear();
		g_currentTimeStamp = std::chrono::steady_clock::now();
	}
}
