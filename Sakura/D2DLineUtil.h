#pragma once
#include <d2d1.h>
#include <wrl.h>
#include <functional>
namespace {
	using Microsoft::WRL::ComPtr;
}
namespace tignear::d2d {
	class Line {
		ComPtr<ID2D1Factory> m_d2d_factory;
		ComPtr<ID2D1StrokeStyle> m_dot_style;
		ComPtr<ID2D1StrokeStyle> m_dash_style;
	private:


	public:
		Line(ComPtr<ID2D1Factory> f):m_d2d_factory(f) {}
		/*
		solid
		*/
		HRESULT DrawSolidLine(ID2D1RenderTarget* target, D2D1_POINT_2F p0, D2D1_POINT_2F p1, ID2D1Brush* brush, FLOAT storokeWidth = (1.0F)) {
			target->DrawLine(p0, p1, brush, storokeWidth,NULL);
			return S_OK;
		}
		/*
		dot
		*/
		HRESULT DrawDOTLine(ID2D1RenderTarget* target, D2D1_POINT_2F p0, D2D1_POINT_2F p1, ID2D1Brush* brush, FLOAT storokeWidth=(1.0F)) {
			if (!m_dot_style && FAILED(InitDOTLineStyle()))
			{
				return E_FAIL;
			}
			target->DrawLine(p0,p1,brush,storokeWidth,m_dot_style.Get());
			return S_OK;
		}
		HRESULT InitDOTLineStyle() {
			D2D1_STROKE_STYLE_PROPERTIES prop{};
			prop.dashStyle = D2D1_DASH_STYLE_DOT;
			prop.dashCap = D2D1_CAP_STYLE_ROUND;
			auto hr= m_d2d_factory->CreateStrokeStyle(prop,NULL,0,&m_dot_style);
			return hr;
		}
		/*
		dash
		*/
		HRESULT DrawDashLine(ID2D1RenderTarget* target, D2D1_POINT_2F p0, D2D1_POINT_2F p1, ID2D1Brush* brush, FLOAT storokeWidth = (1.0F)) {
			if (!m_dash_style && FAILED(InitDashLineStyle()))
			{
				return E_FAIL;
			}
			target->DrawLine(p0, p1, brush, storokeWidth, m_dash_style.Get());
			return S_OK;
		}
		HRESULT InitDashLineStyle() {
			D2D1_STROKE_STYLE_PROPERTIES prop{};
			prop.dashStyle = D2D1_DASH_STYLE_DASH;
			prop.dashCap = D2D1_CAP_STYLE_ROUND;
			auto hr=m_d2d_factory->CreateStrokeStyle(prop, NULL, 0, &m_dash_style);
			return hr;
		}
	};

}