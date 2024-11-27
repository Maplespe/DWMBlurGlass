#pragma once
#include "DWMStruct.h"

namespace MDWMBlurGlassExt::GeometryRecorder
{
	HRESULT STDMETHODCALLTYPE ResourceHelper_CreateGeometryFromHRGN(
		HRGN hrgn,
		DWM::CRgnGeometryProxy** geometry
	);

	HRESULT STDMETHODCALLTYPE ResourceHelper_CreateRectangleGeometry(
		LPCRECT lprc,
		DWM::CRgnGeometryProxy** geometry
	);

	HRESULT STDMETHODCALLTYPE ResourceHelper_CreateCombinedGeometry(
		DWM::CBaseGeometryProxy* geometry1,
		DWM::CBaseGeometryProxy* geometry2,
		UINT combineMode,
		DWM::CCombinedGeometryProxy** combinedGeometry
	);

	HRESULT STDMETHODCALLTYPE CRgnGeometryProxy_Update(
		DWM::CRgnGeometryProxy* This,
		LPCRECT lprc,
		UINT count
	);

	void Attach();
	void Detach();

	void BeginCapture();
	HRGN GetRegionFromGeometry(DWM::CBaseGeometryProxy* geometry);
	size_t GetGeometryCount();
	void EndCapture();
}