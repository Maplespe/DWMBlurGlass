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
#include "../Backdrops/ButtonGlowBackdrop.hpp"
#include <shellscalingapi.h>
#include <algorithm>
//#include <mutex>
#pragma comment(lib, "shcore.lib")

namespace MDWMBlurGlassExt::CustomButton
{
	using namespace MDWMBlurGlass;
	using namespace CommonDef;
	using namespace DWM;

	std::atomic_bool g_startup = false;
	struct buttonData
	{
		std::unordered_map<CButton*, std::pair<int, RECT>> buttonList;
	};
	std::unordered_map<CTopLevelWindow*, buttonData> g_cbuttonList;
	std::unordered_map<CTopLevelWindow*, com_ptr<CButtonGlowBackdrop>> g_glowbackdropMap{};
	//std::mutex g_dslock;

	inline auto FindWindowFromButton(CButton* btn)
	{
		for (auto wnditer = g_cbuttonList.begin(); wnditer != g_cbuttonList.end(); ++wnditer)
		{
			if (auto iter = wnditer->second.buttonList.find(btn); iter != wnditer->second.buttonList.end())
				return wnditer;
		}
		return g_cbuttonList.end();
	}

	com_ptr<CButtonGlowBackdrop> GetOrCreateGlow(CTopLevelWindow* target, bool createIfNecessary)
	{
		auto it{ g_glowbackdropMap.find(target) };

		if (createIfNecessary)
		{
			if (it == g_glowbackdropMap.end())
			{
				auto visual = target->GetVisual();
				auto result{ g_glowbackdropMap.emplace(target, winrt::make_self<CButtonGlowBackdrop>(visual)) };
				if (result.second == true)
				{
					it = result.first;
				}
			}
		}

		return it == g_glowbackdropMap.end() ? nullptr : it->second;
	}

	void RemoveGlow(CTopLevelWindow* target)
	{
		auto it{ g_glowbackdropMap.find(target) };

		if (it != g_glowbackdropMap.end())
		{
			g_glowbackdropMap.erase(it);
		}
	}

	MinHook g_funCButton_UpdateLayout
	{
		"CButton::UpdateLayout", CButton_UpdateLayout
	};

	MinHook g_funCButton_Destructor
	{
		"CButton::~CButton", CButton_Destructor
	};

	MinHook g_funCButton_RedrawVisual
	{
		"CButton::RedrawVisual", CButton_RedrawVisual
	};

	void Attach()
	{
		if (g_startup) return;
		g_startup = true;

		g_funCButton_UpdateLayout.Attach();
		g_funCButton_Destructor.Attach();
		g_CTopLevelWindow_CTopLevelWindow_Destructor_HookDispatcher.enable_hook_routine<1, true>();
		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<1, true>();
		g_funCButton_RedrawVisual.Attach();
	}

	void Detach()
	{
		if (!g_startup) return;

		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<1, false>();
		g_funCButton_UpdateLayout.Detach();
		g_CTopLevelWindow_CTopLevelWindow_Destructor_HookDispatcher.enable_hook_routine<1, false>();
		g_funCButton_RedrawVisual.Detach();
		g_cbuttonList.clear();
		g_glowbackdropMap.clear();
		g_funCButton_Destructor.Detach();

		g_startup = false;
	}

	void Refresh()
	{
		if (!g_startup
			&& (
				(g_configData.blurmethod != blurMethod::DWMAPIBlur && g_configData.oldBtnHeight)
				|| (g_configData.blurmethod == blurMethod::CustomBlur && g_configData.titlebtnGlow)
				))
			Attach();
		else if (g_startup
			&& ((!g_configData.oldBtnHeight && !g_configData.titlebtnGlow) || g_configData.blurmethod == blurMethod::DWMAPIBlur)
			)
			Detach();
	}

