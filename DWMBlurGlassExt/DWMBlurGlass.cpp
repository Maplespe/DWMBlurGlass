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
#include "HookDef.h"
#include "VersionHelper.h"
#include "Helper.h"
#include <dwmapi.h>
#include <Uxtheme.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dxguid.lib")

namespace MDWMBlurGlassExt
{
	using namespace MDWMBlurGlass;

	bool g_startup = false;
	ConfigData g_configData;

	PVOID* g_pDesktopManagerInstance_ptr = nullptr;
	PVOID g_pCreateBitmapFromHBITMAP = nullptr;
	IWICImagingFactory2* g_wicImageFactory = nullptr;
	CWindowList* g_windowList = nullptr;

	decltype(DrawTextW)* g_funDrawTextW = nullptr;
	decltype(FillRect)* g_funFillRect = nullptr;
	decltype(Vtbl::IWICImagingFactory2Vtbl::CreateBitmapFromHBITMAP) g_funCreateBitmapFromHBITMAP = nullptr;
	decltype(CreateRoundRectRgn)* g_funCreateRoundRgn = nullptr;
	decltype(CreateBitmap)* g_funCreateBitmap = nullptr;

	decltype(CWindowList_GetExtendedFrameBounds)* g_funCWindowList_GetExtendedFrameBounds = nullptr;
	decltype(CTopLevelWindow_OnAccentPolicyUpdated)* g_funCTopLevelWindow_OnAccentPolicyUpdated = nullptr;
	decltype(CTopLevelWindow_s_ChooseWindowFrameFromStyle)* g_funCTopLevelWindow_s_ChooseWindowFrameFromStyle = nullptr;
	decltype(CText_SetColor)* g_CText_SetColor = nullptr;
	decltype(CDrawingContext_FillEffect)* g_funCDrawingContext_FillEffect = nullptr;
	decltype(CText_SetText)* g_funCText_SetText = nullptr;
	decltype(CWindowList_GetSyncedWindowData)* g_funCWindowList_GetSyncedWindowData = nullptr;

	thread_local RECT g_blurRegion{ 0, 0, -1, -1 };
	thread_local CAccent* g_accentThis = nullptr;
	thread_local HWND g_window = nullptr;
	thread_local MilRectF g_dstRect = { 0 };

	winrt::com_ptr<ID2D1Bitmap1> g_aeroPeekBmp = nullptr;
	winrt::com_ptr<ID2D1Effect> g_scaleEffect = nullptr;

	//BlendColor
	MinHook g_funCRenderData_DrawSolidColorRectangle
	{
		"CRenderData::DrawSolidColorRectangle", CRenderData_DrawSolidColorRectangle
	};
	//Win11 BlurAmount
	MinHook g_funCRenderingTechnique_ExecuteBlur
	{
		"CRenderingTechnique::ExecuteBlur", CRenderingTechnique_ExecuteBlur
	};
	//Win10 BlurBehind
	MinHook g_funCD2DContext_FillEffect
	{
		"CD2DContext::FillEffect", CD2DContext_FillEffect
	};

	MinHook g_funCFilterEffect_CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive
	{
		"CFilterEffect::CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive", CFilterEffect_CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive
	};

	//Win10 WinUI HostBackdrop Brush
	MinHook g_funCCustomBlur_BuildEffect
	{
		"CCustomBlur::BuildEffect", CCustomBlur_BuildEffect
	};
	MinHook g_funCCustomBlur_DetermineOutputScale
	{
		"CCustomBlur::DetermineOutputScale", CCustomBlur_DetermineOutputScale
	};

	//Win10 ExtendBlendColor
	MinHook g_funCTopLevelWindow_UpdateNCAreaGeometry
	{
		"CTopLevelWindow::UpdateNCAreaGeometry", CTopLevelWindow_UpdateNCAreaGeometry
	};

	MinHook g_funHrgnFromRects
	{
		"HrgnFromRects", HrgnFromRects
	};

	MinHook g_funCAccent_UpdateAccentBlurRect
	{
		"CAccent::UpdateAccentBlurRect", CAccent_UpdateAccentBlurRect
	};

	MinHook g_funCAccent_UpdateAccentPolicy
	{
		"CAccent::UpdateAccentPolicy", CAccent_UpdateAccentPolicy
	};

	MinHook g_funCTopLevelWindow_ValidateVisual
	{
		"CTopLevelWindow::ValidateVisual", CTopLevelWindow_ValidateVisual
	};
	//Win10 TextColor
	MinHook g_funCTopLevelWindow_UpdateWindowVisuals
	{
		"CTopLevelWindow::UpdateWindowVisuals", CTopLevelWindow_UpdateWindowVisuals
	};

	MinHook g_funCGlassColorizationParameters_AdjustWindowColorization
	{
		"CGlassColorizationParameters::AdjustWindowColorization", CGlassColorizationParameters_AdjustWindowColorization
	};

