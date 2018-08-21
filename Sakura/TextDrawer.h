#pragma once
#include <dwrite.h>
#include <d2d1.h>
#include <memory>
#include "D2DLineUtil.h"
namespace tignear::dwrite {
	enum LineStyle {
		LineStyle_Solid, LineStyle_Dot, LineStyle_Dash, LineStyle_Squiggle
	};
	struct DWriteDrawerEffectUnderline {
		const LineStyle lineStyle;
		const bool boldLine;
		const Microsoft::WRL::ComPtr<ID2D1Brush> lineColor;
		DWriteDrawerEffectUnderline(const LineStyle ls,const bool bold, ID2D1Brush* b):lineStyle(ls),boldLine(bold),lineColor(b) {}
	};
	struct DWriteDrawerEffect :IUnknown {
		const Microsoft::WRL::ComPtr<ID2D1Brush> backgroundColor;
		const Microsoft::WRL::ComPtr<ID2D1Brush> textColor;
		std::unique_ptr<DWriteDrawerEffectUnderline> underline;
		DWriteDrawerEffect(ID2D1Brush* bg, ID2D1Brush* fr, std::unique_ptr<DWriteDrawerEffectUnderline> under) :backgroundColor(bg), textColor(fr), underline(std::move(under)), m_ref_cnt(0) {}
		// IUnknown methods
		ULONG STDMETHODCALLTYPE AddRef() override;
		ULONG STDMETHODCALLTYPE Release() override;
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
			void **ppvObject) override;
	private:
		LONG m_ref_cnt;
	};
	struct DWriteDrawerContext
	{
		ID2D1RenderTarget* renderTarget;
		Microsoft::WRL::ComPtr<DWriteDrawerEffect> dafaultEffect;
		DWriteDrawerContext(ID2D1RenderTarget* t, DWriteDrawerEffect* e) :renderTarget(t), dafaultEffect(e) {}
	};
	class DWriteDrawer:public IDWriteTextRenderer {
	private:
		std::unique_ptr<tignear::d2d::Line> m_line;
		Microsoft::WRL::ComPtr<ID2D1Factory> m_factory;
		DWriteDrawer(ID2D1Factory* factory) :m_line(std::make_unique<tignear::d2d::Line>(factory)),m_factory(factory) {}
		ULONG m_ref_cnt;
	public:
		ULONG STDMETHODCALLTYPE AddRef() override;
		ULONG STDMETHODCALLTYPE Release() override;
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
			void **ppvObject) override;
		static void Create(ID2D1Factory* factory, DWriteDrawer** render);
		HRESULT STDMETHODCALLTYPE DrawGlyphRun(
			void * clientDrawingContext,
			FLOAT  baselineOriginX,
			FLOAT  baselineOriginY,
			DWRITE_MEASURING_MODE  measuringMode,
			const DWRITE_GLYPH_RUN * glyphRun,
			const DWRITE_GLYPH_RUN_DESCRIPTION * glyphRunDescription,
			IUnknown * clientDrawingEffect
		)override;
		HRESULT STDMETHODCALLTYPE DrawUnderline(
			void * clientDrawingContext,
			FLOAT  baselineOriginX,
			FLOAT  baselineOriginY,
			const DWRITE_UNDERLINE * underline,
			IUnknown * clientDrawingEffect
		)override;
		
		HRESULT STDMETHODCALLTYPE DrawStrikethrough(
			void * clientDrawingContext,
			FLOAT  baselineOriginX,
			FLOAT  baselineOriginY,
			const DWRITE_STRIKETHROUGH * strikethrough,
			IUnknown * clientDrawingEffect
		) override;
		HRESULT STDMETHODCALLTYPE DrawInlineObject(
			void * clientDrawingContext,
			FLOAT  originX,
			FLOAT  originY,
			IDWriteInlineObject * inlineObject,
			BOOL  isSideways,
			BOOL  isRightToLeft,
			IUnknown * clientDrawingEffect
		)override;

		HRESULT STDMETHODCALLTYPE GetCurrentTransform(
			void * clientDrawingContext,
			DWRITE_MATRIX * transform
		) override;
		HRESULT STDMETHODCALLTYPE GetPixelsPerDip(void * clientDrawingContext,FLOAT * pixelsPerDip) override;
		HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(
			 void* clientDrawingContext,
			 BOOL* isDisabled
		)override;
	};
}