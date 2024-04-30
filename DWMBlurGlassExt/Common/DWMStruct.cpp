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
		else if (os::buildNumber < 22621)
		{
			properties = &reinterpret_cast<BYTE const*>(this)[92];
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
		else if (os::buildNumber < 22621)
		{
			properties = &reinterpret_cast<BYTE*>(this)[92];
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

	CVisual* CTopLevelWindow::GetVisual() const
	{
		CVisual* visual{ nullptr };

		if (os::buildNumber < 19041)
		{
			// TO-DO
		}
		else if (os::buildNumber < 22000)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[33];
		}
		else if (os::buildNumber < 22621)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[34];
		}
		else if (os::buildNumber < 26020)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[36];
		}
		else
		{
			visual = reinterpret_cast<CVisual* const*>(this)[31];
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
			accent = reinterpret_cast<CAccent* const*>(this)[34];
		}

		return accent;
	}
	CVisual* CTopLevelWindow::GetLegacyVisual() const
	{
		CVisual* visual{ nullptr };

		if (os::buildNumber < 22000)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[36];
		}
		else if (os::buildNumber < 22621)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[37];
		}
		else if (os::buildNumber < 26020)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[39];
		}
		else
		{
			visual = reinterpret_cast<CVisual* const*>(this)[40];
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
			visual = reinterpret_cast<CVisual* const*>(this)[37];
		}
		else
		{
			visual = reinterpret_cast<CVisual* const*>(this)[40];
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
	CVisual* CTopLevelWindow::GetAccentColorVisual() const
	{
		CVisual* visual{ nullptr };

		if (os::buildNumber < 22000)
		{
		}
		else if (os::buildNumber < 22621)
		{
		}
		else if (os::buildNumber < 26020)
		{
			visual = reinterpret_cast<CVisual* const*>(this)[41];
		}
		else
		{
			visual = reinterpret_cast<CVisual* const*>(this)[36];
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
		CRgnGeometryProxy* const* geometry{ nullptr };

		if (os::buildNumber < 19041)
		{
			// TO-DO
		}
		else if (os::buildNumber < 22000)
		{
			geometry = &reinterpret_cast<CRgnGeometryProxy* const*>(this)[69];
		}
		else if (os::buildNumber < 22621)
		{
			geometry = &reinterpret_cast<CRgnGeometryProxy* const*>(this)[71];
		}
		else if (os::buildNumber < 26020)
		{
			auto legacyBackgroundVisual{ reinterpret_cast<CVisual* const*>(this)[40] };
			if (legacyBackgroundVisual)
			{
				geometry = &reinterpret_cast<CRgnGeometryProxy* const*>(legacyBackgroundVisual)[40];
			}
		}
		else
		{
			auto legacyBackgroundVisual{ reinterpret_cast<CVisual* const*>(this)[34] };
			if (legacyBackgroundVisual)
			{
				geometry = &reinterpret_cast<CRgnGeometryProxy* const*>(legacyBackgroundVisual)[34];
			}
		}

		return *geometry;
	}

	CRgnGeometryProxy* const& CTopLevelWindow::GetTitlebarGeometry() const
	{
		CRgnGeometryProxy* const* geometry{ nullptr };

		if (os::buildNumber < 19041)
		{
			// TO-DO
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
			auto legacyBackgroundVisual{ reinterpret_cast<CVisual* const*>(this)[33] };
			if (legacyBackgroundVisual)
			{
				geometry = &reinterpret_cast<CRgnGeometryProxy* const*>(legacyBackgroundVisual)[33];
			}
		}

		return *geometry;
	}

	bool CTopLevelWindow::HasNonClientBackground()
	{
		auto windowData{ GetData() };
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
		auto text = (CDWriteText*)*((ULONG64*)this + 71);//CDWriteText This
		typedef void(__fastcall**** CDWriteText_SetColor)(CDWriteText*, UINT);//(***((void(__fastcall****)(ULONG64, __int64))This + 71))(*((ULONG64*)This + 71), inputVec);
		auto pfunCDWriteText_SetColor = ***((CDWriteText_SetColor)this + 71);//CDWriteText::SetColor(CDWriteText *this, int a2)

		pfunCDWriteText_SetColor(text, color);
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
		else
			pt = (POINT*)this + 15;
		return pt;
	}

	SIZE* CButton::GetSize()
	{
		SIZE* size;
		if (os::buildNumber < 22000)
			size = (SIZE*)this + 15;
		else
			size = (SIZE*)this + 16;
		return size;
	}

	CWindowList* CDesktopManager::GetWindowList() const
	{
		return static_cast<CWindowList*>(reinterpret_cast<PVOID const*>(this)[61]);
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

	UINT CDesktopManager::MonitorDpiFromPoint(POINT pt) const
	{
		return DEFCALL_MHOST_METHOD(CDesktopManager::MonitorDpiFromPoint, pt);
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
		return DEFCALL_MHOST_METHOD(CWindowList::GetSyncedWindowDataByHwnd, hwnd, windowData);
	}

	PRLIST_ENTRY CWindowList::GetWindowListForDesktop(ULONG_PTR desktopID)
	{
		return DEFCALL_MHOST_METHOD(CWindowList::GetWindowListForDesktop, desktopID);
	}
}
#pragma pop_macro("DEFCALL_MHOST_METHOD")