	MinHook g_funCTopLevelWindow_UpdateText
	{
		"CTopLevelWindow::UpdateText", CTopLevelWindow_UpdateText
	};

	//为调用DwmEnableBlurBehind的程序启用效果
	MinHook g_funCWindowList_BlurBehindChange
	{
		"CWindowList::BlurBehindChange", CWindowList_BlurBehindChange
	};

	MinHook g_funCText_SetSize
	{
		"CText::SetSize", CText_SetSize
	};

	MinHook g_funCButton_UpdateLayout
	{
		"CButton::UpdateLayout", CButton_UpdateLayout
	};

	bool Startup() try
	{
		if (g_startup) return true;
		g_startup = true;

		MHostLoadProcOffsetList();

		g_funCTopLevelWindow_s_ChooseWindowFrameFromStyle = (decltype(g_funCTopLevelWindow_s_ChooseWindowFrameFromStyle))MHostGetProcAddress("CTopLevelWindow::s_ChooseWindowFrameFromStyle");
		g_funCWindowList_GetExtendedFrameBounds = (decltype(g_funCWindowList_GetExtendedFrameBounds))MHostGetProcAddress("CWindowList::GetExtendedFrameBounds");
		g_funCTopLevelWindow_OnAccentPolicyUpdated = (decltype(g_funCTopLevelWindow_OnAccentPolicyUpdated))MHostGetProcAddress("CTopLevelWindow::OnAccentPolicyUpdated");
		g_CText_SetColor = (decltype(g_CText_SetColor))MHostGetProcAddress("CText::SetColor");
		g_funCDrawingContext_FillEffect = (decltype(g_funCDrawingContext_FillEffect))MHostGetProcAddress("CDrawingContext::FillEffect");
		g_funCText_SetText = (decltype(g_funCText_SetText))MHostGetProcAddress("CText::SetText");
		g_funCWindowList_GetSyncedWindowData = (decltype(g_funCWindowList_GetSyncedWindowData))MHostGetProcAddress("CWindowList::GetSyncedWindowData");

		g_pDesktopManagerInstance_ptr = (PVOID*)MHostGetProcAddress("CDesktopManager::s_pDesktopManagerInstance");
		if (g_pDesktopManagerInstance_ptr && *g_pDesktopManagerInstance_ptr)
		{
			if (os::buildNumber >= 22621)
			{
				// Windows 11 22H2
				g_wicImageFactory = (IWICImagingFactory2*)*((PVOID*)*g_pDesktopManagerInstance_ptr + 30);
			}
			else if (os::buildNumber < 22000)
			{
				// Windows 10
				g_wicImageFactory = (IWICImagingFactory2*)*((PVOID*)*g_pDesktopManagerInstance_ptr + 39);
				g_windowList = (CWindowList*)(*((PVOID*)*g_pDesktopManagerInstance_ptr + 61));
			}

			if (((Vtbl::IWICImagingFactory2*)g_wicImageFactory)->lpVtbl)
			{
				g_pCreateBitmapFromHBITMAP = &((Vtbl::IWICImagingFactory2*)g_wicImageFactory)->lpVtbl->CreateBitmapFromHBITMAP;
				g_funCreateBitmapFromHBITMAP = ((Vtbl::IWICImagingFactory2*)g_wicImageFactory)->lpVtbl->CreateBitmapFromHBITMAP;
			}

			if (g_pCreateBitmapFromHBITMAP)
				WriteMemory(g_pCreateBitmapFromHBITMAP, [&] { *(PVOID*)g_pCreateBitmapFromHBITMAP = MyCreateBitmapFromHBITMAP; });
		}

		if(os::buildNumber < 22000)
		{
			g_funCText_SetSize.Attach();
			g_funCRenderData_DrawSolidColorRectangle.Attach();
			g_funCFilterEffect_CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive.Attach();
			g_funCD2DContext_FillEffect.Attach();
			g_funCCustomBlur_BuildEffect.Attach();
			g_funCCustomBlur_DetermineOutputScale.Attach();
			g_funCAccent_UpdateAccentBlurRect.Attach();
			g_funCAccent_UpdateAccentPolicy.Attach();
			g_funCTopLevelWindow_UpdateWindowVisuals.Attach();
			g_funCTopLevelWindow_UpdateNCAreaGeometry.Attach();
			g_funHrgnFromRects.Attach();
		}
		if(os::buildNumber >= 22000)
		{
			g_funCRenderingTechnique_ExecuteBlur.Attach();
			g_funCTopLevelWindow_UpdateText.Attach();
		}

		g_funCButton_UpdateLayout.Attach();
		g_funCTopLevelWindow_ValidateVisual.Attach();
		g_funCGlassColorizationParameters_AdjustWindowColorization.Attach();
		g_funCWindowList_BlurBehindChange.Attach();

		HMODULE udwmModule = GetModuleHandleW(L"udwm.dll");
		if(os::buildNumber <= 22621)
		{
			HMODULE hModule = GetModuleHandleW(L"user32.dll");
			g_funFillRect = (decltype(g_funFillRect))GetProcAddress(hModule, "FillRect");
			g_funDrawTextW = (decltype(g_funDrawTextW))GetProcAddress(hModule, "DrawTextW");
			WriteIAT(udwmModule, "User32.dll", {{ "FillRect", MyFillRect }, { "DrawTextW", MyDrawTextW }});
		}
		if (os::buildNumber <= 22000)
		{
			HMODULE hModule = GetModuleHandleW(L"gdi32.dll");
			g_funCreateRoundRgn = (decltype(g_funCreateRoundRgn))GetProcAddress(hModule, "CreateRoundRectRgn");
			g_funCreateBitmap = (decltype(g_funCreateBitmap))GetProcAddress(hModule, "CreateBitmap");
			WriteIAT(udwmModule, "gdi32.dll", { { "CreateRoundRectRgn", MyCreateRoundRectRgn }, { "CreateBitmap", MyCreateBitmap }});
		}
		return true;
	}
	catch(...)
	{
		return false;
	}

