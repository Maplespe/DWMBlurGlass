/**
 * FileName: DWMBlurGlass.cpp
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
#include "DefFunctionList.h"
#include "DWMBlurGlass.h"
#include "Section/OcclusionCulling.h"
#include "HookDef.h"
#include <mutex>
#include <minidumpapiset.h>

namespace MDWMBlurGlassExt
{
	using namespace MDWMBlurGlass;
	using namespace DWM;

	namespace CommonDef
	{
		ConfigData g_configData;
		CWindowList* g_windowList = nullptr;
		thread_local HWND g_window = nullptr;
		COLORREF g_accentColor = 0;
		IWICImagingFactory2* g_wicImageFactory = nullptr;
	}

	using namespace CommonDef;

	bool g_startup = false;

	decltype(CreateRoundRectRgn)* g_funCreateRoundRgn = nullptr;
	LPTOP_LEVEL_EXCEPTION_FILTER g_oldExceptionFilter = nullptr;

	MinHook g_funHrgnFromRects
	{
		"HrgnFromRects", HrgnFromRects
	};

	LONG NTAPI TopLevelExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
	{
		LONG result{ g_oldExceptionFilter ? g_oldExceptionFilter(exceptionInfo) : EXCEPTION_CONTINUE_SEARCH };
		[exceptionInfo]()
			{
				CreateDirectoryW((Utils::GetCurrentDir() + L"\\data\\dumps").c_str(), nullptr);

				std::time_t tt{ std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) };
				tm _tm{};
				WCHAR time[MAX_PATH + 1];
				localtime_s(&_tm, &tt);
				std::wcsftime(time, MAX_PATH, L"%Y-%m-%d-%H-%M-%S", &_tm);

				wil::unique_hfile fileHandle
				{
					CreateFile2(
						(Utils::GetCurrentDir() +
							std::wstring{L"\\data\\dumps\\minidump-"} + time + L".dmp"
						).c_str(),
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ,
						CREATE_ALWAYS,
						nullptr
					)
				};
				RETURN_LAST_ERROR_IF(!fileHandle.is_valid());

				MINIDUMP_EXCEPTION_INFORMATION minidumpExceptionInfo{ GetCurrentThreadId(), exceptionInfo, FALSE };
				RETURN_IF_WIN32_BOOL_FALSE(
					MiniDumpWriteDump(
						GetCurrentProcess(),
						GetCurrentProcessId(),
						fileHandle.get(),
						static_cast<MINIDUMP_TYPE>(
							MINIDUMP_TYPE::MiniDumpNormal |
							MINIDUMP_TYPE::MiniDumpWithThreadInfo |
							MINIDUMP_TYPE::MiniDumpWithUnloadedModules |
							MINIDUMP_TYPE::MiniDumpWithProcessThreadData
							),
						&minidumpExceptionInfo,
						nullptr,
						nullptr
					)
				);

				return S_OK;
			} ();

		return result;
	}

	bool Startup() try
	{
		using namespace CommonDef;

		if (g_startup) return true;
		g_startup = true;

		g_oldExceptionFilter = SetUnhandledExceptionFilter(TopLevelExceptionFilter);

		MHostLoadProcOffsetList();

		CDesktopManager::s_pDesktopManagerInstance = *MHostGetProcAddress<CDesktopManager*>("CDesktopManager::s_pDesktopManagerInstance");
		CDesktopManager::s_csDwmInstance = MHostGetProcAddress<CRITICAL_SECTION>("CDesktopManager::s_csDwmInstance");

		if(os::buildNumber < 22000)
		{
			g_funHrgnFromRects.Attach();
		}

		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<4, true>();

		TitleTextTweaker::Attach();

		HMODULE udwmModule = GetModuleHandleW(L"udwm.dll");
		if (os::buildNumber < 22000)
		{
			HMODULE hModule = GetModuleHandleW(L"gdi32.dll");
			g_funCreateRoundRgn = (decltype(g_funCreateRoundRgn))GetProcAddress(hModule, "CreateRoundRectRgn");
			WriteIAT(udwmModule, "gdi32.dll", { { "CreateRoundRectRgn", MyCreateRoundRectRgn } });
		}

		Refresh();
		return true;
	}
	catch(...)
	{
		return false;
	}

	void Shutdown()
	{
		if (!g_startup) return;

		if (os::buildNumber < 22000)
		{
			g_funHrgnFromRects.Detach();
		}

		CustomButton::Detach();
		OcclusionCulling::Detach();
		TitleTextTweaker::Detach();
		BlurRadiusTweaker::Detach();
		AccentBlur::Detach();
		CustomBackdrop::Detach();
		DwmAPIEffect::Detach();
		ScaleOptimizer::Detach();

		g_CTopLevelWindow_ValidateVisual_HookDispatcher.enable_hook_routine<4, false>();

		HMODULE udwmModule = GetModuleHandleW(L"udwm.dll");
		if(os::buildNumber < 22000)
			WriteIAT(udwmModule, "gdi32.dll", { { "CreateRoundRectRgn", g_funCreateRoundRgn } });

		if (g_oldExceptionFilter)
		{
			SetUnhandledExceptionFilter(g_oldExceptionFilter);
			g_oldExceptionFilter = nullptr;
		}
		g_startup = false;

		PostMessageW(FindWindowW(L"Dwm", nullptr), WM_THEMECHANGED, 0, 0);
	}

	void Refresh(bool reload)
	{
		if (!g_startup) return;

		static std::mutex lock;
		std::lock_guard _lock(lock);

		if (reload)
		{
			auto path = Utils::GetCurrentDir() + L"\\data\\config.ini";
			g_configData = ConfigData::LoadFromFile(path);
		}
		DwmAPIEffect::Refresh();
		CustomBackdrop::Refresh();
		OcclusionCulling::Refresh();
		AccentBlur::Refresh();
		BlurRadiusTweaker::Refresh();
		CustomButton::Refresh();
		ScaleOptimizer::Refresh();

		if (g_configData.useAccentColor)
			RefreshAccentColor(0);

		PostMessageW(FindWindowW(L"Dwm", nullptr), WM_THEMECHANGED, 0, 0);
	}

	void RefreshAccentColor(COLORREF color)
	{
		static COLORREF lastColor = 0;
		if(color == 0)
		{
			PostMessageW(FindWindowW(L"Dwm", nullptr), WM_THEMECHANGED, 0, 0);

			BOOL blend = TRUE;
			DwmGetColorizationColor(&color, &blend);
		}

		BYTE blue = GetRValue(color);
		BYTE green = GetGValue(color);
		BYTE red = GetBValue(color);
		color = RGB(red, green, blue);

		if(lastColor != color && g_configData.useAccentColor)
		{
			//CustomBackdrop::Refresh();
			//AccentBlur::Refresh();

			g_accentColor = color;
			lastColor = color;
		}
	}

	namespace Common
	{
		HRESULT CTopLevelWindow_ValidateVisual(CTopLevelWindow* This)
		{
			g_window = nullptr;
			return S_OK;
		}
	}

	HRGN __stdcall MyCreateRoundRectRgn(int x1, int y1, int x2, int y2, int w, int h)
	{
		if(g_configData.extendBorder)
		{
			w = h = g_configData.extendRound;
		}
		return g_funCreateRoundRgn(x1, y1, x2, y2, w, h);
	}


	DWORD64 __stdcall HrgnFromRects(const tagRECT* Src, unsigned int a2, HRGN* a3)
	{
		auto ret = g_funHrgnFromRects.call_org(Src, a2, a3);
		if (g_window && g_configData.extendBorder && ret == S_OK)
		{
			RECT rect;
			GetRgnBox(*a3, &rect);
			HRGN hRgn = MyCreateRoundRectRgn(rect.left, rect.top, rect.right, rect.bottom, 8, 8);
			CombineRgn(*a3, hRgn, nullptr, RGN_AND);
			DeleteObject(hRgn);
			return ret;
		}
		return ret;
	}

	//DWORD64 CWindowList_BlurBehindChange(
	//	CWindowList* This,
	//	IDwmWindow* a2,
	//	const DWM_BLURBEHIND* a3)
	//{
	//	auto ret = g_funCWindowList_BlurBehindChange.call_org(This, a2, a3);
	//	if(a3->hRgnBlur)
	//	{
	//		RECT rect;
	//		GetRgnBox(a3->hRgnBlur, &rect);
	//		if (rect.right - rect.left <= 1 || rect.bottom - rect.top <= 1)
	//			return ret;
	//	}

	//	CWindowData* windowData = nullptr;
	//	This->GetSyncedWindowData(a2, false, &windowData);

	//	if (!windowData)
	//		return ret;

	//	HWND hWnd = windowData->GetHWND();

	//	if (os::buildNumber >= 22621)
	//	{
	//		DWM_SYSTEMBACKDROP_TYPE type = a3->fEnable ? DWMSBT_TRANSIENTWINDOW : DWMSBT_NONE;
	//		DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &type, sizeof type);
	//	}
	//	else
	//	{
	//		/*
	//		struct WINDOWCOMPOSITIONATTRIBUTEDATA
	//		{
	//			DWORD dwAttribute;
	//			PVOID pvData;
	//			SIZE_T cbData;
	//		};
	//		static auto SetWindowCompositionAttribute = reinterpret_cast<BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBUTEDATA*)>(
	//			GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"));
	//		ACCENT_POLICY policy = {};
	//		WINDOWCOMPOSITIONATTRIBUTEDATA data =
	//		{
	//			19,
	//			&policy,
	//			sizeof(policy)
	//		};
	//		policy.nAccentState = a3->fEnable ? 3 : 0;
	//		policy.nFlags = 0;
	//		SetWindowCompositionAttribute(hWnd, &data);*/
	//	}

	//	return ret;
	//}
}
