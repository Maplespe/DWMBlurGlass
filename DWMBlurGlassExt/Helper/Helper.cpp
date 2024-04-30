/**
 * FileName: Helper.cpp
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
#include "Helper.h"

namespace MDWMBlurGlassExt
{
	HRESULT DrawTextWithGlow(HDC hdc, LPCWSTR pszText, int cchText, LPRECT prc, UINT dwFlags,
		UINT crText, UINT crGlow, UINT nGlowRadius, UINT nGlowIntensity, BOOL bPreMultiply,
		DTT_CALLBACK_PROC actualDrawTextCallback, LPARAM lParam)
	{
		static auto drawTextWithGlow = (decltype(DrawTextWithGlow)*)GetProcAddress(GetModuleHandleW(L"uxtheme.dll"), MAKEINTRESOURCEA(126));

		if (!drawTextWithGlow)
			return E_NOTIMPL;

		return drawTextWithGlow(hdc, pszText, cchText, prc, dwFlags, crText, crGlow,
			nGlowRadius, nGlowIntensity, bPreMultiply, actualDrawTextCallback, lParam);
	}

	HRESULT DrawThemeContent(HDC hdc, const RECT& paintRect, LPCRECT clipRect, LPCRECT excludeRect,
		DWORD additionalFlags, std::function<void(HDC, HPAINTBUFFER, RGBQUAD*, int)> callback,
		std::byte alpha, bool useBlt, bool update) try
	{
		BOOL updateTarget{ FALSE };
		HDC memoryDC{ nullptr };
		HPAINTBUFFER bufferedPaint{ nullptr };
		BLENDFUNCTION blendFunction{ AC_SRC_OVER, 0, std::to_integer<BYTE>(alpha), AC_SRC_ALPHA };
		BP_PAINTPARAMS paintParams{ sizeof(BP_PAINTPARAMS), BPPF_ERASE | additionalFlags, excludeRect, !useBlt ? &blendFunction : nullptr };

		winrt::check_bool(callback);

		int saved = SaveDC(hdc);

		auto cleanUp = MDWMBlurGlass::RAIIHelper::scope_exit([&]
		{
			if (bufferedPaint != nullptr)
			{
					EndBufferedPaint(bufferedPaint, updateTarget);
					bufferedPaint = nullptr;
			}
			RestoreDC(hdc, saved);
		});

		if (clipRect)
		{
			IntersectClipRect(hdc, clipRect->left, clipRect->top, clipRect->right, clipRect->bottom);
		}
		if (excludeRect)
		{
			ExcludeClipRect(hdc, excludeRect->left, excludeRect->top, excludeRect->right, excludeRect->bottom);
		}

		bufferedPaint = BeginBufferedPaint(hdc, &paintRect, BPBF_TOPDOWNDIB, &paintParams, &memoryDC);
		winrt::check_bool(bufferedPaint);
		{
			auto selectedFont{ SelectObject(memoryDC, GetCurrentObject(hdc, OBJ_FONT)) };
			auto selectedBrush{ SelectObject(memoryDC, GetCurrentObject(hdc, OBJ_BRUSH)) };
			auto selectedPen{ SelectObject(memoryDC, GetCurrentObject(hdc, OBJ_PEN)) };

			auto restore = MDWMBlurGlass::RAIIHelper::scope_exit([&]
			{
				SelectObject(memoryDC, selectedFont);
				SelectObject(memoryDC, selectedBrush);
				SelectObject(memoryDC, selectedPen);
			});
			int cxRow{ 0 };
			RGBQUAD* buffer{ nullptr };
			winrt::check_hresult(GetBufferedPaintBits(bufferedPaint, &buffer, &cxRow));

			callback(memoryDC, bufferedPaint, buffer, cxRow);
		}

		updateTarget = TRUE;

		return S_OK;
	}
	catch (...)
	{
		return winrt::to_hresult();
	}

	HRESULT DrawTextWithAlpha(HDC hdc, LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format, int& result) try
	{
		auto drawTextCallback = [](HDC hdc, LPWSTR pszText, int cchText, LPRECT prc, UINT dwFlags, LPARAM lParam) -> int
		{
			return *reinterpret_cast<int*>(lParam) = DrawTextW(hdc, pszText, cchText, prc, dwFlags);
		};

		auto callback = [&](HDC memoryDC, HPAINTBUFFER, RGBQUAD*, int)
		{
			winrt::check_hresult
			(
				DrawTextWithGlow(
					memoryDC,
					lpchText,
					cchText,
					lprc,
					format,
					GetTextColor(hdc),
					0,
					0,
					0,
					TRUE,
					drawTextCallback,
					(LPARAM)&result
				)
			);
		};

		auto callbackAsFallback = [&](HDC memoryDC, HPAINTBUFFER, RGBQUAD*, int)
		{
			DTTOPTS Options =
			{
				sizeof(DTTOPTS),
				DTT_TEXTCOLOR | DTT_COMPOSITED | DTT_CALLBACK | DTT_SHADOWCOLOR | DTT_GLOWSIZE,
				GetTextColor(hdc),
				0,
				0,
				0,
				{},
				0,
				0,
				0,
				0,
				FALSE,
				15,
				drawTextCallback,
				(LPARAM)&result
			};
			HTHEME hTheme = OpenThemeData(nullptr, L"Composited::Window");

			auto clean = MDWMBlurGlass::RAIIHelper::scope_exit([&] { CloseThemeData(hTheme); });

			winrt::check_pointer(hTheme);
			RECT offset = *lprc;
			offset.left += 20;
			offset.top += 10;
			winrt::check_hresult(DrawThemeTextEx(hTheme, memoryDC, 0, 0, lpchText, cchText, format, &offset, &Options));
		};

		winrt::check_pointer(hdc);

		/*if (FAILED(DrawThemeContent(hdc, *lprc, nullptr, nullptr, 0, callback)))
		{
			winrt::check_hresult(DrawThemeContent(hdc, *lprc, nullptr, nullptr, 0, callbackAsFallback));
		}*/
		winrt::check_hresult(DrawThemeContent(hdc, *lprc, nullptr, nullptr, 0, callbackAsFallback));

		GdiFlush();

		return S_OK;
	}
	catch (...)
	{
		return winrt::to_hresult();
	}

	winrt::com_ptr<IWICBitmap> CreateWICBitmap(IWICImagingFactory2* factory, std::wstring_view filename)
	{
		using namespace winrt;

		if (!factory)
			return nullptr;

		com_ptr<IWICBitmapDecoder> decoder = nullptr;

		HRESULT hr = factory->CreateDecoderFromFilename(filename.data(), &GUID_VendorMicrosoft, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.put());

		if (FAILED(hr))
			return nullptr;

		com_ptr<IWICBitmapFrameDecode> frame = nullptr;
		hr = decoder->GetFrame(0, frame.put());
		if (FAILED(hr))
			return nullptr;

		com_ptr<IWICFormatConverter> converter = nullptr;
		hr = factory->CreateFormatConverter(converter.put());
		if (FAILED(hr))
			return nullptr;

		com_ptr<IWICPalette> palette = nullptr;
		hr = converter->Initialize(frame.get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, palette.get(), 0, WICBitmapPaletteTypeCustom);

		if (FAILED(hr))
			return nullptr;

		com_ptr<IWICBitmap> wicbitmap = nullptr;
		factory->CreateBitmapFromSource(converter.get(), WICBitmapNoCache, wicbitmap.put());

		return wicbitmap;
	}

	winrt::com_ptr<ID2D1Bitmap1> CreateD2DBitmap(IWICImagingFactory2* factory, ID2D1DeviceContext* context,
	                                             std::wstring_view filename)
	{
		using namespace winrt;

		com_ptr<IWICBitmap> wicbitmap = CreateWICBitmap(factory, filename);

		if (!wicbitmap)
			return nullptr;

		com_ptr<ID2D1Bitmap1> ret = nullptr;
		D2D1_PIXEL_FORMAT format;
		format.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
		format.format = DXGI_FORMAT_B8G8R8A8_UNORM;

		D2D1_BITMAP_PROPERTIES1 bitmapProp;
		bitmapProp.dpiX = 96;
		bitmapProp.dpiY = 96;
		bitmapProp.colorContext = nullptr;
		bitmapProp.pixelFormat = format;
		bitmapProp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;

		context->CreateBitmapFromWicBitmap(wicbitmap.get(), bitmapProp, ret.put());

		return ret;
	}

	HBITMAP CreateAlphaBitmap(int width, int height)
	{
		BITMAPINFOHEADER bmih = { 0 };
		bmih.biSize = sizeof(BITMAPINFOHEADER);
		bmih.biWidth = width;
		bmih.biHeight = -height;
		bmih.biPlanes = 1;
		bmih.biBitCount = 32;
		bmih.biCompression = BI_RGB;

		BITMAPINFO bmi = { 0 };
		bmi.bmiHeader = bmih;

		HDC hdc = GetDC(nullptr);
		void* bits;
		HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
		ReleaseDC(nullptr, hdc);
		return hBitmap;
	}

	void EnumMonitors(std::vector<RECT>& monitorRects)
	{
		auto MonitorEnumProc = [](HMONITOR, HDC, LPRECT lprcMonitor, LPARAM dwData)
		{
			auto pList = (std::vector<RECT>*)dwData;
			pList->push_back(*lprcMonitor);
			return TRUE;
		};
		monitorRects.clear();
		EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&monitorRects);
	}

	bool IsRectInside(const RECT& rect1, const RECT& rect2)
	{
		return rect1.left <= rect2.left && rect1.top <= rect2.top
			&& rect1.right >= rect2.left && rect1.bottom >= rect2.top;
	}

	bool GetDesktopID(ULONG_PTR type, ULONG_PTR* desktopID)
	{
		if (static const auto pfnGetDesktopID
		{
			reinterpret_cast<decltype(&GetDesktopID)>(GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDesktopID"))
		})
		{
			return pfnGetDesktopID(type, desktopID);
		}
		return false;
	}
}
