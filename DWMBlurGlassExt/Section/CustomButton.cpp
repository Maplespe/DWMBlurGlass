/**
 * FileName: CustomButton.cpp
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
#include "CustomButton.h"
#include "../DWMBlurGlass.h"
#include <shellscalingapi.h>
#pragma comment(lib, "shcore.lib")

namespace MDWMBlurGlassExt::CustomButton
{
	using namespace MDWMBlurGlass;
	using namespace CommonDef;
	using namespace DWM;

	std::atomic_bool g_startup = false;
	thread_local std::unordered_map<CButton*, RECT> g_cbuttonList;

	MinHook g_funCButton_UpdateLayout
	{
		"CButton::UpdateLayout", CButton_UpdateLayout
	};

	void Attach()
	{
		if (g_startup) return;
		g_startup = true;

		g_funCButton_UpdateLayout.Attach();
		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<1, true>();

	}

	void Detach()
	{
		if (!g_startup) return;

		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<1, false>();
		g_funCButton_UpdateLayout.Detach();
		g_cbuttonList.clear();

		g_startup = false;
	}

	void Refresh()
	{
		if (g_configData.blurmethod != blurMethod::DWMAPIBlur && g_configData.oldBtnHeight && !g_startup)
			Attach();
		else if ((!g_configData.oldBtnHeight || g_configData.blurmethod == blurMethod::DWMAPIBlur) && g_startup)
			Detach();
	}

	HRESULT CTopLevelWindow_ValidateVisual(CTopLevelWindow* This)
	{
		auto data = This->GetData();
		if (!g_configData.oldBtnHeight || !data)
			return S_OK;

		g_window = data->GetHWND();

		if (!g_window || GetWindowLongPtrW(g_window, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
			return S_OK;

		auto PushButtonPtr = [&](int index, int x, int width, int height)
		{
			CButton** button;
			if (os::buildNumber < 22000)
				button = (CButton**)((DWORD64)This + 8 * (index + 61));
			else if (os::buildNumber < 22621)
				button = (CButton**)((DWORD64)This + 8 * (index + 63));
			else
				button = (CButton**)((DWORD64)This + 8 * (index + 66));

			if (button && *button)
			{
				g_cbuttonList[*button] = { x, 0, width, height };
				if (os::buildNumber < 22000)
				{
					DWORD right = x;
					This->UpdateNCAreaButton(index, height, (*button)->GetPoint()->y, &right);
				}
				else
					((CVisual*)(*button))->SetInsetFromParentRight(x);
				return true;
			}

			return false;
		};

		RECT rect;
		This->GetActualWindowRect(&rect, 0, 0, false);
		auto monitor = MonitorFromPoint({ max(0, rect.left), max(0, rect.top) }, 0);
		UINT dpi = 96;// CDesktopManager::s_pDesktopManagerInstance->MonitorDpiFromPoint({ rect.left, rect.top });
		GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi, &dpi);

		const float scale = (float)dpi / 96.f;

		RECT client;
		This->GetActualWindowRect(&client, 0, 0, true);

		if (IsIconic(g_window))
			return S_OK;

		int frameY = 20;
		int borderW = ((rect.right - rect.left) - (client.right - client.left)) / 2;

		if (scale != 1.f)
			borderW -= (int)ceil(1.1f * scale);

		if(os::buildNumber < 22000 && borderW <= 0)
		{
			borderW = 6;
			if (IsZoomed(g_window))
				borderW = 10;
		}

		OffsetRect(&rect, -rect.left, -rect.top);

		if (g_configData.titlebtnOffsetX != -1)
			borderW = g_configData.titlebtnOffsetX;

		borderW = int((float)borderW * scale);
		const int minbtn_width = int(29.f * scale);
		const int maxbtn_width = int(27.f * scale);
		const int closebtn_width = int(49.f * scale);
		const int height = int((float)frameY * scale);

		bool maxbtn = false;
		bool minbtn = false;

		int offset = rect.right - borderW;
		//关闭按钮
		if (PushButtonPtr(3, rect.right - closebtn_width - borderW, closebtn_width, height))
			offset -= closebtn_width + maxbtn_width;
		//最大化按钮
		if (PushButtonPtr(2, offset, maxbtn_width, height))
		{
			offset -= minbtn_width;
			maxbtn = true;
		}
		//最小化按钮
		if (PushButtonPtr(1, offset, minbtn_width, height))
		{
			offset -= minbtn_width;
			minbtn = true;
		}
		//帮助按钮
		if (!maxbtn && !minbtn)
			offset -= int(2.f * scale);
		PushButtonPtr(0, offset, minbtn_width, height);


		return S_OK;
	}

	HRESULT CButton_UpdateLayout(CButton* This)
	{
		if (g_configData.oldBtnHeight)
		{
			POINT* pt = This->GetPoint();
			SIZE* size = This->GetSize();

			auto iter = g_cbuttonList.find(This);
			if (iter == g_cbuttonList.end())
				return g_funCButton_UpdateLayout.call_org(This);

			size->cx = iter->second.right;
			size->cy = iter->second.bottom;

			auto ret = g_funCButton_UpdateLayout.call_org(This);
			pt->x = iter->second.left;

			if (!g_window)
				g_cbuttonList.erase(iter);
			return ret;

		}
		return g_funCButton_UpdateLayout.call_org(This);
	}
}
