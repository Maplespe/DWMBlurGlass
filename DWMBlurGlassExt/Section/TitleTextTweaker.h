/**
 * FileName: TitleTextTweaker.h
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
#pragma once
#include "DWMStruct.h"

namespace MDWMBlurGlassExt::TitleTextTweaker
{
	void Attach();
	void Detach();

	int WINAPI MyFillRect(
		HDC hdc,
		LPRECT rect,
		HBRUSH hbrush
	);

	int WINAPI MyDrawTextW(
		HDC     hdc,
		LPCWSTR lpchText,
		int     cchText,
		LPRECT  lprc,
		UINT    format
	);

	HRESULT WINAPI MyCreateBitmapFromHBITMAP(
		IWICImagingFactory2* This,
		HBITMAP hBitmap,
		HPALETTE hPalette,
		WICBitmapAlphaChannelOption options,
		IWICBitmap** ppIBitmap
	);

	HBITMAP WINAPI MyCreateBitmap(
		int nWidth,
		int nHeight,
		UINT nPlanes,
		UINT nBitCount,
		CONST VOID* lpBits
	);

	HRESULT WINAPI CTopLevelWindow_UpdateWindowVisuals(
		DWM::CTopLevelWindow* This
	);

	HRESULT WINAPI CTopLevelWindow_UpdateText(
		DWM::CTopLevelWindow* This,
		DWM::CTopLevelWindow::WindowFrame* a2,
		double a3
	);

	HRESULT CText_UpdateAlignmentTransform(DWM::CText* This);
}