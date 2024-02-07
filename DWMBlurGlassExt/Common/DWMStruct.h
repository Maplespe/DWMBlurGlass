/**
 * FileName: DWMStruct.h
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
#include "../Helper/HookHelper.h"
#include "../Helper/Helper.h"
#include "DcompPrivate.h"

namespace MDWMBlurGlassExt::DWM
{
	struct CBaseObject
	{
		void* operator new(size_t size);

		void operator delete(void* ptr);

		size_t AddRef();

		size_t Release();

		HRESULT QueryInterface(REFIID riid, PVOID* ppvObject);

		template <typename T, typename... Args>
		static HRESULT Create(T** object, Args&&... args) try
		{
			winrt::com_ptr<T> allocatedObject{ new T(std::forward<Args>(args)...), winrt::take_ownership_from_abi };
			winrt::check_hresult(allocatedObject->Initialize());

			*object = allocatedObject.detach();
			return S_OK;
		}
		catch (...) { return winrt::to_hresult(); }

	protected:
		CBaseObject() { *(reinterpret_cast<DWORD*>(this) + 2) = 1; }
		virtual ~CBaseObject() = default;
	};

	struct CSolidColorLegacyMilBrushProxy {};
	struct CDrawingContext {};
	struct CDrawListEntryBuilder {};
	struct MilRectF
	{
		FLOAT left;
		FLOAT top;
		FLOAT right;
		FLOAT bottom;
	};

	enum ACCENT_STATE : DWORD
	{
		ACCENT_DISABLED,
		ACCENT_ENABLE_GRADIENT,
		ACCENT_ENABLE_TRANSPARENTGRADIENT,
		ACCENT_ENABLE_BLURBEHIND,	// Removed in Windows 11 22H2+
		ACCENT_ENABLE_ACRYLICBLURBEHIND,
		ACCENT_ENABLE_HOSTBACKDROP,
		ACCENT_INVALID_STATE
	};

	struct ACCENT_POLICY
	{
		ACCENT_STATE nAccentState;
		int32_t	nFlags;
		uint32_t nColor;
		int32_t	nAnimationId;
	};

	struct CWindowData : CBaseObject
	{
		HWND GetHWND() const;
		bool IsUsingDarkMode() const;
		DWORD GetNonClientAttribute() const;
		bool IsImmersiveWindow() const;

		ACCENT_POLICY* GetAccentPolicy();
	};

	struct VisualCollection;
	struct CBaseGeometryProxy : CBaseObject {};
	struct CRgnGeometryProxy : CBaseGeometryProxy {};

	struct MilMatrix3x2D
	{
		DOUBLE S_11;
		DOUBLE S_12;
		DOUBLE S_21;
		DOUBLE S_22;
		DOUBLE DX;
		DOUBLE DY;
	};

	struct CMatrixTransformProxy
	{
		HRESULT Update(const MilMatrix3x2D* matrix);
	};

	struct CVisualProxy : CBaseObject
	{
		HRESULT SetClip(CBaseGeometryProxy* geometry);
		HRESULT SetSize(double width, double height);
	};

	struct CVisual : CBaseObject
	{
		static HRESULT Create(CVisual** pVisual);

		VisualCollection* GetVisualCollection() const;

		CVisualProxy* GetVisualProxy() const;

		bool AllowVisualTreeClone(bool allow);

		void SetInsetFromParent(MARGINS* margins);

		void SetInsetFromParentTop(int top);

		HRESULT SetSize(const SIZE& size);

		void SetOffset(const POINT& pt);

		HRESULT InitializeVisualTreeClone(CBaseObject* baseObject, UINT cloneOptions);

		void Unhide();

		void Hide();

		void ConnectToParent(bool connect);

		void SetOpacity(double opacity);

		void SetScale(double x, double y);

		HRESULT SendSetOpacity(double opacity);

		HRESULT RenderRecursive();

		void SetDirtyChildren();

		static HRESULT CreateFromSharedHandle(HANDLE handle, CVisual** visual);

		HRESULT InitializeFromSharedHandle(HANDLE handle);

		HRESULT MoveToFront(bool unknown);

		HRESULT Initialize();
	};

	struct CContainerVisual : CBaseObject
	{
		static HRESULT Create(CVisual** pVisual);
	};

	struct CVisualTree {};

	struct VisualCollection
	{
		HRESULT RemoveAll();
		HRESULT Remove(CVisual* visual);
		HRESULT InsertRelative(CVisual* visual, CVisual* reference, bool insterAfter, bool updateNow);
	};

	struct CText : CVisual
	{
		void SetColor(COLORREF color);
		HRESULT SetText(LPCWSTR text);
		HRESULT SetSize(SIZE* size);
		LPCWSTR GetText();

		CMatrixTransformProxy* GetMatrixProxy();
	};

	struct CDWriteText {};

	struct IDwmWindow {};

	struct CWindowList
	{
		HRESULT GetExtendedFrameBounds(HWND hWnd, RECT* rect);
		HRESULT GetSyncedWindowData(IDwmWindow* dwmWindow, bool shared, CWindowData** pWindowData);
	};

	struct CTopLevelWindow : CVisual
	{
		struct WindowFrame {};

		CWindowData* GetData();
		VisualCollection* GetNCAreaVisualCollection();
		CVisual* GetVisual() const;
		CVisual* GetNCAreaBackgroundVisual() const;
		CVisual* GetClientBlurVisual() const;
		std::vector<winrt::com_ptr<CVisual>> GetNCBackgroundVisualList() const;
		void ShowNCBackgroundVisualList(bool show);

		CRgnGeometryProxy* const& GetBorderGeometry() const;
		CRgnGeometryProxy* const& GetTitlebarGeometry() const;

		bool HasNonClientBackground();

		bool IsSystemBackdropApplied();

		//仅限Windows 11 22H2之前
		CText* GetCText();
		//Windows 11 22H2+
		void CDWriteTextSetColor(COLORREF color);

		bool TreatAsActiveWindow();

		RECT* GetActualWindowRect(RECT* rect, char eraseOffset, char includeNonClient, bool excludeBorderMargins);

		void GetBorderMargins(MARGINS* margins) const;

		//Windows 10
		static WindowFrame* s_ChooseWindowFrameFromStyle(char a1, bool a2, bool a3);
	};

	struct CImageSource {};
	struct CRenderingTechnique {};
	struct EffectInput {};
	struct ID2DContextOwner {};
	struct CCustomBlur
	{
		ID2D1Effect* CropEffect();
		ID2D1Effect* ScaleEffect();
		ID2D1Effect* DirBlurKernelXEffect();
		ID2D1Effect* DirBlurKernelYEffect();
	};

	struct CAccent
	{
		static bool s_IsPolicyActive(const ACCENT_POLICY* accentPolicy);
	};

	struct COcclusionContext {};
	struct CFilterEffect {};

	struct CButton
	{
		POINT* GetPoint();
		SIZE* GetSize();
	};

	struct CDesktopManager
	{
		inline static CDesktopManager* s_pDesktopManagerInstance{ nullptr };

		CWindowList* GetWindowList() const;

		IWICImagingFactory2* GetWICFactory() const;

		ID2D1Device* GetD2DDevice() const;

		DCompPrivate::IDCompositionDesktopDevicePartner* GetDCompositionInteropDevice() const;

		UINT MonitorDpiFromPoint(POINT pt) const;
	};

	union GpCC
	{
		struct {
			BYTE b;
			BYTE g;
			BYTE r;
			BYTE a;
		};
		UINT32 argb;
	};

	template<class E>
	struct TMILFlagsEnum
	{
		E flags;

		TMILFlagsEnum() { }
		TMILFlagsEnum(const E& _Right) { flags = _Right; }
		TMILFlagsEnum(const int& _Right) { flags = static_cast<E>(_Right); }

		operator const E& () const { return flags; }

		TMILFlagsEnum& operator|=(const int& _Right)
		{
			flags = static_cast<E>(flags | _Right);
			return *this;
		}

		TMILFlagsEnum& operator&=(const int& _Right)
		{
			flags = static_cast<E>(flags & _Right);
			return *this;
		}

		TMILFlagsEnum& operator^=(const int& _Right)
		{
			flags = static_cast<E>(flags ^ _Right);
			return *this;
		}
	};

	enum ColorizationFlags
	{
		Color_TitleBackground_ActiveDark = -115,
		Color_TitleBackground_InactiveDark,
		Color_TitleBackground_Active_SWCA = 9,
		Color_TitleBackground_Inactive_SWCA,
		Color_TitleBackground_Active = 13,
		Color_TitleBackground_Inactive,
		Color_TitleBackground_ActiveDark_Win11 = 141,
		Color_TitleBackground_InactiveDark_Win11,
		Color_TitleBackground_ActiveBackDrop = 269,
		Color_TitleBackground_InactiveBackDrop,
		Color_TitleBackground_ActiveBackDropMica = 333,
		Color_TitleBackground_InactiveBackDropMica,
		Color_TitleBackground_ActiveBackDropDark = 397,
		Color_TitleBackground_InactiveBackDropDark,
		Color_TitleBackground_ActiveBackDropMicaDark = 461,
		Color_TitleBackground_InactiveBackDropMicaDark,
	};

	struct ResourceHelper
	{
		static HRESULT CreateGeometryFromHRGN(HRGN hrgn, CRgnGeometryProxy** geometry);
	};
}