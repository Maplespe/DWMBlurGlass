/**
 * FileName: Notifier.cpp
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
#include "../DWMBlurGlass.h"
#include "../Common/CommonDef.h"
#pragma comment(lib, "Comctl32.lib")

namespace MDWMBlurGlassExt
{
	using namespace MDWMBlurGlass;

	HWND g_msgWnd = nullptr;
	WNDPROC g_dwmWndProc = nullptr;
	HPOWERNOTIFY g_powerNotify = nullptr;
	bool g_enableTransparency = true;
	bool g_powerSavingMode = false;

	HWND FindMessageWnd()
	{
		HWND hwnd = nullptr;
		do
		{
			hwnd = FindWindowExW(HWND_MESSAGE, hwnd, DWMBlurGlassHostNotifyClassName, nullptr);
			if (hwnd != nullptr)
			{
				return hwnd;
			}
		} while (hwnd != nullptr);
		return nullptr;
	}

	LRESULT CALLBACK MsgWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_CREATE)
		{
			Refresh();
		}
		else if (message == WM_APP + 20)
		{
			if (wParam == (ULONG_PTR)MHostNotifyType::Refresh)
			{
				Refresh();
			}
			else if (wParam == (ULONG_PTR)MHostNotifyType::Shutdown)
			{
				Shutdown();
				PostQuitMessage(0);
			}
			else if (wParam == (ULONG_PTR)MHostNotifyType::EnableTransparency)
			{
				g_enableTransparency = lParam;
				if (g_powerSavingMode || !g_enableTransparency)
					CommonDef::g_configData.powerSavingMode = true;
				else if (g_enableTransparency)
					CommonDef::g_configData.powerSavingMode = false;
				static bool lastValue = false;
				if (lastValue != CommonDef::g_configData.powerSavingMode)
				{
					Refresh(false);
					lastValue = CommonDef::g_configData.powerSavingMode;
				}
			}
		}
		else if(message == WM_CLOSE)
		{
			PostQuitMessage(0);
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

	LRESULT WINAPI MyWndSubProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_DWMCOLORIZATIONCOLORCHANGED:
			RefreshAccentColor(wParam);
			break;
		case WM_POWERBROADCAST:
			{
				if (wParam != PBT_POWERSETTINGCHANGE)
					break;

				auto settings = (PPOWERBROADCAST_SETTING)lParam;
				if (!IsEqualGUID(settings->PowerSetting, GUID_POWER_SAVING_STATUS))
					break;

				g_powerSavingMode = *(DWORD*)(settings->Data);
				CommonDef::g_configData.powerSavingMode = g_powerSavingMode;
				if (!g_enableTransparency)
					CommonDef::g_configData.powerSavingMode = true;
				Refresh(false);
			}
			break;
		case WM_SETTINGCHANGE:
			PostMessageW(FindMessageWnd(), WM_APP + 20, (WPARAM)MClientNotifyType::QueryTransparency, 0);
			break;
		default:
			if (g_dwmWndProc)
				return g_dwmWndProc(hWnd, message, wParam, lParam);
			break;
		}
		return 0;
	}

	void CreateNotifyThread()
	{
		std::thread([]
		{
			WNDCLASSEXW wx = {};
			wx.cbSize = sizeof(WNDCLASSEX);
			wx.lpfnWndProc = MsgWndProc;
			wx.hInstance = this_inst;
			wx.lpszClassName = DWMBlurGlassNotifyClassName;
			if (!RegisterClassExW(&wx))
				return;

			g_msgWnd = CreateWindowExW(0, wx.lpszClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
			ChangeWindowMessageFilterEx(g_msgWnd, WM_APP + 20, MSGFLT_ALLOW, nullptr);

			HWND dwmWnd = FindWindowW(L"Dwm", nullptr);
			g_dwmWndProc = (WNDPROC)SetWindowLongPtrW(dwmWnd, GWLP_WNDPROC, (LONG_PTR)MyWndSubProc);
			ChangeWindowMessageFilterEx(dwmWnd, WM_DWMCOLORIZATIONCOLORCHANGED, MSGFLT_ALLOW, nullptr);

			if((g_powerNotify = RegisterPowerSettingNotification(dwmWnd, &GUID_POWER_SAVING_STATUS, DEVICE_NOTIFY_WINDOW_HANDLE)))
				ChangeWindowMessageFilterEx(dwmWnd, WM_POWERBROADCAST, MSGFLT_ALLOW, nullptr);

			ChangeWindowMessageFilterEx(dwmWnd, WM_SETTINGCHANGE, MSGFLT_ALLOW, nullptr);

			PostMessageW(FindMessageWnd(), WM_APP + 20, (WPARAM)MClientNotifyType::QueryTransparency, 0);

			MSG msg;
			while (GetMessageW(&msg, nullptr, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}

			DestroyWindow(g_msgWnd);

			UnregisterClassW(wx.lpszClassName, this_inst);
			if (g_powerNotify)
				UnregisterPowerSettingNotification(g_powerNotify);

			SetWindowLongPtrW(dwmWnd, GWLP_WNDPROC, (LONG_PTR)g_dwmWndProc);
		}).detach();
	}

	void CloseNotifyThread()
	{
		SendMessageW(g_msgWnd, WM_CLOSE, 0, 0);
	}
}