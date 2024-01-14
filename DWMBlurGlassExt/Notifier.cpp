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
#include "DWMBlurGlass.h"
#include "framework.h"
#include "Common.h"

namespace MDWMBlurGlassExt
{
	using namespace MDWMBlurGlass;

	HWND g_msgWnd = nullptr;

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
		}
		else if(message == WM_CLOSE)
		{
			PostQuitMessage(0);
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
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

			MSG msg;
			while (GetMessageW(&msg, nullptr, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}

			DestroyWindow(g_msgWnd);

			UnregisterClassW(wx.lpszClassName, this_inst);
		}).detach();
	}

	void CloseNotifyThread()
	{
		SendMessageW(g_msgWnd, WM_CLOSE, 0, 0);
	}
}