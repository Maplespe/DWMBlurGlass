#pragma once
#include "../Helper/Helper.h"
#include "../Common/winrt.h"
#include "../Common/wil.h"

namespace MDWMBlurGlassExt::Geometry
{
	class CanvasGeometry : public winrt::implements<CanvasGeometry, ABI::Windows::Graphics::IGeometrySource2D, ABI::Windows::Graphics::IGeometrySource2DInterop>
	{
		winrt::com_ptr<ID2D1Geometry> m_geometry{ nullptr };
	public:
		void SetGeometry(ID2D1Geometry* geometry)
		{
			winrt::copy_from_abi(m_geometry, geometry);
		}
		IFACEMETHOD(GetGeometry)(
			ID2D1Geometry** geometry
		) override
		{
			m_geometry.copy_to(geometry);
			return S_OK;
		}
		IFACEMETHOD(TryGetGeometryUsingFactory)(
			ID2D1Factory* factory,
			ID2D1Geometry** geometry
		) override
		{
			winrt::com_ptr<ID2D1Factory> geometryFactory{ nullptr };
			m_geometry->GetFactory(geometryFactory.put());
			if (geometryFactory.get() == factory)
			{
				m_geometry.copy_to(geometry);
			}
			return S_OK;
		}

		static winrt::com_ptr<CanvasGeometry> CreateGeometryFromRectangles(
			ID2D1Factory* factory,
			const std::vector<RECT>& rectangles
		) try
		{
			auto canvasGeometry{ winrt::make_self<CanvasGeometry>() };

			winrt::com_ptr<ID2D1PathGeometry> geometry{ nullptr };
			THROW_IF_FAILED(
				factory->CreatePathGeometry(
					geometry.put()
				)
			);
			winrt::com_ptr<ID2D1GeometrySink> sink{ nullptr };
			THROW_IF_FAILED(geometry->Open(sink.put()));

			winrt::com_ptr<ID2D1RectangleGeometry> emptyGeometry{ nullptr };
			THROW_IF_FAILED(
				factory->CreateRectangleGeometry(
					D2D1::RectF(),
					emptyGeometry.put()
				)
			);
			for (const auto& rect : rectangles)
			{
				winrt::com_ptr<ID2D1RectangleGeometry> rectangleGeometry{ nullptr };
				THROW_IF_FAILED(
					factory->CreateRectangleGeometry(
						D2D1::RectF(
							static_cast<float>(rect.left),
							static_cast<float>(rect.top),
							static_cast<float>(rect.right),
							static_cast<float>(rect.bottom)
						),
						rectangleGeometry.put()
					)
				);
				THROW_IF_FAILED(
					emptyGeometry->CombineWithGeometry(
						rectangleGeometry.get(),
						D2D1_COMBINE_MODE::D2D1_COMBINE_MODE_UNION,
						nullptr,
						sink.get()
					)
				);
			}
			THROW_IF_FAILED(sink->Close());

			canvasGeometry->m_geometry = geometry;
			return canvasGeometry;
		}
		catch (...) { return nullptr; }

		static winrt::com_ptr<CanvasGeometry> CreateGeometryFromHRGN(
			ID2D1Factory* factory,
			HRGN hrgn
		) try
		{
			auto canvasGeometry{ winrt::make_self<CanvasGeometry>() };

			auto size{ GetRegionData(hrgn, 0, nullptr) };
			THROW_LAST_ERROR_IF(size == 0);
			std::unique_ptr<RGNDATA, decltype([](LPRGNDATA ptr) { return free(ptr); })> rgnData{reinterpret_cast<LPRGNDATA>(malloc(size))};
			THROW_LAST_ERROR_IF(GetRegionData(hrgn, size, rgnData.get()) == 0);

			winrt::com_ptr<ID2D1PathGeometry> geometry{ nullptr };
			THROW_IF_FAILED(
				factory->CreatePathGeometry(
					geometry.put()
				)
			);
			winrt::com_ptr<ID2D1GeometrySink> sink{ nullptr };
			THROW_IF_FAILED(geometry->Open(sink.put()));

			winrt::com_ptr<ID2D1RectangleGeometry> emptyGeometry{ nullptr };
			THROW_IF_FAILED(
				factory->CreateRectangleGeometry(
					D2D1::RectF(),
					emptyGeometry.put()
				)
			);
			for (size_t i = 0; i < rgnData->rdh.nCount; i++)
			{
				auto rectangle{ &reinterpret_cast<LPRECT>(rgnData->Buffer)[i] };

				winrt::com_ptr<ID2D1RectangleGeometry> rectangleGeometry{ nullptr };
				THROW_IF_FAILED(
					factory->CreateRectangleGeometry(
						D2D1::RectF(
							static_cast<float>(rectangle->left),
							static_cast<float>(rectangle->top),
							static_cast<float>(rectangle->right),
							static_cast<float>(rectangle->bottom)
						),
						rectangleGeometry.put()
					)
				);
				THROW_IF_FAILED(
					emptyGeometry->CombineWithGeometry(
						rectangleGeometry.get(),
						D2D1_COMBINE_MODE::D2D1_COMBINE_MODE_UNION,
						nullptr,
						sink.get()
					)
				);
			}
			THROW_IF_FAILED(sink->Close());

			canvasGeometry->m_geometry = geometry;
			return canvasGeometry;
		}
		catch(...) { return nullptr; }
	};
}