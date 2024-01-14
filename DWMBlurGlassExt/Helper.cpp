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
#include "Common.h"

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
				DTT_TEXTCOLOR | DTT_COMPOSITED | DTT_CALLBACK,
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
				0,
				drawTextCallback,
				(LPARAM)&result
			};
			HTHEME hTheme = OpenThemeData(nullptr, L"Composited::Window");

			auto clean = MDWMBlurGlass::RAIIHelper::scope_exit([&] { CloseThemeData(hTheme); });

			winrt::check_pointer(hTheme);
			winrt::check_hresult(DrawThemeTextEx(hTheme, memoryDC, 0, 0, lpchText, cchText, format, lprc, &Options));
		};

		winrt::check_pointer(hdc);

		if (FAILED(DrawThemeContent(hdc, *lprc, nullptr, nullptr, 0, callback)))
		{
			winrt::check_hresult(DrawThemeContent(hdc, *lprc, nullptr, nullptr, 0, callbackAsFallback));
		}

		GdiFlush();

		return S_OK;
	}
	catch (...)
	{
		return winrt::to_hresult();
	}
}