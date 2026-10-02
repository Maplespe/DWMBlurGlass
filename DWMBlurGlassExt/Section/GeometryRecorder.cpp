#include "GeometryRecorder.h"
#include <wil.h>
#include <windowsx.h>

namespace MDWMBlurGlassExt::GeometryRecorder
{
	using namespace MDWMBlurGlass;

	std::atomic_bool g_startup = false;
	size_t g_captureRef{ 0 };
	std::unordered_map<DWM::CBaseGeometryProxy*, wil::unique_hrgn> g_geometryMap{};

	MinHook g_funResourceHelper_CreateGeometryFromHRGN
	{
		"ResourceHelper::CreateGeometryFromHRGN",
		ResourceHelper_CreateGeometryFromHRGN
	};

	MinHook g_funResourceHelper_CreateRectangleGeometry
	{
		"?CreateRectangleGeometry@ResourceHelper@@SAJPEBUtagRECT@@PEAPEAVCRectangleGeometryProxy@@@Z",
		ResourceHelper_CreateRectangleGeometry
	};

	MinHook g_funResourceHelper_CreateCombinedGeometry
	{
		"ResourceHelper::CreateCombinedGeometry",
		ResourceHelper_CreateCombinedGeometry
	};

	MinHook g_funCRgnGeometryProxy_Update
	{
		"CRgnGeometryProxy::Update",
		CRgnGeometryProxy_Update
	};

	void RecordGeometry(DWM::CBaseGeometryProxy* geometry, HRGN region)
	{
		g_geometryMap.insert_or_assign(geometry, std::move(wil::unique_hrgn{ region }));
	}

	void Attach()
	{
		if (g_startup) return;
		g_startup = true;

		g_funResourceHelper_CreateGeometryFromHRGN.Attach();
		g_funResourceHelper_CreateRectangleGeometry.Attach();
		g_funResourceHelper_CreateCombinedGeometry.Attach();
		if (os::buildNumber >= 22000)
		{
			g_funCRgnGeometryProxy_Update.Attach();
		}
	}

	void Detach()
	{
		if (!g_startup) return;

		g_funResourceHelper_CreateCombinedGeometry.Detach();
		g_funResourceHelper_CreateRectangleGeometry.Detach();
		g_funResourceHelper_CreateGeometryFromHRGN.Detach();
		if (os::buildNumber >= 22000)
		{
			g_funCRgnGeometryProxy_Update.Detach();
		}

		g_startup = false;
	}

	void BeginCapture()
	{
		g_captureRef++;
	}

	HRGN GetRegionFromGeometry(DWM::CBaseGeometryProxy* geometry)
	{
		auto it{ g_geometryMap.find(geometry) };
		if (it == g_geometryMap.end())
		{
			return nullptr;
		}

		return it->second.get();
	}

	size_t GetGeometryCount()
	{
		return g_geometryMap.size();
	}

	bool FindFallbackGeometries(
		DWM::CBaseGeometryProxy** outCaptionGeom, HRGN* outCaptionRgn,
		DWM::CBaseGeometryProxy** outBorderGeom, HRGN* outBorderRgn
	)
	{
		if (g_geometryMap.empty()) return false;

		for (const auto& [geom, rgn] : g_geometryMap)
		{
			if (!rgn) continue;
			RECT rc{};
			auto type = GetRgnBox(rgn.get(), &rc);
			if (type == NULLREGION || IsRectEmpty(&rc)) continue;

			int h = rc.bottom - rc.top;
			int w = rc.right - rc.left;

			// A titlebar/caption region is typically wide and between 15 and 80 px in height, located at or near top
			if (h >= 15 && h <= 100 && w > 80 && rc.top <= 10)
			{
				if (outCaptionGeom && !*outCaptionGeom)
				{
					*outCaptionGeom = geom;
					if (outCaptionRgn) *outCaptionRgn = rgn.get();
					continue;
				}
			}

			// Otherwise it might be the border/frame
			if (outBorderGeom && !*outBorderGeom)
			{
				*outBorderGeom = geom;
				if (outBorderRgn) *outBorderRgn = rgn.get();
			}
		}

		return (outCaptionGeom && *outCaptionGeom) || (outBorderGeom && *outBorderGeom);
	}

	void EndCapture()
	{
		g_captureRef--;
		if (g_captureRef == 0)
		{
			g_geometryMap.clear();
		}
	}

	//CTopLevelWindow::UpdateNCGeometry
	HRESULT ResourceHelper_CreateGeometryFromHRGN(HRGN hrgn, DWM::CRgnGeometryProxy** geometry)
	{
		HRESULT hr{ g_funResourceHelper_CreateGeometryFromHRGN.call_org(hrgn, geometry) };

		if (SUCCEEDED(hr) && geometry && *geometry && g_captureRef)
		{
			HRGN region{ CreateRectRgn(0, 0, 0, 0) };
			CopyRgn(region, hrgn);
			RecordGeometry(*geometry, region);
		}

		return hr;
	}

	//CTopLevelWindow::UpdateClientBlur
	HRESULT ResourceHelper_CreateRectangleGeometry(LPCRECT lprc, DWM::CRgnGeometryProxy** geometry)
	{
		HRESULT hr{ g_funResourceHelper_CreateRectangleGeometry.call_org(lprc, geometry) };

		if (SUCCEEDED(hr) && geometry && *geometry && g_captureRef)
		{
			HRGN region{ CreateRectRgnIndirect(lprc) };
			RecordGeometry(*geometry, region);
		}

		return hr;
	}

	//CTopLevelWindow::UpdateClientBlur
	HRESULT ResourceHelper_CreateCombinedGeometry(DWM::CBaseGeometryProxy* geometry1,
		DWM::CBaseGeometryProxy* geometry2, UINT combineMode, DWM::CCombinedGeometryProxy** combinedGeometry)
	{
		HRESULT hr{ g_funResourceHelper_CreateCombinedGeometry.call_org(geometry1, geometry2, combineMode, combinedGeometry) };

		if (SUCCEEDED(hr) && combinedGeometry && *combinedGeometry && g_captureRef)
		{
			HRGN region{ CreateRectRgn(0, 0, 0, 0) };
			CombineRgn(
				region,
				GetRegionFromGeometry(geometry1),
				GetRegionFromGeometry(geometry2),
				RGN_AND
			);
			RecordGeometry(*combinedGeometry, region);
		}

		return hr;
	}

	//Windows 11 21H2 CTopLevelWindow::UpdateNCGeometry / Windows 11 22H2 CLegacyNonClientBackground::SetBorderRegion
	HRESULT CRgnGeometryProxy_Update(DWM::CRgnGeometryProxy* This, LPCRECT lprc, UINT count)
	{
		HRESULT hr{ g_funCRgnGeometryProxy_Update.call_org(This, lprc, count) };

		if (SUCCEEDED(hr))
		{
			if (lprc && count)
			{
				HRGN region{ CreateRectRgnIndirect(lprc) };
				RecordGeometry(This, region);
			}
			else
			{
				HRGN region{ CreateRectRgn(0, 0, 0, 0) };
				RecordGeometry(This, region);
			}
		}

		return hr;
	}
}
