/**
 * FileName: DwmAPIEffect.cpp
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
#include "DwmAPIEffect.h"
#include "CommonDef.h"

namespace MDWMBlurGlassExt::DwmAPIEffect
{
	using namespace MDWMBlurGlass;
	using namespace CommonDef;
	using namespace DWM;

	std::atomic_bool g_startup = false;

	void Attach()
	{
		if (os::buildNumber < 22621)
			return;
		if (g_startup) return;
		g_startup = true;
		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<3, true>();
	}

	void Detach()
	{
		if (!g_startup) return;
		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<3, false>();
		g_startup = false;
	}

	void Refresh()
	{
		if (os::buildNumber < 22621)
			return;
		if (g_configData.blurmethod == blurMethod::DWMAPIBlur && !g_startup && !g_configData.powerSavingMode)
			Attach();
		else if ((g_configData.blurmethod != blurMethod::DWMAPIBlur || g_configData.powerSavingMode) && g_startup)
			Detach();
	}

	HRESULT CTopLevelWindow_ValidateVisual(CTopLevelWindow* This)
	{
		if (const auto windowData = This->GetData())
		{
			const HWND hWnd = windowData->GetHWND();

			if (os::buildNumber >= 22621 && This->HasNonClientBackground())
			{
				DWM_SYSTEMBACKDROP_TYPE type = DWMSBT_NONE;
				switch ((int)g_configData.effectType)
				{
				case 0:
					type = DWMSBT_MAINWINDOW;
					break;
				case 1:
					type = DWMSBT_TRANSIENTWINDOW;
					break;
				case 2:
					type = DWMSBT_TABBEDWINDOW;
					break;
				default:
					type = DWMSBT_AUTO;
					break;
				}
				DWM_SYSTEMBACKDROP_TYPE type1;
				DwmGetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &type1, sizeof type);
				if(type1 != type)
					DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &type, sizeof type);
			}
		}
		return S_OK;
	}
}