	void Shutdown()
	{
		if (!g_startup) return;

		if (os::buildNumber >= 22000)
		{
			g_funCTopLevelWindow_UpdateText.Detach();
			g_funCRenderingTechnique_ExecuteBlur.Detach();
		}
		if (os::buildNumber < 22000)
		{
			g_funCText_SetSize.Detach();
			g_funHrgnFromRects.Detach();
			g_funCTopLevelWindow_UpdateNCAreaGeometry.Detach();
			g_funCTopLevelWindow_UpdateWindowVisuals.Detach();
			g_funCAccent_UpdateAccentPolicy.Detach();
			g_funCAccent_UpdateAccentBlurRect.Detach();
			g_funCCustomBlur_DetermineOutputScale.Detach();
			g_funCCustomBlur_BuildEffect.Detach();
			g_funCD2DContext_FillEffect.Detach();
			g_funCFilterEffect_CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive.Detach();
			g_funCRenderData_DrawSolidColorRectangle.Detach();
		}

		g_funCButton_UpdateLayout.Detach();
		g_funCWindowList_BlurBehindChange.Detach();
		g_funCGlassColorizationParameters_AdjustWindowColorization.Detach();
		g_funCTopLevelWindow_ValidateVisual.Detach();

		if(g_pCreateBitmapFromHBITMAP)
			WriteMemory(g_pCreateBitmapFromHBITMAP, [&] {*(PVOID*)g_pCreateBitmapFromHBITMAP = g_funCreateBitmapFromHBITMAP; });

		HMODULE udwmModule = GetModuleHandleW(L"udwm.dll");
		if(os::buildNumber <= 22621)
		{
			WriteIAT(udwmModule, "User32.dll", {{ "DrawTextW", g_funDrawTextW } , { "FillRect", g_funFillRect }});
		}
		WriteIAT(udwmModule, "gdi32.dll", { { "CreateRoundRectRgn", g_funCreateRoundRgn }, { "CreateBitmap", g_funCreateBitmap }});

		g_startup = false;
	}

	void Refresh()
	{
		auto path = Utils::GetCurrentDir() + L"\\data\\config.ini";
		auto ret = Utils::GetIniString(path, L"config", L"blurAmount");

		g_configData.applyglobal = Utils::GetIniString(path, L"config", L"applyglobal") == L"true";
		g_configData.extendBorder = Utils::GetIniString(path, L"config", L"extendBorder") == L"true";
		g_configData.reflection = Utils::GetIniString(path, L"config", L"reflection") == L"true";
		g_configData.oldBtnHeight = Utils::GetIniString(path, L"config", L"oldBtnHeight") == L"true";

		if (!ret.empty())
			g_configData.extendRound = (float)std::clamp(_wtoi(ret.data()), 0, 16);

		if (!ret.empty())
			g_configData.blurAmount = (float)std::clamp(_wtof(ret.data()), 0.0, 50.0);

		ret = Utils::GetIniString(path, L"config", L"activeTextColor");
		if (!ret.empty())
		{
			g_configData.activeTextColor = (COLORREF)_wtoll(ret.data());
			g_configData.activeTextColor = (g_configData.activeTextColor & 0x00FFFFFF) | 0xFF000000;
		}

		ret = Utils::GetIniString(path, L"config", L"inactiveTextColor");
		if (!ret.empty())
		{
			g_configData.inactiveTextColor = (COLORREF)_wtoll(ret.data());
			g_configData.inactiveTextColor = (g_configData.inactiveTextColor & 0x00FFFFFF) | 0xFF000000;
		}

		ret = Utils::GetIniString(path, L"config", L"activeBlendColor");
		if (!ret.empty())
			g_configData.activeBlendColor = (COLORREF)_wtoll(ret.data());

		ret = Utils::GetIniString(path, L"config", L"inactiveBlendColor");
		if (!ret.empty())
			g_configData.inactiveBlendColor = (COLORREF)_wtoll(ret.data());

		if (g_scaleEffect)
			g_scaleEffect = nullptr;
		if (g_aeroPeekBmp)
			g_aeroPeekBmp = nullptr;
	}
	