	HRESULT CTopLevelWindow_ValidateVisual(CTopLevelWindow* This)
	{
		auto data = This->GetData();
		if ((!g_configData.oldBtnHeight && !g_configData.titlebtnGlow) || !data)
			return S_OK;

		g_window = data->GetHWND();

		if (!g_window)
			return S_OK;

		bool isToolWindow = GetWindowLongPtrW(g_window, GWL_EXSTYLE) & WS_EX_TOOLWINDOW;

		auto GetButtonPtr = [&](int index) -> CButton*
		{
			CButton** button;
			if (os::buildNumber < 22000)
				button = (CButton**)((DWORD64)This + 8 * (index + 61));
			else if (os::buildNumber < 22621)
				button = (CButton**)((DWORD64)This + 8 * (index + 63));
			else if (os::buildNumber < 26100)
				button = (CButton**)((DWORD64)This + 8 * (index + 66));
			else
				button = (CButton**)((DWORD64)This + 488 + 8 * index);

			if (button && *button)
				return *button;
			return nullptr;
		};

		auto PushButtonPtr = [&](int index, int x, int width, int height)
		{
			if (auto button = GetButtonPtr(index))
			{
				g_cbuttonList[This].buttonList[button] = std::make_pair(index, RECT{ x, 0, width, height });

				if (!g_configData.oldBtnHeight) return true;

				if (os::buildNumber < 22000 || os::buildNumber >= 26100)
				{
					DWORD right = x;
					This->UpdateNCAreaButton(index, height, button->GetPoint()->y, &right);
				}
				else
					((CVisual*)button)->SetInsetFromParentRight(x);
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

		int borderW = ((rect.right - rect.left) - (client.right - client.left)) / 2;

		if (scale != 1.f)
			borderW -= (int)ceil(1.1f * scale);

		if (os::buildNumber < 22000 && borderW <= 0)
		{
			borderW = 6;
			if (IsZoomed(g_window))
				borderW = 10;
		}

		OffsetRect(&rect, -rect.left, -rect.top);

		if (g_configData.titlebtnOffsetX != -1)
			borderW = g_configData.titlebtnOffsetX;

		borderW = int((float)borderW * scale);

		// 如果启用了自定义尺寸 使用配置中的按钮尺寸
		// 默认使用 Windows7 的数值尺寸
		int minbtn_width_base = 29;
		int maxbtn_width_base = 27;
		int closebtn_width_base = 49;
		int frameY = 20;

		if (g_configData.customTitleBtnSize)
		{
			closebtn_width_base = std::clamp(g_configData.customCloseBtnW, 14, 200);
			maxbtn_width_base = std::clamp(g_configData.customMaxBtnW, 14, 200);
			minbtn_width_base = std::clamp(g_configData.customMinBtnW, 14, 200);
			frameY = std::clamp(g_configData.customBtnFrameH, 14, 100);
		}

		const int minbtn_width = int((float)minbtn_width_base * scale);
		const int maxbtn_width = int((float)maxbtn_width_base * scale);
		const int closebtn_width = int((float)closebtn_width_base * scale);
		const int height = int((float)frameY * scale);

		bool closebtn = false;
		bool maxbtn = false;
		bool minbtn = false;

		int offset = rect.right - borderW;
		//关闭按钮
		if (PushButtonPtr(3, rect.right - closebtn_width - borderW, closebtn_width, height))
		{
			offset -= closebtn_width + maxbtn_width;
			closebtn = true;
		}
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

		if (!g_configData.titlebtnGlow) return S_OK;

		//按钮发光
		if (CWindowData* windowData = This->GetData())
		{
			com_ptr<CButtonGlowBackdrop> backdrop{ nullptr };
			if (
				This->HasNonClientBackground() &&
				AccentBlur::CheckWindowType(windowData->GetHWND())
				)
			{
				if (closebtn)
					backdrop = GetOrCreateGlow(This, true);
				else if ((backdrop = GetOrCreateGlow(This, false)))
					RemoveGlow(This);

				auto closebtnptr = GetButtonPtr(3);
				if (backdrop && closebtnptr)
				{
					backdrop->Update(3, *closebtnptr->GetPoint(), *closebtnptr->GetSize(), scale, 0.f);
				}
			}
		}

		return S_OK;
	}

	void CTopLevelWindow_Destructor(CTopLevelWindow* This)
	{
		if (g_configData.titlebtnGlow)
		{
			RemoveGlow(This);
		}
		//std::lock_guard lock{ g_dslock };
		if (auto iter = g_cbuttonList.find(This); iter != g_cbuttonList.end())
		{
			g_cbuttonList.erase(iter);
		}
	}

	void CButton_Destructor(CButton* This)
	{
		g_funCButton_Destructor.call_org(This);

		if (auto iter = FindWindowFromButton(This); iter != g_cbuttonList.end())
		{
			iter->second.buttonList.erase(This);
		}
	}

	HRESULT CButton_UpdateLayout(CButton* This)
	{
		if (g_configData.oldBtnHeight)
		{
			POINT* pt = This->GetPoint();
			SIZE* size = This->GetSize();

			auto iter = FindWindowFromButton(This);
			if (iter == g_cbuttonList.end())
				return g_funCButton_UpdateLayout.call_org(This);

			auto& data = iter->second.buttonList[This].second;
			size->cx = data.right;
			size->cy = data.bottom;

			auto ret = g_funCButton_UpdateLayout.call_org(This);
			pt->x = data.left;

			return ret;

		}
		return g_funCButton_UpdateLayout.call_org(This);
	}

	HRESULT CButton_RedrawVisual(CButton* This)
	{
		auto hr = g_funCButton_RedrawVisual.call_org(This);
		if (!g_configData.titlebtnGlow) return hr;

		//std::lock_guard lock{ g_dslock };
		auto iter = FindWindowFromButton(This);
		if (iter == g_cbuttonList.end())
			return hr;

		auto cwnd = iter->first;

		RECT rect;
		cwnd->GetActualWindowRect(&rect, 0, 0, false);
		auto monitor = MonitorFromPoint({ max(0, rect.left), max(0, rect.top) }, 0);
		UINT dpi = 96;
		GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi, &dpi);
		const float scale = (float)dpi / 96.f;

		if (auto backdrop = GetOrCreateGlow(cwnd, false))
		{
			auto& data = iter->second.buttonList[This];

			int state = 0;
			if (os::buildNumber >= 26100)
				state = *(int*)((char*)This + 328);
			else
				state = *(int*)((char*)This + 376);

			//OutputDebugStringW((L"state:" + std::to_wstring(state) + L"\n").c_str());
			SIZE size = { data.second.right, data.second.bottom };
			if (!g_configData.oldBtnHeight)
				size = *This->GetSize();
			backdrop->Update(data.first, *This->GetPoint(), size, scale, state == 2 || state == 1);
		}

		return hr;
	}
}
