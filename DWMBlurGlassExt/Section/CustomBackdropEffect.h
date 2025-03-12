/**
 * FileName: CustomBackdropEffect.h
 *
 * Copyright (C) 2024 Maplespe
 *
 * This file is part of MToolBox and DWMBlurGlass and MDWMBlurGlassExt.
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
#include "CompositedBackdrop.hpp"

namespace MDWMBlurGlassExt::CustomBackdrop
{
	void Attach();
	void Detach();
	void Refresh();

	struct MILCMD
	{
		HWND GetHwnd() const
		{
			return *reinterpret_cast<HWND*>(reinterpret_cast<ULONG_PTR>(this) + 4);
		}
		LPCRECT GetRect() const
		{
			return reinterpret_cast<LPCRECT>(reinterpret_cast<ULONG_PTR>(this) + 12);
		}
	};
	struct MILCMD_DWM_REDIRECTION_ACCENTBLURRECTUPDATE : MILCMD {};

	CompositedBackdropKind GetActualBackdropKind(DWM::CTopLevelWindow* This, bool overrideAccent = true);

	HRESULT STDMETHODCALLTYPE CDrawGeometryInstruction_Create(DWM::CBaseLegacyMilBrushProxy* brush, DWM::CBaseGeometryProxy* geometry, DWM::CDrawGeometryInstruction** instruction);
	HRESULT STDMETHODCALLTYPE CTopLevelWindow_UpdateNCAreaBackground(DWM::CTopLevelWindow* This);
	HRESULT STDMETHODCALLTYPE CTopLevelWindow_UpdateClientBlur(DWM::CTopLevelWindow* This);
	HRESULT STDMETHODCALLTYPE CTopLevelWindow_ValidateVisual(DWM::CTopLevelWindow* This);
	HRESULT STDMETHODCALLTYPE CTopLevelWindow_UpdateAccent(DWM::CTopLevelWindow* This, bool visibleAndUncloaked);
	DWORD STDMETHODCALLTYPE CTopLevelWindow_CalculateBackgroundType(DWM::CTopLevelWindow* This);
	HRESULT STDMETHODCALLTYPE CTopLevelWindow_UpdateSystemBackdropVisual(DWM::CTopLevelWindow* This);
	void STDMETHODCALLTYPE CTopLevelWindow_Destructor(DWM::CTopLevelWindow* This);
	HRESULT STDMETHODCALLTYPE CTopLevelWindow_InitializeVisualTreeClone(DWM::CTopLevelWindow* This, DWM::CTopLevelWindow* window, UINT cloneOptions);
	HRESULT STDMETHODCALLTYPE CTopLevelWindow_CloneVisualTree(DWM::CTopLevelWindow* This, DWM::CTopLevelWindow** window, bool unknown);
	HRESULT STDMETHODCALLTYPE CTopLevelWindow_CloneVisualTree_Pre1903(DWM::CTopLevelWindow* This, DWM::CTopLevelWindow** window, bool unknown1, bool unknown2, bool unknown3);
	HRESULT STDMETHODCALLTYPE CTopLevelWindow_OnClipUpdated(DWM::CTopLevelWindow* This);
	HRESULT STDMETHODCALLTYPE CWindowList_UpdateAccentBlurRect(DWM::CWindowList* This, const MILCMD_DWM_REDIRECTION_ACCENTBLURRECTUPDATE* milCmd);

	DWORD64 STDMETHODCALLTYPE CGlassColorizationParameters_AdjustWindowColorization_Win10(DWM::GpCC* a1, DWM::GpCC* a2, float a3, char a4);
	DWORD64 STDMETHODCALLTYPE CGlassColorizationParameters_AdjustWindowColorization_Win11(DWM::GpCC* a1, DWM::GpCC* a2, float a3, short a4);
}