	HRESULT __stdcall CRenderData_DrawSolidColorRectangle(
		CRenderData* This, CDrawingContext* a2, CDrawListEntryBuilder* a3,
		bool a4, const MilRectF* a5, D3DCOLORVALUE* a6)
	{
		auto color = g_configData.inactiveBlendColor;
		if(g_window && GetForegroundWindow() == g_window)
			color = g_configData.activeBlendColor;
		a6->a = float(UINT(color >> 24) & 0xff) / 255.f;
		return g_funCRenderData_DrawSolidColorRectangle.call_org(This, a2, a3, a4, a5, a6);
	}

	DWORD64 __stdcall CRenderingTechnique_ExecuteBlur(
		CRenderingTechnique* This, CDrawingContext* a2, const EffectInput* a3,
		const D2D_VECTOR_2F* a4, EffectInput* a5)
	{
		auto blurVector = *a4;
		blurVector.x = blurVector.y = (g_configData.blurAmount / 2.f) / 10.f;
		return g_funCRenderingTechnique_ExecuteBlur.call_org(This, a2, a3, &blurVector, a5);
	}

	winrt::com_ptr<ID2D1Bitmap1> CreateD2DBitmap(ID2D1DeviceContext* context, std::wstring_view filename)
	{
		using namespace winrt;

		if (!context && !g_wicImageFactory)
			return nullptr;

		com_ptr<IWICBitmapDecoder> decoder = nullptr;

		HRESULT hr = g_wicImageFactory->CreateDecoderFromFilename(filename.data(), &GUID_VendorMicrosoft, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.put());

		if (FAILED(hr))
			return nullptr;

		com_ptr<IWICBitmapFrameDecode> frame = nullptr;
		hr = decoder->GetFrame(0, frame.put());
		if (FAILED(hr))
			return nullptr;

		com_ptr<IWICFormatConverter> converter = nullptr;
		hr = g_wicImageFactory->CreateFormatConverter(converter.put());
		if (FAILED(hr))
			return nullptr;

		com_ptr<IWICPalette> palette = nullptr;
		hr = converter->Initialize(frame.get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, palette.get(), 0, WICBitmapPaletteTypeCustom);

		if (FAILED(hr))
			return nullptr;

		com_ptr<IWICBitmap> wicbitmap = nullptr;
		hr = g_wicImageFactory->CreateBitmapFromSource(converter.get(), WICBitmapNoCache, wicbitmap.put());

		if (FAILED(hr))
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

	void EnumMonitors(std::vector<RECT>& monitorRects)
	{
		auto MonitorEnumProc = [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
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

	DWORD64 __stdcall CD2DContext_FillEffect(
		ID2D1DeviceContext** This,
		const ID2DContextOwner* a2,
		ID2D1Effect* inputEffect,
		const D2D_RECT_F* srcRect,
		const D2D_POINT_2F* dstPoint,
		D2D1_INTERPOLATION_MODE interMode,
		D2D1_COMPOSITE_MODE mode)
	{
		const UINT32 count = inputEffect->GetPropertyCount();
		for (UINT i = 0; i < count; i++)
		{
			wchar_t name[256];
			inputEffect->GetPropertyName(i, name, 256);

			if (wcscmp(name, L"StandardDeviation") != 0)
				continue;

			float dv = 0.f;
			inputEffect->GetValue(i, &dv);

			//HostBackdrop 
			if (count == 4)
			{
				return g_funCD2DContext_FillEffect.call_org(This, a2, inputEffect, srcRect, dstPoint, interMode, mode);
			}

			//BlurBehind
			inputEffect->SetValue(D2D1_DIRECTIONALBLUR_PROP_STANDARD_DEVIATION, g_configData.blurAmount);

			auto ret = S_OK;

			//无论如何 哪怕是0 都会有一点点的模糊 所以如果要透明 可以不绘制
			if(g_configData.blurAmount != 0.f)
				g_funCD2DContext_FillEffect.call_org(This, a2, inputEffect, srcRect, dstPoint, interMode, mode);

			if(!g_aeroPeekBmp && g_configData.reflection)
			{
				g_aeroPeekBmp = CreateD2DBitmap(This[30], Utils::GetCurrentDir() + L"\\data\\AeroPeek.png");
				if(g_aeroPeekBmp)
					This[30]->CreateEffect(CLSID_D2D1Scale, g_scaleEffect.put());
			}
			if(g_aeroPeekBmp)
			{
				g_scaleEffect->SetInput(0, g_aeroPeekBmp.get());

				auto size = g_aeroPeekBmp->GetSize();

				std::vector<RECT> monitorList;
				EnumMonitors(monitorList);

				MilRectF scaleRect = g_dstRect;
				D2D1_SIZE_F dstSize = { scaleRect.right - scaleRect.left, scaleRect.bottom - scaleRect.top };
				scaleRect.left = -scaleRect.left;
				scaleRect.top = -scaleRect.top;
				scaleRect.right = scaleRect.left + dstSize.width;
				scaleRect.bottom = scaleRect.top + dstSize.height;

				if (scaleRect.left < 0)
					scaleRect.left = 0.f;
				if (scaleRect.top < 0)
					scaleRect.top = 0.f;

				scaleRect.left *= 2.f;
				scaleRect.top *= 2.f;
				scaleRect.right *= 2.f;
				scaleRect.bottom *= 2.f;

				//查找目标矩形在哪个显示器
				SIZE monitorSize = { 0 };
				for (auto& rect : monitorList)
				{
					RECT _rect{ (int)scaleRect.left, (int)scaleRect.top, (int)scaleRect.right, (int)scaleRect.bottom };
					if (!IsRectInside(rect, _rect))
						continue;
					monitorSize.cx = rect.right - rect.left;
					monitorSize.cy = rect.bottom - rect.top;
					scaleRect.left -= (float)rect.left;
					scaleRect.top -= (float)rect.top;
					scaleRect.right = scaleRect.left + dstSize.width;
					scaleRect.bottom = scaleRect.top + dstSize.height;
					break;
				}

				scaleRect.left *= 0.5f;
				scaleRect.top *= 0.5f;
				scaleRect.right *= 0.5f;
				scaleRect.bottom *= 0.5f;

				float scaleX;
				float scaleY;
				if ((float)monitorSize.cx > size.width)
					scaleX = (float)monitorSize.cx / size.width;
				else
					scaleX = 1.f / (size.width / (float)monitorSize.cx / 1.f);

				if ((float)monitorSize.cy > size.height)
					scaleY = (float)monitorSize.cy / size.height;
				else
					scaleY = 1.f / (size.height / (float)monitorSize.cy);

				g_scaleEffect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(scaleX * 0.5f, scaleY * 0.5f));

				This[30]->DrawImage(g_scaleEffect.get(), dstPoint, (D2D1_RECT_F*)&scaleRect);
			}

			inputEffect->SetValue(D2D1_DIRECTIONALBLUR_PROP_STANDARD_DEVIATION, 3.f);
			return ret;
		}
		return g_funCD2DContext_FillEffect.call_org(This, a2, inputEffect, srcRect, dstPoint, interMode, mode);
	}

	DWORD64 CFilterEffect_CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive(
		CFilterEffect* This,
		MilRectF* a2,
		MilRectF* a3,
		MilRectF* a4,
		MilRectF* a5,
		MilRectF* a6)
	{
		g_dstRect = *a4;
		return g_funCFilterEffect_CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive.call_org(This, a2, a3, a4, a5, a6);
	}

	DWORD64 __stdcall CCustomBlur_BuildEffect(
		CCustomBlur* This,
		ID2D1Image* backdropImage,
		const D2D_RECT_F* srcRect,
		const D2D_SIZE_F* kSize,
		D2D1_GAUSSIANBLUR_OPTIMIZATION a5,
		const D2D_VECTOR_2F* a6,
		D2D_VECTOR_2F* a7)
	{
		//ID2D1Effect* cropEffect = *((ID2D1Effect**)This + 3);
		//ID2D1Effect* scaleEffect = *((ID2D1Effect**)This + 5);
		ID2D1Effect* directionalBlurKernelEffectX = *((ID2D1Effect**)This + 6);
		ID2D1Effect* directionalBlurKernelEffectY = *((ID2D1Effect**)This + 7);

		auto ret = g_funCCustomBlur_BuildEffect.call_org(This, backdropImage, srcRect, kSize, a5, a6, a7);

		directionalBlurKernelEffectX->SetValue(0, g_configData.blurAmount);
		directionalBlurKernelEffectY->SetValue(0, g_configData.blurAmount);

		return ret;
	}

	float __stdcall CCustomBlur_DetermineOutputScale(
		float a1,
		float a2, 
		D2D1_GAUSSIANBLUR_OPTIMIZATION a3)
	{
		//hostBackdrop画刷会预先使用矩阵缩放为2.5倍 再利用BuildEffect中的D2D Scale效果缩放回去 然后利用2次自定义高斯内核模糊 达到快速高模糊度计算
		//这里设置为1 则不缩放 否则BuildEffect中的图像会非常模糊 马赛克一样
		return 1.f;
	}

	int __stdcall MyFillRect(
		HDC hdc, 
		LPRECT rect,
		HBRUSH hbrush)
	{
		return g_funFillRect(hdc, rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}

	int MyDrawTextW(
		HDC hdc,
		LPCWSTR lpchText, 
		int cchText,
		LPRECT lprc,
		UINT format)
	{
		SetBkMode(hdc, TRANSPARENT);
		int result = 0;

		if ((format & DT_CALCRECT) || (format & DT_INTERNAL) || (format & DT_NOCLIP))
		{
			//兼容第三方主题绘制发光字需要预留一些区域
			if(format & DT_CALCRECT)
			{
				auto ret = g_funDrawTextW(hdc, lpchText, cchText, lprc, format);
				lprc->right += 30;
				lprc->bottom += 20;
				lprc->top = -5;
				lprc->left = -10;
				return ret + 20;
			}
			return g_funDrawTextW(hdc, lpchText, cchText, lprc, format);
		}
		/*if (FAILED(DrawTextWithAlpha(
			hdc,
			lpchText,
			cchText,
			lprc,
			DT_LEFT | DT_TOP,
			result
		)))
		{
			return g_funDrawTextW(hdc, lpchText, cchText, lprc, format);
		}*/
		auto drawTextCallback = [](HDC hdc, LPWSTR pszText, int cchText, LPRECT prc, UINT dwFlags, LPARAM lParam) -> int
		{
			return *reinterpret_cast<int*>(lParam) = DrawTextW(hdc, pszText, cchText, prc, dwFlags);
		};
		DTTOPTS Options =
		{
			sizeof(DTTOPTS),
			DTT_TEXTCOLOR | DTT_CALLBACK | DTT_COMPOSITED | DTT_GLOWSIZE,
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

		auto clean = RAIIHelper::scope_exit([&] { CloseThemeData(hTheme); });

		if (!hTheme)
			return g_funDrawTextW(hdc, lpchText, cchText, lprc, format);

		RECT offset = *lprc;
		offset.left += 20;
		offset.top += 10;
		if(FAILED(DrawThemeTextEx(hTheme, hdc, 0, 0, lpchText, cchText, DT_LEFT | DT_TOP, &offset, &Options)))
			return g_funDrawTextW(hdc, lpchText, cchText, lprc, format);
		return 1;
	}

	HRESULT __stdcall MyCreateBitmapFromHBITMAP(
		IWICImagingFactory2* This,
		HBITMAP hBitmap,
		HPALETTE hPalette,
		WICBitmapAlphaChannelOption options,
		IWICBitmap** ppIBitmap)
	{
		options = WICBitmapUsePremultipliedAlpha;
		return g_funCreateBitmapFromHBITMAP(This, hBitmap, hPalette, options, ppIBitmap);
	}

	DWORD64 __stdcall CTopLevelWindow_UpdateNCAreaGeometry(CTopLevelWindow* This)
	{
		if (g_window)
		{
			RECT* pRect = (RECT*)(*((DWORD64*)This + 91) + 48);
			DWORD* extendLeft = (DWORD*)This + 153;
			DWORD* extendRight = (DWORD*)This + 154;
			DWORD* extendTop = (DWORD*)This + 155;

			if (g_configData.extendBorder)
			{
				*extendLeft = 0;
				*extendRight = 0;
				*extendTop = pRect->bottom - pRect->top;
			}
			else
			{
				RECT rect;
				g_funCWindowList_GetExtendedFrameBounds(g_windowList, g_window, &rect);
				*extendTop = rect.bottom - rect.top;
			}
		}
		return g_funCTopLevelWindow_UpdateNCAreaGeometry.call_org(This);
	}

	HRGN __stdcall MyCreateRoundRectRgn(int x1, int y1, int x2, int y2, int w, int h)
	{
		if(g_window && g_configData.extendBorder)
		{
			w = h = g_configData.extendRound;
		}
		return g_funCreateRoundRgn(x1, y1, x2, y2, w, h);
	}

	HBITMAP MyCreateBitmap(int nWidth, int nHeight, UINT nPlanes, UINT nBitCount, const void* lpBits)
	{
		if (nWidth == 1 && nHeight == 1)
			return g_funCreateBitmap(nWidth, nHeight, nPlanes, nBitCount, lpBits);

		BITMAPINFOHEADER bmih = { 0 };
		bmih.biSize = sizeof(BITMAPINFOHEADER);
		bmih.biWidth = nWidth;
		bmih.biHeight = -nHeight;
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

	DWORD64 __stdcall HrgnFromRects(const tagRECT* Src, unsigned int a2, HRGN* a3)
	{
		if (g_window && g_configData.extendBorder)
		{
			*a3 = MyCreateRoundRectRgn(Src->left, Src->top, Src->right, Src->bottom, 8, 8);
			return 0;
		}
		return g_funHrgnFromRects.call_org(Src, a2, a3);
	}

	void __stdcall CAccent_UpdateAccentBlurRect(
		CAccent* This,
		const RECT* a2)
	{
		g_accentThis = This;
		g_blurRegion = *a2;
		return g_funCAccent_UpdateAccentBlurRect.call_org(This, a2);
	}

	HRESULT __stdcall CAccent_UpdateAccentPolicy(
		CAccent* This,
		RECT* a2,
		ACCENT_POLICY* a3, 
		CBaseGeometryProxy* a4)
	{
		thread_local auto redirected{ false };
		thread_local RECT windowRect = { -1 };

		if (!redirected && GetWindowLongPtrW(g_window, GWL_STYLE) & WS_CAPTION)
		{
			g_funCWindowList_GetExtendedFrameBounds(g_windowList, g_window, &windowRect);

			redirected = true;
			HRESULT hr = g_funCAccent_UpdateAccentPolicy.call_org(This, a2, a3, a4);

			if (!IsZoomed(g_window) && !IsIconic(g_window))
			{
				OffsetRect(&windowRect, -windowRect.left + (windowRect.left - a2->left), -windowRect.top);

				if (g_accentThis == This)
				{
					IntersectRect(&windowRect, &g_blurRegion, &windowRect);
					g_accentThis = nullptr;
					g_blurRegion = { 0, 0, -1, -1 };
				}
				g_funCAccent_UpdateAccentBlurRect.call_org(This, &windowRect);
			}
			else
			{
				windowRect = { -1 };
				if (g_accentThis == This)
				{
					g_accentThis = nullptr;
					windowRect = g_blurRegion;
				}
				g_funCAccent_UpdateAccentBlurRect.call_org(This, &windowRect);
			}
			redirected = false;
			g_window = nullptr;

			return hr;
		}

		return g_funCAccent_UpdateAccentPolicy.call_org(This, a2, a3, a4);
	}

	HRESULT __stdcall CTopLevelWindow_ValidateVisual(CTopLevelWindow* This)
	{
		PVOID windowData;
		ACCENT_POLICY* accent_policy;

		if (os::buildNumber >= 22000)
		{
			windowData = *((PVOID*)This + 94);
			accent_policy = (ACCENT_POLICY*)((ULONGLONG)windowData + 168);
		}
		else
		{
			if(os::buildNumber > 18363)
				windowData = *((PVOID*)This + 91);
			else
				windowData = *((PVOID*)This + 90);

			accent_policy = (ACCENT_POLICY*)((ULONGLONG)windowData + 152);
		}

		bool needUpdate{ false };
		if (windowData)
		{
			HWND hWnd = *((HWND*)windowData + 5);
			g_window = hWnd;

			//扩展到非客户区的窗口才有效
			BOOL enableNC = FALSE;
			DwmGetWindowAttribute(hWnd, DWMWA_NCRENDERING_ENABLED, &enableNC, sizeof enableNC);

			//忽略已有Backdrop效果的窗口
			DWM_SYSTEMBACKDROP_TYPE type = DWMSBT_NONE;
			if (os::buildNumber >= 22621)
			{
				DwmGetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &type, sizeof type);
			}
			if (g_configData.applyglobal && (type == DWMSBT_MAINWINDOW || type == DWMSBT_TABBEDWINDOW))
				type = DWMSBT_NONE;

			if (enableNC && (type == DWMSBT_NONE || type == DWMSBT_AUTO))
			{
				if (accent_policy->nAccentState == 0)
				{
					accent_policy->nAccentState = 3;
					accent_policy->nFlags = 3584;
					needUpdate = true;
				}
				//对于HostBackdrop 启用之后就可以更改flag了 否则会造成Backdrop不刷新的问题
				else if (accent_policy->nAccentState == 5)
				{
					needUpdate = true;
					accent_policy->nAccentState = 3;
					accent_policy->nFlags = 3584;
				}
				//DrawText处给发光字预留了空间 需要刷新文本布局
				if (os::buildNumber < 22000)
				{
					if (CText* textThis = *((CText**)This + 65))
					{
						wchar_t* wndText = (wchar_t*)*((DWORD64*)textThis + 36);
						if (wndText)
							g_funCText_SetText(textThis, wndText);
					}
				}
			}
		}

		HRESULT hr = g_funCTopLevelWindow_ValidateVisual.call_org(This);
		//手动通知
		if (needUpdate)
		{
			g_funCTopLevelWindow_OnAccentPolicyUpdated(This);
		}

		g_window = nullptr;
		return hr;
	}

	DWORD64 __stdcall CTopLevelWindow_UpdateWindowVisuals(CTopLevelWindow* This)
	{
		auto frame = g_funCTopLevelWindow_s_ChooseWindowFrameFromStyle
		(
			(char)*((DWORD*)This + 148),
			0,
			(char)(*(BYTE*)(*((ULONG64*)This + 91) + 611) & 0x20) != 0
		);

		//对于Ribbon或扩展到NC区域的窗口 我们无法处理标题文本颜色 需要过滤掉 它应该由应用程序处理
		if (frame && (*((BYTE*)This + 592) & 8) != 0)
		{
			auto ret = g_funCTopLevelWindow_UpdateWindowVisuals.call_org(This);

			auto TreatAsActiveWindow = [&]
			{
				return (*((BYTE*)This + 592) & 0x40) != 0 || (*(BYTE*)(*((ULONG64*)This + 91) + 611) & 0x20) != 0;
			};

			auto text = *((CText**)This + 65);//CText This

			if (TreatAsActiveWindow())
				g_CText_SetColor(text, g_configData.activeTextColor);
			else
				g_CText_SetColor(text, g_configData.inactiveTextColor);

			return ret;
		}

		return g_funCTopLevelWindow_UpdateWindowVisuals.call_org(This);
	}

	DWORD64 CText_SetSize(CText* This, SIZE* a2)
	{
		SIZE size = *a2;
		size.cy += 10;
		return g_funCText_SetSize.call_org(This, &size);
	}

	DWORD64 __stdcall CGlassColorizationParameters_AdjustWindowColorization(
		GpCC* a1,
		float a2, 
		TMILFlagsEnum<ColorizationFlags> a3)
	{
		auto ret = g_funCGlassColorizationParameters_AdjustWindowColorization.call_org(a1, a2, a3);
		if (a3 == Color_TitleBackground)
		{
			auto color = g_configData.inactiveBlendColor;
			if (g_window && GetForegroundWindow() == g_window)
				color = g_configData.activeBlendColor;

			if (os::buildNumber >= 22000)
				a1->a = (color >> 24) & 0xff;
			else
				a1->a = 255;
			a1->r = GetRValue(color);
			a1->g = GetGValue(color);
			a1->b = GetBValue(color);
		}
		return ret;
	}

	DWORD64 __stdcall CTopLevelWindow_UpdateText(
		CTopLevelWindow* This,
		CTopLevelWindow_WindowFrame* a2,
		double a3)
	{
		if (a2 && (*((BYTE*)This + 624) & 8) != 0)
		{
			auto ret = g_funCTopLevelWindow_UpdateText.call_org(This, a2, a3);

			auto text = (CDWriteText*)*((ULONG64*)This + 71);//CDWriteText This
			typedef void(__fastcall**** CDWriteText_SetColor)(CDWriteText*, UINT);//(***((void(__fastcall****)(ULONG64, __int64))This + 71))(*((ULONG64*)This + 71), inputVec);
			auto pfunCDWriteText_SetColor = ***((CDWriteText_SetColor)This + 71);//CDWriteText::SetColor(CDWriteText *this, int a2)

			auto CTopLevelWindow_TreatAsActiveWindow = [&]() -> bool
			{
				return (*((BYTE*)This + 624) & 0x40) != 0 || (*(BYTE*)(*((ULONG64*)This + 94) + 675) & 0x10) != 0;
			};

			if (CTopLevelWindow_TreatAsActiveWindow())
				pfunCDWriteText_SetColor(text, g_configData.activeTextColor);//Active
			else
				pfunCDWriteText_SetColor(text, g_configData.inactiveTextColor);//Inactive

			return ret;
		}

		return g_funCTopLevelWindow_UpdateText.call_org(This, a2, a3);
	}

	DWORD64 CWindowList_BlurBehindChange(
		CWindowList* This,
		IDwmWindow* a2,
		const DWM_BLURBEHIND* a3)
	{
		auto ret = g_funCWindowList_BlurBehindChange.call_org(This, a2, a3);
		if(a3->hRgnBlur)
		{
			RECT rect;
			GetRgnBox(a3->hRgnBlur, &rect);
			if (rect.right - rect.left <= 1 || rect.bottom - rect.top <= 1)
				return ret;
		}

		struct WINDOWCOMPOSITIONATTRIBUTEDATA
		{
			DWORD dwAttribute;
			PVOID pvData;
			SIZE_T cbData;
		};
		static auto SetWindowCompositionAttribute = reinterpret_cast<BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBUTEDATA*)>(
			GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"));

		CWindowData* windowData = nullptr;
		g_funCWindowList_GetSyncedWindowData(This, a2, 0, &windowData);

		if (!windowData)
			return ret;

		HWND hWnd = *((HWND*)windowData + 5);

		if (os::buildNumber >= 22621)
		{
			DWM_SYSTEMBACKDROP_TYPE type = a3->fEnable ? DWMSBT_TRANSIENTWINDOW : DWMSBT_NONE;
			DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &type, sizeof type);
		}
		else
		{
			ACCENT_POLICY policy = {};
			WINDOWCOMPOSITIONATTRIBUTEDATA data =
			{
				19,
				&policy,
				sizeof(policy)
			};
			policy.nAccentState = a3->fEnable ? 3 : 0;
			policy.nFlags = 0;
			SetWindowCompositionAttribute(hWnd, &data);
		}

		return ret;
	}

	DWORD64 CButton_UpdateLayout(CButton* This)
	{
		if (g_configData.oldBtnHeight)
		{
			SIZE* size;
			UINT dpi = GetDpiForWindow(GetForegroundWindow());
			if (dpi == 0)
				dpi = GetDpiForSystem();
			float scale = (float)dpi / 96.f;
			int cap = int(22.f * scale);
			if (os::buildNumber < 22000)
			{
				size = (SIZE*)This + 15;
				size->cy = cap;
			}
			else
			{
				size = (SIZE*)This + 16;
				size->cy = cap;
			}
		}
		return g_funCButton_UpdateLayout.call_org(This);
	}
}
