/**
 * FileName: DWMStruct.cpp
 *
 * Copyright (C) 2024 Maplespe、ALTaleX531
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
#include "DWMStruct.h"

#pragma push_macro("DEFCALL_MHOST_METHOD")
#define DEFCALL_MHOST_METHOD(name, ...) (this->*class_method_cast<decltype(&name)>(#name))(__VA_ARGS__);

namespace MDWMBlurGlassExt::DWM
{
	using namespace MDWMBlurGlass;

	inline HANDLE g_hProcessHeap{ GetProcessHeap() };

	HRESULT Core::CVisual::GetVisualTree(CVisualTree** visualTree, bool value) const
	{
		return DEFCALL_MHOST_METHOD(CVisual::GetVisualTree, visualTree, value);
	}

	const D2D1_RECT_F& Core::CVisual::GetBounds(CVisualTree* visualTree) const
	{
		return DEFCALL_MHOST_METHOD(CVisual::GetBounds, visualTree);
	}

	HWND Core::CVisual::GetHwnd() const
	{
		return DEFCALL_MHOST_METHOD(CVisual::GetHwnd);
	}

	HWND Core::CVisual::GetTopLevelWindow() const
	{
		return DEFCALL_MHOST_METHOD(CVisual::GetTopLevelWindow);
	}

	HRESULT Core::CZOrderedRect::UpdateDeviceRect(const MilMatrix3x2D* matrix)
	{
		return DEFCALL_MHOST_METHOD(CZOrderedRect::UpdateDeviceRect, matrix);
	}

	HRESULT Core::CArrayBasedCoverageSet::Add(const D2D1_RECT_F& lprc, int depth, const MilMatrix3x2D* matrix)
	{
		return DEFCALL_MHOST_METHOD(CArrayBasedCoverageSet::Add, lprc, depth, matrix);
	}

	Core::DynArray<Core::CZOrderedRect>* Core::CArrayBasedCoverageSet::GetAntiOccluderArray() const
	{
		DynArray<CZOrderedRect>* array{ nullptr };
		if (os::buildNumber < 19041)
		{
			array = reinterpret_cast<DynArray<CZOrderedRect>*>(const_cast<CArrayBasedCoverageSet*>(this + 52));
		}
		else
		{
			array = reinterpret_cast<DynArray<CZOrderedRect>*>(const_cast<CArrayBasedCoverageSet*>(this + 49));
		}

		return array;
	}

	Core::CVisual* Core::COcclusionContext::GetVisual() const
	{
		CVisual* visual{ nullptr };

		if (os::buildNumber < 19041)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[6];
		}
		else
		{
			visual = reinterpret_cast<CVisual* const*>(this)[1];
		}

		return visual;
	}

	HRESULT Core::COcclusionContext::PostSubgraph(CVisualTree* visualTree, bool* unknown)
	{
		return DEFCALL_MHOST_METHOD(COcclusionContext::PostSubgraph, visualTree, unknown);
	}

	HRESULT Core::CD2DContext::FillEffect(const ID2DContextOwner* contextOwner, ID2D1Effect* effect,
		const D2D_RECT_F* lprc, const D2D_POINT_2F* point, D2D1_INTERPOLATION_MODE interpolationMode,
		D2D1_COMPOSITE_MODE compositeMode)
	{
		return DEFCALL_MHOST_METHOD(CD2DContext::FillEffect, contextOwner, effect, lprc, point, interpolationMode, compositeMode);
	}

	Core::CD2DContext* Core::CDrawingContext::GetD2DContext() const
	{
		if (os::buildNumber < 19041)
		{
			return reinterpret_cast<CD2DContext* const*>(this)[48];
		}
		return reinterpret_cast<CD2DContext*>(reinterpret_cast<ULONG_PTR const*>(this)[5] + 16);
	}

	Core::ID2DContextOwner* Core::CDrawingContext::GetD2DContextOwner() const
	{
		if (os::buildNumber < 19041)
		{
			return reinterpret_cast<ID2DContextOwner* const*>(this)[8];
		}
		return reinterpret_cast<ID2DContextOwner*>(reinterpret_cast<ULONG_PTR const>(this) + 24);
	}

	bool Core::CDrawingContext::IsOccluded(const D2D1_RECT_F& lprc, int flag) const
	{
		return DEFCALL_MHOST_METHOD(CDrawingContext::IsOccluded, lprc, flag);
	}

	Core::CVisual* Core::CDrawingContext::GetCurrentVisual() const
	{
		return DEFCALL_MHOST_METHOD(CDrawingContext::GetCurrentVisual);
	}

	HRESULT Core::CDrawingContext::GetClipBoundsWorld(D2D1_RECT_F& lprc) const
	{
		return DEFCALL_MHOST_METHOD(CDrawingContext::GetClipBoundsWorld, lprc);
	}

	void Core::CCustomBlur::Reset()
	{
		DEFCALL_MHOST_METHOD(CCustomBlur::Reset);
	}

	HRESULT Core::CCustomBlur::Create(ID2D1DeviceContext* deviceContext, CCustomBlur** customBlur)
	{
		static auto pfun = MHostGetProcAddress<decltype(Create)>("CCustomBlur::Create");
		return pfun(deviceContext, customBlur);
	}

	float Core::CCustomBlur::DetermineOutputScale(float size, float blurAmount,
		D2D1_GAUSSIANBLUR_OPTIMIZATION optimization)
	{
		static auto pfun = MHostGetProcAddress<decltype(DetermineOutputScale)>("CCustomBlur::DetermineOutputScale");
		return pfun(size, blurAmount, optimization);
	}

	ULONGLONG Core::GetCurrentFrameId()
	{
		static auto pfun = MHostGetProcAddress<decltype(GetCurrentFrameId)>("GetCurrentFrameId");
		return pfun();
	}

	void* CBaseObject::operator new(size_t size)
	{
		return HeapAlloc(g_hProcessHeap, 0, size);
	}

	void CBaseObject::operator delete(void* ptr)
	{
		if (ptr)
			HeapFree(g_hProcessHeap, 0, ptr);
	}

	size_t CBaseObject::AddRef()
	{
		return InterlockedIncrement(reinterpret_cast<DWORD*>(this) + 2);
	}

	size_t CBaseObject::Release()
	{
		auto result{ InterlockedDecrement(reinterpret_cast<DWORD*>(this) + 2) };
		if (!result)
		{
			delete this;
		}
		return result;
	}

	HRESULT CBaseObject::QueryInterface(const IID& riid, PVOID* ppvObject)
	{
		*ppvObject = this;
		return S_OK;
	}

	HWND CWindowData::GetHWND() const
	{
		const HWND hWnd = *((HWND*)this + 5);
		return hWnd;
	}
	CTopLevelWindow* CWindowData::GetWindow() const
	{
		CTopLevelWindow* window{ nullptr };

		if (os::buildNumber < 22000)
		{
			window = reinterpret_cast<CTopLevelWindow* const*>(this)[48];
		}
		else
		{
			window = reinterpret_cast<CTopLevelWindow* const*>(this)[55];
		}
		return window;
	}

	bool CWindowData::IsUsingDarkMode() const
	{
		bool darkMode = false;

		if (os::buildNumber < 19041)
		{
			// TO-DO
		}
		else if (os::buildNumber < 22000)
		{
			darkMode = (reinterpret_cast<BYTE const*>(this)[613] & 8) != 0;
		}
		else if (os::buildNumber < 22621)
		{
			darkMode = (reinterpret_cast<BYTE const*>(this)[669] & 4) != 0;
		}
		else
		{
			darkMode = (reinterpret_cast<BYTE const*>(this)[677] & 4) != 0; // ok with build 26020
		}

		return darkMode;
	}

	DWORD CWindowData::GetNonClientAttribute() const
	{
		DWORD attribute{ 0 };

		if (os::buildNumber < 19041)
		{
			// TO-DO
		}
		else if (os::buildNumber < 22000)
		{
			attribute = *reinterpret_cast<const DWORD*>(reinterpret_cast<BYTE const*>(this) + 608);
		}
		else if (os::buildNumber < 22621)
		{
			attribute = *reinterpret_cast<const DWORD*>(reinterpret_cast<BYTE const*>(this) + 664);
		}
		else
		{
			attribute = *reinterpret_cast<const DWORD*>(reinterpret_cast<BYTE const*>(this) + 672); // ok with build 26020
		}

		return attribute;
	}

	bool CWindowData::IsImmersiveWindow() const
	{
		return DEFCALL_MHOST_METHOD(CWindowData::IsImmersiveWindow);
	}

	bool CWindowData::IsWindowVisibleAndUncloaked() const
	{
		return DEFCALL_MHOST_METHOD(CWindowData::IsWindowVisibleAndUncloaked);
	}

	ACCENT_POLICY* CWindowData::GetAccentPolicy()
	{
		ACCENT_POLICY* accentpolicy;
		if (os::buildNumber >= 22000)
			accentpolicy = (ACCENT_POLICY*)((ULONG64)this + 168);
		else
			accentpolicy = (ACCENT_POLICY*)((ULONG64)this + 152);
		return accentpolicy;
	}

	const MARGINS* CWindowData::GetExtendedFrameMargins() const
	{
		const MARGINS* margins{ nullptr };

		if (os::buildNumber < 22000)
		{
			margins = reinterpret_cast<const MARGINS*>(reinterpret_cast<ULONG_PTR>(this) + 80);
		}
		else
		{
			margins = reinterpret_cast<const MARGINS*>(reinterpret_cast<ULONG_PTR>(this) + 96);
		}

		return margins;
	}

	bool CWindowData::IsFrameExtendedIntoClientAreaLRB() const
	{
		const MARGINS* margins{ GetExtendedFrameMargins() };

		return margins->cxLeftWidth || margins->cxRightWidth || margins->cyBottomHeight;
	}

	HRESULT CMatrixTransformProxy::Update(const MilMatrix3x2D* matrix)
	{
		return DEFCALL_MHOST_METHOD(CMatrixTransformProxy::Update, matrix);
	}

	HRESULT CVisualProxy::SetClip(CBaseGeometryProxy* geometry)
	{
		return DEFCALL_MHOST_METHOD(CVisualProxy::SetClip, geometry);
	}

	HRESULT CVisualProxy::SetSize(double width, double height)
	{
		return DEFCALL_MHOST_METHOD(CVisualProxy::SetSize, width, height);
	}

	HRESULT CContainerVisual::Create(CVisual** pVisual)
	{
		static auto pfun = MHostGetProcAddress<decltype(Create)>("CContainerVisual::Create");
		return pfun(pVisual);
	}

	HRESULT CVisual::Create(CVisual** pVisual)
	{
		static auto pfun = MHostGetProcAddress<decltype(Create)>("CVisual::Create");
		return pfun(pVisual);
	}

	VisualCollection* CVisual::GetVisualCollection() const
	{
		if (os::buildNumber < 26020)
		{
			return const_cast<VisualCollection*>(reinterpret_cast<VisualCollection const*>(reinterpret_cast<const char*>(this) + 32));
		}
		return const_cast<VisualCollection*>(reinterpret_cast<VisualCollection const*>(reinterpret_cast<const char*>(this) + 144));
	}

	CVisualProxy* CVisual::GetVisualProxy() const
	{
		return reinterpret_cast<CVisualProxy*>(reinterpret_cast<const ULONG_PTR*>(this)[2]);
	}

	bool CVisual::IsCloneAllowed() const
	{
		const BYTE* properties{ nullptr };

		if (os::buildNumber < 22000)
		{
			properties = &reinterpret_cast<BYTE const*>(this)[84];
		}
		else if (os::buildNumber < 26020)
		{
			properties = &reinterpret_cast<BYTE const*>(this)[92];
		}
		else
		{
			properties = &reinterpret_cast<BYTE const*>(this)[36];
		}

		bool allowed{ false };
		if (properties)
		{
			allowed = (*properties & 8) == 0;
		}

		return allowed;
	}
	bool CVisual::AllowVisualTreeClone(bool allow)
	{
		BYTE* properties;

		if (os::buildNumber < 22000)
		{
			properties = &reinterpret_cast<BYTE*>(this)[84];
		}
		else if (os::buildNumber < 26020)
		{
			properties = &reinterpret_cast<BYTE*>(this)[92];
		}
		else
		{
			properties = &reinterpret_cast<BYTE*>(this)[36];
		}

		bool allowed{ false };
		if (properties)
		{
			allowed = (*properties & 8) == 0;
			if (allow)
			{
				*properties = *properties & ~8;
			}
			else
			{
				*properties |= 8;
			}
		}

		return allowed;
	}
	void CVisual::Cloak(bool cloak)
	{
		if (!cloak)
		{
			SetOpacity(1.);
			UpdateOpacity();
		}
		else
		{
			SetOpacity(0.);
			UpdateOpacity();
		}
	}

	void CVisual::SetInsetFromParent(MARGINS* margins)
	{
		DEFCALL_MHOST_METHOD(CVisual::SetInsetFromParent, margins);
	}

	void CVisual::SetInsetFromParentRight(int right)
	{
		DEFCALL_MHOST_METHOD(CVisual::SetInsetFromParentRight, right);
	}

	void CVisual::SetInsetFromParentLeft(int left)
	{
		DEFCALL_MHOST_METHOD(CVisual::SetInsetFromParentLeft, left);
	}

	HRESULT CVisual::SetSize(const SIZE& size)
	{
		return DEFCALL_MHOST_METHOD(CVisual::SetSize, size);
	}

	void CVisual::SetOffset(const POINT& pt)
	{
		DEFCALL_MHOST_METHOD(CVisual::SetOffset, pt);
	}

	HRESULT CVisual::InitializeVisualTreeClone(CBaseObject* baseObject, UINT cloneOptions)
	{
		return DEFCALL_MHOST_METHOD(CVisual::InitializeVisualTreeClone, baseObject, cloneOptions);
	}

	void CVisual::Unhide()
	{
		DEFCALL_MHOST_METHOD(CVisual::Unhide);
	}

	void CVisual::Hide()
	{
		DEFCALL_MHOST_METHOD(CVisual::Hide);
	}

	void CVisual::ConnectToParent(bool connect)
	{
		DEFCALL_MHOST_METHOD(CVisual::ConnectToParent, connect);
	}

	void CVisual::SetOpacity(double opacity)
	{
		DEFCALL_MHOST_METHOD(CVisual::SetOpacity, opacity);
	}
	HRESULT STDMETHODCALLTYPE CVisual::UpdateOpacity()
	{
		return DEFCALL_MHOST_METHOD(CVisual::UpdateOpacity, );
	}

	void CVisual::SetScale(double x, double y)
	{
		DEFCALL_MHOST_METHOD(CVisual::SetScale, x, y);
	}

	HRESULT CVisual::SendSetOpacity(double opacity)
	{
		return DEFCALL_MHOST_METHOD(CVisual::SendSetOpacity, opacity);
	}

	HRESULT CVisual::RenderRecursive()
	{
		return DEFCALL_MHOST_METHOD(CVisual::RenderRecursive);
	}

	void CVisual::SetDirtyChildren()
	{
		if (os::buildNumber < 26020)
		{
			return DEFCALL_MHOST_METHOD(CVisual::SetDirtyChildren);
		}
		return (this->*class_method_cast<decltype(&CVisual::SetDirtyChildren)>("CContainerVisual"))();
	}

	HRESULT CVisual::WrapExistingResource(UINT handleIndex, CVisual** visual)
	{
		static auto pfun = (HRESULT(*)(UINT, CVisual**))MHostGetProcAddress("CVisual::WrapExistingResource");
		return pfun(handleIndex, visual);
	}

	HRESULT CVisual::WrapExistingResource(Core::CChannel* channel, UINT handleIndex, CVisual** visual)
	{
		static auto pfun = (HRESULT(*)(Core::CChannel*, UINT, CVisual**))MHostGetProcAddress("CVisual::WrapExistingResource");
		return pfun(channel, handleIndex, visual);
	}

	HRESULT CVisual::CreateFromSharedHandle(HANDLE handle, CVisual** visual)
	{
		static auto pfun = MHostGetProcAddress<decltype(CreateFromSharedHandle)>("CVisual::CreateFromSharedHandle");
		return pfun(handle, visual);
	}

	HRESULT CVisual::InitializeFromSharedHandle(HANDLE handle)
	{
		return DEFCALL_MHOST_METHOD(CVisual::InitializeFromSharedHandle, handle);
	}

	HRESULT CVisual::MoveToFront(bool unknown)
	{
		return DEFCALL_MHOST_METHOD(CVisual::MoveToFront, unknown);
	}

	HRESULT CVisual::Initialize()
	{
		return DEFCALL_MHOST_METHOD(CVisual::Initialize);
	}

	LONG CVisual::GetWidth() const
	{
		LONG width{ 0 };

		if (os::buildNumber < 22000)
		{
			width = reinterpret_cast<LONG const*>(this)[30];
		}
		else
		{
			width = reinterpret_cast<LONG const*>(this)[32];
		}

		return width;
	}

	LONG CVisual::GetHeight() const
	{
		LONG height{ 0 };

		if (os::buildNumber < 22000)
		{
			height = reinterpret_cast<LONG const*>(this)[31];
		}
		else
		{
			height = reinterpret_cast<LONG const*>(this)[33];
		}

		return height;
	}

	MARGINS* CVisual::GetMargins()
	{
		MARGINS* margins{ nullptr };

		if (os::buildNumber < 22000)
		{
			margins = reinterpret_cast<MARGINS*>(this) + 8;
		}
		else
		{
			margins = reinterpret_cast<MARGINS*>(reinterpret_cast<ULONG_PTR>(this) + 136);
		}

		return margins;
	}

	HRESULT VisualCollection::RemoveAll()
	{
		return DEFCALL_MHOST_METHOD(VisualCollection::RemoveAll);
	}

	HRESULT VisualCollection::Remove(CVisual* visual)
	{
		return DEFCALL_MHOST_METHOD(VisualCollection::Remove, visual);
	}

	HRESULT VisualCollection::InsertRelative(CVisual* visual, CVisual* reference, bool insterAfter, bool updateNow)
	{
		return DEFCALL_MHOST_METHOD(VisualCollection::InsertRelative, visual, reference, insterAfter, updateNow);
	}

	CWindowData* CTopLevelWindow::GetData()
	{
		PVOID windowData;

		if (os::buildNumber >= 22000)
		{
			if (os::buildNumber < 26020)
				windowData = *((PVOID*)this + 94);
			else
				windowData = *((PVOID*)this + 89);
		}
		else
		{
			if (os::buildNumber > 18363)
				windowData = *((PVOID*)this + 91);
			else
				windowData = *((PVOID*)this + 90);
		}

		return (CWindowData*)windowData;
	}

	VisualCollection* CTopLevelWindow::GetNCAreaVisualCollection()
	{
		const auto collection = (VisualCollection*)(*((ULONG64*)this + 33) + 32);
		return collection;
	}

	CCanvasVisual* CTopLevelWindow::GetVisual() const
	{
		CCanvasVisual* visual{ nullptr };

		if (os::buildNumber < 19041)
		{
			visual = reinterpret_cast<CCanvasVisual* const*>(this)[32];
		}
		else if (os::buildNumber < 22000)
		{
			visual = reinterpret_cast<CCanvasVisual* const*>(this)[33];
		}
		else if (os::buildNumber < 22621)
		{
			visual = reinterpret_cast<CCanvasVisual* const*>(this)[34];
		}
		else if (os::buildNumber < 26020)
		{
			visual = reinterpret_cast<CCanvasVisual* const*>(this)[36];
		}
		else
		{
			visual = reinterpret_cast<CCanvasVisual* const*>(this)[31];
		}

		return visual;
	}

	CAccent* CTopLevelWindow::GetAccent() const
	{
		CAccent* accent{ nullptr };

		if (os::buildNumber < 22000)
		{
			accent = reinterpret_cast<CAccent* const*>(this)[34];
		}
		else if (os::buildNumber < 22621)
		{
			accent = reinterpret_cast<CAccent* const*>(this)[35];
		}
		else if (os::buildNumber < 26020)
		{
			accent = reinterpret_cast<CAccent* const*>(this)[37];
		}
		else
		{
			accent = reinterpret_cast<CAccent* const*>(this)[32];
		}

		return accent;
	}

	CCanvasVisual* CTopLevelWindow::GetLegacyVisual() const
	{
		CCanvasVisual* visual{ nullptr };

		if (os::buildNumber < 19041)
		{
			visual = reinterpret_cast<CCanvasVisual* const*>(this)[35];
		}
		else if (os::buildNumber < 22000)
		{
			visual = reinterpret_cast<CCanvasVisual* const*>(this)[36];
		}
		else if (os::buildNumber < 22621)
		{
			visual = reinterpret_cast<CCanvasVisual* const*>(this)[37];
		}
		else if (os::buildNumber < 26100)
		{
			visual = reinterpret_cast<CCanvasVisual* const*>(this)[39];
		}
		else
		{
			visual = reinterpret_cast<CCanvasVisual* const*>(this)[34];
		}

		return visual;
	}
	CVisual* CTopLevelWindow::GetClientBlurVisual() const
	{
		CVisual* visual{ nullptr };

		if (os::buildNumber < 22000)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[37];
		}
		else if (os::buildNumber < 22621)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[39];
		}
		else if (os::buildNumber < 26020)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[42];
		}
		else
		{
			visual = reinterpret_cast<CVisual* const*>(this)[37];
		}

		return visual;
	}
	CVisual* CTopLevelWindow::GetSystemBackdropVisual() const
	{
		CVisual* visual{ nullptr };

		if (os::buildNumber < 22000)
		{
		}
		else if (os::buildNumber < 22621)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[38];
		}
		else if (os::buildNumber < 26020)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[40];
		}
		else
		{
			visual = reinterpret_cast<CVisual* const*>(this)[35];
		}

		return visual;
	}
	CCanvasVisual* CTopLevelWindow::GetAccentColorVisual() const
	{
		CCanvasVisual* visual{ nullptr };

		if (os::buildNumber < 22000)
		{
		}
		else if (os::buildNumber < 22621)
		{
		}
		else if (os::buildNumber < 26020)
		{
			visual = reinterpret_cast<CCanvasVisual* const*>(this)[41];
		}
		else
		{
			visual = reinterpret_cast<CCanvasVisual* const*>(this)[36];
		}

		return visual;
	}

	CVisual* CTopLevelWindow::GetClientAreaContainerParentVisual() const
	{
		CVisual* visual{ nullptr };

		if (os::buildNumber < 19041)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[67];
		}
		else if (os::buildNumber < 22000)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[68];
		}
		else if (os::buildNumber < 22621)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[70];
		}
		else if (os::buildNumber < 26100)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[74];
		}
		else
		{
			visual = reinterpret_cast<CVisual* const*>(this)[69];
		}

		return visual;
	}

	std::vector<winrt::com_ptr<CVisual>> CTopLevelWindow::GetNCBackgroundVisuals() const
	{
		std::vector<winrt::com_ptr<CVisual>> visuals{};

		if (GetLegacyVisual())
		{
			visuals.emplace_back(nullptr).copy_from(GetLegacyVisual());
		}
		if (GetAccent())
		{
			visuals.emplace_back(nullptr).copy_from(GetAccent());
		}
		if (GetClientBlurVisual())
		{
			visuals.emplace_back(nullptr).copy_from(GetClientBlurVisual());
		}
		if (GetSystemBackdropVisual())
		{
			visuals.emplace_back(nullptr).copy_from(GetSystemBackdropVisual());
		}
		if (GetAccentColorVisual())
		{
			visuals.emplace_back(nullptr).copy_from(GetAccentColorVisual());
		}

		return visuals;
	}
	bool CTopLevelWindow::IsNCBackgroundVisualsCloneAllAllowed()
	{
		for (const auto& visual : GetNCBackgroundVisuals())
		{
			if (!visual->IsCloneAllowed())
			{
				return false;
			}
		}

		return true;
	}
	CSolidColorLegacyMilBrushProxy* const* CTopLevelWindow::GetBorderMilBrush() const
	{
		CSolidColorLegacyMilBrushProxy* const* brush{ nullptr };

		if (os::buildNumber < 19041)
		{
			// TO-DO
		}
		else if (os::buildNumber < 22000)
		{
			brush = &reinterpret_cast<CSolidColorLegacyMilBrushProxy* const*>(this)[94];
		}
		else if (os::buildNumber < 22621)
		{
			brush = &reinterpret_cast<CSolidColorLegacyMilBrushProxy* const*>(this)[98];
		}
		else if (os::buildNumber < 26020)
		{
			auto legacyBackgroundVisual{ reinterpret_cast<CVisual* const*>(this)[39] };
			if (legacyBackgroundVisual)
			{
				brush = &reinterpret_cast<CSolidColorLegacyMilBrushProxy* const*>(legacyBackgroundVisual)[38];
			}
		}
		else
		{
			auto legacyBackgroundVisual{ reinterpret_cast<CVisual* const*>(this)[34] };
			if (legacyBackgroundVisual)
			{
				brush = &reinterpret_cast<CSolidColorLegacyMilBrushProxy* const*>(legacyBackgroundVisual)[32];
			}
		}

		return brush;
	}

	CRgnGeometryProxy* const& CTopLevelWindow::GetBorderGeometry() const
	{
		CRgnGeometryProxy* geometry{ nullptr };

		if (os::buildNumber < 19041)
		{
			geometry = reinterpret_cast<CRgnGeometryProxy* const*>(this)[68];
		}
		else if (os::buildNumber < 22000)
		{
			geometry = reinterpret_cast<CRgnGeometryProxy* const*>(this)[69];
		}
		else if (os::buildNumber < 22621)
		{
			geometry = reinterpret_cast<CRgnGeometryProxy* const*>(this)[71];
		}
		else if (os::buildNumber < 26100)
		{
			auto legacyBackgroundVisual{ reinterpret_cast<CVisual* const*>(this)[39] };
			if (legacyBackgroundVisual)
			{
				geometry = reinterpret_cast<CRgnGeometryProxy* const*>(legacyBackgroundVisual)[40];
			}
		}
		else
		{
			auto legacyBackgroundVisual{ reinterpret_cast<CVisual* const*>(this)[34] };
			if (legacyBackgroundVisual)
			{
				geometry = reinterpret_cast<CRgnGeometryProxy* const*>(legacyBackgroundVisual)[34];
			}
		}

		return geometry;
	}

	CRgnGeometryProxy* const& CTopLevelWindow::GetTitlebarGeometry() const
	{
		CRgnGeometryProxy* const* geometry{ nullptr };

		if (os::buildNumber < 19041)
		{
			geometry = &reinterpret_cast<CRgnGeometryProxy* const*>(this)[69];
		}
		else if (os::buildNumber < 22000)
		{
			geometry = &reinterpret_cast<CRgnGeometryProxy* const*>(this)[70];
		}
		else if (os::buildNumber < 22621)
		{
			geometry = &reinterpret_cast<CRgnGeometryProxy* const*>(this)[72];
		}
		else if (os::buildNumber < 26020)
		{
			auto legacyBackgroundVisual{ reinterpret_cast<CVisual* const*>(this)[39] };
			if (legacyBackgroundVisual)
			{
				geometry = &reinterpret_cast<CRgnGeometryProxy* const*>(legacyBackgroundVisual)[39];
			}
		}
		else
		{
			auto legacyBackgroundVisual{ reinterpret_cast<CVisual* const*>(this)[34] };
			if (legacyBackgroundVisual)
			{
				geometry = &reinterpret_cast<CRgnGeometryProxy* const*>(legacyBackgroundVisual)[33];
			}
		}

		return *geometry;
	}

	bool CTopLevelWindow::HasNonClientBackground(CWindowData* data)
	{
		auto windowData{ GetData() };
		if (data)
			windowData = data;
		if ((windowData->GetNonClientAttribute() & 8) == 0)
		{
			return false;
		}
		/*if (windowData->IsImmersiveWindow())
		{
			return false;
		}*/

		bool nonClientEmpty{ false };
		if (os::buildNumber < 19041)
		{
			// TO-DO
		}
		else if (os::buildNumber < 22000)
		{
			nonClientEmpty = !reinterpret_cast<DWORD const*>(this)[153] &&
				!reinterpret_cast<DWORD const*>(this)[154] &&
				!reinterpret_cast<DWORD const*>(this)[155] &&
				!reinterpret_cast<DWORD const*>(this)[156];
		}
		else if (os::buildNumber < 22621)
		{
			nonClientEmpty = !reinterpret_cast<DWORD const*>(this)[157] &&
				!reinterpret_cast<DWORD const*>(this)[158] &&
				!reinterpret_cast<DWORD const*>(this)[159] &&
				!reinterpret_cast<DWORD const*>(this)[160];
		}
		else if (os::buildNumber < 26020)
		{
			nonClientEmpty = !reinterpret_cast<DWORD const*>(this)[161] &&
				!reinterpret_cast<DWORD const*>(this)[162] &&
				!reinterpret_cast<DWORD const*>(this)[163] &&
				!reinterpret_cast<DWORD const*>(this)[164];
		}
		else
		{
			nonClientEmpty = !reinterpret_cast<DWORD const*>(this)[151] &&
				!reinterpret_cast<DWORD const*>(this)[152] &&
				!reinterpret_cast<DWORD const*>(this)[153] &&
				!reinterpret_cast<DWORD const*>(this)[154];
		}
		if (nonClientEmpty)
		{
			return false;
		}

		return true;
	}

	bool CTopLevelWindow::IsSystemBackdropApplied()
	{
		bool systemBackdropApplied{ false };

		if (os::buildNumber < 22000)
		{
			systemBackdropApplied = false;
		}
		else if (os::buildNumber < 22621)
		{
			systemBackdropApplied = *reinterpret_cast<DWORD*>(reinterpret_cast<ULONG_PTR>(GetData()) + 204);
		}
		else if (os::buildNumber < 26020)
		{
			systemBackdropApplied = (reinterpret_cast<DWORD const*>(this)[210] <= 3);
		}
		else
		{
			systemBackdropApplied = (reinterpret_cast<DWORD const*>(this)[200] <= 3);
		}

		return systemBackdropApplied;
	}

	HRESULT CTopLevelWindow::UpdateNCAreaButton(int index, int height, int top, DWORD* right)
	{
		return DEFCALL_MHOST_METHOD(CTopLevelWindow::UpdateNCAreaButton, index, height, top, right);
	}

	CText* CTopLevelWindow::GetCText()
	{
		CText* text{nullptr};
		if (os::buildNumber < 22000)
		{
			text = *((CText**)this + 65);
		}
		else
		{
			text = *((CText**)this + 67);
		}
		return text;
	}

	void CTopLevelWindow::CDWriteTextSetColor(COLORREF color)
	{
		if(os::buildNumber < 26100)
		{
			auto text = (CDWriteText*)*((ULONG64*)this + 71);//CDWriteText This
			typedef void(__fastcall**** CDWriteText_SetColor)(CDWriteText*, UINT);//(***((void(__fastcall****)(ULONG64, __int64))This + 71))(*((ULONG64*)This + 71), inputVec);
			auto pfunCDWriteText_SetColor = ***((CDWriteText_SetColor)this + 71);//CDWriteText::SetColor(CDWriteText *this, int a2)

			pfunCDWriteText_SetColor(text, color);
		}
		else
		{
			auto text = (CDWriteText*)*((ULONG64*)this + 66);
			typedef void(__fastcall**** CDWriteText_SetColor)(CDWriteText*, UINT);
			auto pfunCDWriteText_SetColor = ***((CDWriteText_SetColor)this + 66);

			pfunCDWriteText_SetColor(text, color);
		}
	}

	bool CTopLevelWindow::TreatAsActiveWindow()
	{
		return DEFCALL_MHOST_METHOD(CTopLevelWindow::TreatAsActiveWindow);
	}

	RECT* CTopLevelWindow::GetActualWindowRect(RECT* rect, char eraseOffset, char includeNonClient,
		bool excludeBorderMargins)
	{
		return DEFCALL_MHOST_METHOD(CTopLevelWindow::GetActualWindowRect, rect, eraseOffset, includeNonClient, excludeBorderMargins);
	}

	void CTopLevelWindow::GetBorderMargins(MARGINS* margins) const
	{
		DEFCALL_MHOST_METHOD(CTopLevelWindow::GetBorderMargins, margins);
	}

	bool CTopLevelWindow::IsTrullyMinimized()
	{
		RECT borderRect{};
		if (!GetActualWindowRect(&borderRect, false, true, false))
			return true;

		return borderRect.left <= -32000 || borderRect.top <= -32000;
	}

	void CTopLevelWindow::SetDirtyFlags(int flags)
	{
		DEFCALL_MHOST_METHOD(CTopLevelWindow::SetDirtyFlags, flags);
	}

	HRESULT CTopLevelWindow::OnSystemBackdropUpdated()
	{
		return DEFCALL_MHOST_METHOD(CTopLevelWindow::OnSystemBackdropUpdated);
	}

	HRESULT CTopLevelWindow::OnClipUpdated()
	{
		return DEFCALL_MHOST_METHOD(CTopLevelWindow::OnClipUpdated);
	}

	HRESULT CTopLevelWindow::OnBlurBehindUpdated()
	{
		return DEFCALL_MHOST_METHOD(CTopLevelWindow::OnBlurBehindUpdated);
	}

	HRESULT CTopLevelWindow::OnAccentPolicyUpdated()
	{
		return DEFCALL_MHOST_METHOD(CTopLevelWindow::OnAccentPolicyUpdated);
	}

	bool CTopLevelWindow::IsRTLMirrored() const
	{
		bool rtlMirrored{ false };

		if (os::buildNumber < 19041)
		{
			rtlMirrored = (reinterpret_cast<DWORD const*>(this)[146] & 0x20000) != 0;
		}
		else if (os::buildNumber < 22000)
		{
			rtlMirrored = (reinterpret_cast<DWORD const*>(this)[148] & 0x20000) != 0;
		}
		else if (os::buildNumber < 22621)
		{
			rtlMirrored = (reinterpret_cast<DWORD const*>(this)[152] & 0x20000) != 0;
		}
		else if (os::buildNumber < 26100)
		{
			rtlMirrored = (reinterpret_cast<DWORD const*>(this)[156] & 0x20000) != 0;
		}
		else
		{
			rtlMirrored = (reinterpret_cast<DWORD const*>(this)[146] & 0x20000) != 0;
		}

		return rtlMirrored;
	}

	DWORD CTopLevelWindow::GetSolidColorCaptionColor() const
	{
		return DEFCALL_MHOST_METHOD(CTopLevelWindow::GetSolidColorCaptionColor);
	}

	DWORD CTopLevelWindow::GetWindowColorizationColor(BYTE flags) const
	{
		return DEFCALL_MHOST_METHOD(CTopLevelWindow::GetWindowColorizationColor, flags);
	}

	DWORD* CTopLevelWindow::GetCurrentDefaultColorizationFlags(DWORD* flags) const
	{
		return DEFCALL_MHOST_METHOD(CTopLevelWindow::GetCurrentDefaultColorizationFlags, flags);
	}

	DWORD CTopLevelWindow::GetCurrentColorizationColor() const
	{
		DWORD color{};

		if (os::buildNumber < 22621)
		{
			DWORD flags{};
			color = GetWindowColorizationColor(static_cast<BYTE>(*GetCurrentDefaultColorizationFlags(&flags) | 8u));
		}
		else
		{
			color = GetSolidColorCaptionColor();
		}

		return color;
	}

	CTopLevelWindow::WindowFrame* CTopLevelWindow::s_ChooseWindowFrameFromStyle(char a1, bool a2, bool a3)
	{
		static auto pfun = MHostGetProcAddress<decltype(s_ChooseWindowFrameFromStyle)>("CTopLevelWindow::s_ChooseWindowFrameFromStyle");
		return pfun(a1, a2, a3);
	}

	ID2D1Effect* CCustomBlur::CropEffect()
	{
		ID2D1Effect* cropEffect = *((ID2D1Effect**)this + 3);
		return cropEffect;
	}

	ID2D1Effect* CCustomBlur::ScaleEffect()
	{
		ID2D1Effect* scaleEffect = *((ID2D1Effect**)this + 5);
		return scaleEffect;
	}

	ID2D1Effect* CCustomBlur::DirBlurKernelXEffect()
	{
		ID2D1Effect* dirBlurKernelXEffect = *((ID2D1Effect**)this + 6);
		return dirBlurKernelXEffect;
	}

	ID2D1Effect* CCustomBlur::DirBlurKernelYEffect()
	{
		ID2D1Effect* dirBlurKernelYEffect = *((ID2D1Effect**)this + 7);
		return dirBlurKernelYEffect;
	}

	bool CAccent::s_IsPolicyActive(const ACCENT_POLICY* accentPolicy)
	{
		static auto pfun = MHostGetProcAddress<decltype(s_IsPolicyActive)>("CAccent::s_IsPolicyActive");
		return pfun(accentPolicy);
	}
	CBaseGeometryProxy* const& CAccent::GetClipGeometry() const
	{
		CBaseGeometryProxy* const* geometry{ nullptr };

		if (os::buildNumber < 22000)
		{
			geometry = &reinterpret_cast<CBaseGeometryProxy* const*>(this)[52];
		}
		else if (os::buildNumber < 22621)
		{
			geometry = &reinterpret_cast<CBaseGeometryProxy* const*>(this)[53];
		}
		else if (os::buildNumber < 26020)
		{
			geometry = &reinterpret_cast<CBaseGeometryProxy* const*>(this)[48];
		}
		else
		{
			geometry = &reinterpret_cast<CBaseGeometryProxy* const*>(this)[42];
		}

		return *geometry;
	}

	HRESULT STDMETHODCALLTYPE CDrawGeometryInstruction::Create(CBaseLegacyMilBrushProxy* brush, CBaseGeometryProxy* geometry, CDrawGeometryInstruction** instruction)
	{
		static auto pfun = MHostGetProcAddress<decltype(Create)>("CDrawGeometryInstruction::Create");
		return pfun(brush, geometry, instruction);
	}
	HRESULT STDMETHODCALLTYPE CRenderDataVisual::AddInstruction(CRenderDataInstruction* instruction)
	{
		return DEFCALL_MHOST_METHOD(CRenderDataVisual::AddInstruction, instruction);
	}
	HRESULT STDMETHODCALLTYPE CRenderDataVisual::ClearInstructions()
	{
		return DEFCALL_MHOST_METHOD(CRenderDataVisual::ClearInstructions, );
	}
	HRESULT STDMETHODCALLTYPE CCanvasVisual::Create(CCanvasVisual** visual)
	{
		static auto pfun = MHostGetProcAddress<decltype(Create)>("CCanvasVisual::Create");
		return pfun(visual);
	}

	POINT* CButton::GetPoint()
	{
		POINT* pt;
		if (os::buildNumber < 22000)
			pt = (POINT*)this + 14;
		else if(os::buildNumber < 26100)
			pt = (POINT*)this + 15;
		else
			pt = (POINT*)this + 8;
		return pt;
	}

	SIZE* CButton::GetSize()
	{
		SIZE* size;
		if (os::buildNumber < 22000)
			size = (SIZE*)this + 15;
		else if (os::buildNumber < 26100)
			size = (SIZE*)this + 16;
		else
			size = (SIZE*)this + 9;
		return size;
	}

	HRESULT CCompositor::CreateVisualProxyFromSharedHandle(HANDLE handle, CVisualProxy** visualProxy)
	{
		return (this->*class_method_cast<decltype(&CCompositor::CreateVisualProxyFromSharedHandle)>("CCompositor::CreateProxyFromSharedHandle<CVisualProxy>"))(handle, visualProxy);
	}

	Core::CChannel* CCompositor::GetChannel() const
	{
		Core::CChannel* channel{ nullptr };

		if (os::buildNumber < 18362)
		{
			channel = reinterpret_cast<Core::CChannel*>(const_cast<CCompositor*>(this));
		}
		else if (os::buildNumber < 19041)
		{
			channel = reinterpret_cast<Core::CChannel* const*>(this)[2];
		}

		return channel;
	}

	CWindowList* CDesktopManager::GetWindowList() const
	{
		CWindowList* windowList{ nullptr };
		if (os::buildNumber < 22000)
		{
			windowList = reinterpret_cast<CWindowList* const*>(this)[61];
		}
		else if (os::buildNumber < 22621)
		{
			windowList = reinterpret_cast<CWindowList* const*>(this)[52];
		}
		else if (os::buildNumber < 26100)
		{
			windowList = reinterpret_cast<CWindowList* const*>(this)[54];
		}
		else
		{
			windowList = reinterpret_cast<CWindowList* const*>(this)[53];
		}
		return windowList;
	}

	IWICImagingFactory2* CDesktopManager::GetWICFactory() const
	{
		IWICImagingFactory2* factory = nullptr;

		if (os::buildNumber < 19041)
		{
			// TO-DO
		}
		else if (os::buildNumber < 22000)
		{
			factory = static_cast<IWICImagingFactory2*>(reinterpret_cast<PVOID const*>(this)[39]);
		}
		else if (os::buildNumber < 22621)
		{
			factory = static_cast<IWICImagingFactory2*>(reinterpret_cast<PVOID const*>(this)[30]);
		}
		else if (os::buildNumber < 26020)
		{
			factory = static_cast<IWICImagingFactory2*>(reinterpret_cast<PVOID const*>(this)[31]);
		}
		else
		{
			factory = static_cast<IWICImagingFactory2*>(reinterpret_cast<PVOID const*>(this)[30]);
		}

		return factory;
	}

	ID2D1Device* CDesktopManager::GetD2DDevice() const
	{
		ID2D1Device* d2dDevice = nullptr;

		if (os::buildNumber < 19041)
		{
			// TO-DO
		}
		else if (os::buildNumber < 22000)
		{
			d2dDevice = reinterpret_cast<ID2D1Device* const*>(this)[29];
		}
		else if (os::buildNumber < 22621)
		{
			d2dDevice = static_cast<ID2D1Device**>(reinterpret_cast<void* const*>(this)[6])[3];
		}
		else if (os::buildNumber < 26020)
		{
			d2dDevice = static_cast<ID2D1Device**>(reinterpret_cast<void* const*>(this)[7])[3];
		}
		else
		{
			d2dDevice = static_cast<ID2D1Device**>(reinterpret_cast<void* const*>(this)[7])[4];
		}

		return d2dDevice;
	}

	DCompPrivate::IDCompositionDesktopDevicePartner* CDesktopManager::GetDCompositionInteropDevice() const
	{
		DCompPrivate::IDCompositionDesktopDevicePartner* interopDevice = nullptr;

		if (os::buildNumber < 19041)
		{
			// TO-DO
		}
		else if (os::buildNumber < 22000)
		{
			interopDevice = reinterpret_cast<DCompPrivate::IDCompositionDesktopDevicePartner* const*>(this)[27];

			BOOL valid{ FALSE };
			com_ptr<IDCompositionDevice> desktopDevice{ nullptr };
			check_hresult(interopDevice->QueryInterface(desktopDevice.put()));
			check_hresult(desktopDevice->CheckDeviceState(&valid));
			if (!valid)
			{
				class_method_cast<void(STDMETHODCALLTYPE*)()>("CDesktopManager::HandleInteropDeviceLost")();
			}
		}
		else if (os::buildNumber < 22621)
		{
			interopDevice = static_cast<DCompPrivate::IDCompositionDesktopDevicePartner**>(reinterpret_cast<void* const*>(this)[5])[4];
		}
		else
		{
			interopDevice = static_cast<DCompPrivate::IDCompositionDesktopDevicePartner**>(reinterpret_cast<void* const*>(this)[6])[4]; // ok with build 26020
		}

		return interopDevice;
	}

	CCompositor* CDesktopManager::GetCompositor() const
	{
		CCompositor* compositor{ nullptr };

		if (os::buildNumber < 22000)
		{
			compositor = reinterpret_cast<CCompositor* const*>(this)[5];
		}
		else
		{
			compositor = reinterpret_cast<CCompositor* const*>(this)[6];
		}

		return compositor;
	}

	UINT CDesktopManager::MonitorDpiFromPoint(POINT pt) const
	{
		return DEFCALL_MHOST_METHOD(CDesktopManager::MonitorDpiFromPoint, pt);
	}

	HWND CWindowNode::GetHwnd() const
	{
		return DEFCALL_MHOST_METHOD(CWindowNode::GetHwnd);
	}

	HRESULT ResourceHelper::CreateGeometryFromHRGN(HRGN hrgn, CRgnGeometryProxy** geometry)
	{
		static auto pfun = MHostGetProcAddress<decltype(CreateGeometryFromHRGN)>("ResourceHelper::CreateGeometryFromHRGN");
		return pfun(hrgn, geometry);
	}

	void CText::SetColor(COLORREF color)
	{
		DEFCALL_MHOST_METHOD(CText::SetColor, color);
	}

	HRESULT CText::SetText(LPCWSTR text)
	{
		return DEFCALL_MHOST_METHOD(CText::SetText, text);
	}

	HRESULT CText::SetSize(SIZE* size)
	{
		return DEFCALL_MHOST_METHOD(CText::SetSize, size);
	}

	LPCWSTR CText::GetText()
	{
		LPCWSTR text{nullptr};
		if (os::buildNumber < 22000)
		{
			text = (wchar_t*)*((DWORD64*)this + 36);
		}
		else
		{
			text = (wchar_t*)*((DWORD64*)this + 37);
		}
		return text;
	}

	bool CText::IsRTL() const
	{
		return (reinterpret_cast<BYTE const*>(this)[280] & 2) != 0;
	}

	CMatrixTransformProxy* CText::GetMatrixProxy()
	{
		if(os::buildNumber >= 22000)
			return (CMatrixTransformProxy*)*((DWORD64*)this + 50);
		return (CMatrixTransformProxy*)*((DWORD64*)this + 49);
	}

	HRESULT CWindowList::GetExtendedFrameBounds(HWND hWnd, RECT* rect)
	{
		return DEFCALL_MHOST_METHOD(CWindowList::GetExtendedFrameBounds, hWnd, rect);
	}

	HRESULT CWindowList::GetSyncedWindowData(IDwmWindow* dwmWindow, bool shared, CWindowData** pWindowData)
	{
		return DEFCALL_MHOST_METHOD(CWindowList::GetSyncedWindowData, dwmWindow, shared, pWindowData);
	}
	HRESULT STDMETHODCALLTYPE CWindowList::GetSyncedWindowDataByHwnd(HWND hwnd, CWindowData** windowData)
	{
		auto hr = DEFCALL_MHOST_METHOD(CWindowList::GetSyncedWindowDataByHwnd, hwnd, windowData);
		if (os::buildNumber >= 26020)
			return S_OK;
		return hr;
	}

	PRLIST_ENTRY CWindowList::GetWindowListForDesktop(ULONG_PTR desktopID)
	{
		return DEFCALL_MHOST_METHOD(CWindowList::GetWindowListForDesktop, desktopID);
	}

	HWND CWindowList::GetShellWindowForDesktop(ULONG_PTR desktopID)
	{
		return DEFCALL_MHOST_METHOD(CWindowList::GetShellWindowForDesktop, desktopID);
	}

	namespace Core
	{
		UINT CResource::GetOwningProcessId()
		{
			return DEFCALL_MHOST_METHOD(CResource::GetOwningProcessId);
		}

		HRESULT CChannel::DuplicateSharedResource(HANDLE handle, UINT type, UINT* handleIndex)
		{
			return DEFCALL_MHOST_METHOD(CChannel::DuplicateSharedResource, handle, type, handleIndex);
		}

		HRESULT CChannel::MatrixTransformUpdate(UINT handleIndex, MilMatrix3x2D* matrix)
		{
			return DEFCALL_MHOST_METHOD(CChannel::MatrixTransformUpdate, handleIndex, matrix);
		}
	}

	PRLIST_ENTRY CWindowList::GetWindowListForDesktop(ULONG_PTR desktopID)
	{
		return DEFCALL_MHOST_METHOD(CWindowList::GetWindowListForDesktop, desktopID);
	}
}
#pragma pop_macro("DEFCALL_MHOST_METHOD")