#pragma once
#include <dwrite.h>
#include <d2d1.h>
#include <memory>
#include "D2DLineUtil.h"
namespace {
	using tignear::d2d::Line;
}
namespace tignear::tsf {
	enum LineStyle {
		LineStyle_Solid, LineStyle_Dot, LineStyle_Dash, LineStyle_Squiggle
	};
	struct TsfDWriteDrawerEffectUnderline {
		const LineStyle lineStyle;
		const bool boldLine;
		const Microsoft::WRL::ComPtr<ID2D1Brush> lineColor;
		TsfDWriteDrawerEffectUnderline(const LineStyle ls,const bool bold, ID2D1Brush* b):lineStyle(ls),boldLine(bold),lineColor(b) {}
	};
	struct TsfDWriteDrawerEffect :IUnknown {
		const Microsoft::WRL::ComPtr<ID2D1Brush> backgroundColor;
		const Microsoft::WRL::ComPtr<ID2D1Brush> textColor;
		std::unique_ptr<TsfDWriteDrawerEffectUnderline> underline;
		TsfDWriteDrawerEffect(ID2D1Brush* bg, ID2D1Brush* fr, std::unique_ptr<TsfDWriteDrawerEffectUnderline> under) :backgroundColor(bg), textColor(fr), underline(std::move(under)), m_ref_cnt(0) {}
		// IUnknown methods
		ULONG STDMETHODCALLTYPE AddRef() override;
		ULONG STDMETHODCALLTYPE Release() override;
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
			void **ppvObject) override;
	private:
		LONG m_ref_cnt;
	};
	struct TsfDWriteDrawerContext
	{
		ID2D1RenderTarget* renderTarget;
		Microsoft::WRL::ComPtr<TsfDWriteDrawerEffect> dafaultEffect;
		TsfDWriteDrawerContext(ID2D1RenderTarget* t, TsfDWriteDrawerEffect* e) :renderTarget(t), dafaultEffect(e) {}
	};
	class TsfDWriteDrawer:public IDWriteTextRenderer {
	private:
		std::unique_ptr<Line> m_line;
		Microsoft::WRL::ComPtr<ID2D1Factory> m_factory;
		TsfDWriteDrawer(ID2D1Factory* factory) :m_line(std::make_unique<Line>(factory)),m_factory(factory) {}
		ULONG m_ref_cnt;
	public:
		ULONG STDMETHODCALLTYPE AddRef() override;
		ULONG STDMETHODCALLTYPE Release() override;
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
			void **ppvObject) override;
		static void Create(ID2D1Factory* factory, TsfDWriteDrawer** render);
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