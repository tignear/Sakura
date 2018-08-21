#include "stdafx.h"
#include <utility>
#include "TextDrawer.h"
#include "D2DLineUtil.h"
#include "FailToThrow.h"
using tignear::dwrite::DWriteDrawer;
using tignear::dwrite::DWriteDrawerEffect;
//http://www.charlespetzold.com/blog/2014/01/Character-Formatting-Extensions-with-DirectWrite.html
namespace {
	constexpr auto pi = 3.141592653589793238L;
}
ULONG  DWriteDrawerEffect::AddRef() {
	m_ref_cnt++;
	return m_ref_cnt;
}
ULONG  DWriteDrawerEffect::Release() {
	m_ref_cnt--;
	if (m_ref_cnt == 0) {
		delete this;
		return 0;
	}
	else {
		return m_ref_cnt;

	}
}
HRESULT  DWriteDrawerEffect::QueryInterface(REFIID riid,
	void **ppvObject) {
	if (riid == IID_IUnknown) {
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else {
		return E_NOINTERFACE;
	}
}

ULONG  DWriteDrawer::AddRef() {
	m_ref_cnt++;
	return m_ref_cnt;
}
ULONG  DWriteDrawer::Release() {
	m_ref_cnt--;
	if (m_ref_cnt == 0) {
		delete this;
		return 0;
	}
	else {
		return m_ref_cnt;

	}
}
HRESULT  DWriteDrawer::QueryInterface(REFIID riid,
	void **ppvObject) {
	if (riid == IID_IUnknown) {
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else {
		return E_NOINTERFACE;
	}
}
HRESULT DWriteDrawer::DrawGlyphRun(
	void * clientDrawingContext,
	FLOAT  baselineOriginX,
	FLOAT  baselineOriginY,
	DWRITE_MEASURING_MODE  measuringMode,
	const DWRITE_GLYPH_RUN * glyphRun,
	const DWRITE_GLYPH_RUN_DESCRIPTION * glyphRunDescription,
	IUnknown * clientDrawingEffect
) {
	if (clientDrawingContext == nullptr) {
		return E_INVALIDARG;
	}
	auto context=static_cast<DWriteDrawerContext*>(clientDrawingContext);
	if ( context->renderTarget == nullptr) {
		return E_INVALIDARG;
	}
	auto effect = static_cast<DWriteDrawerEffect*>(clientDrawingEffect);
	auto target = context->renderTarget;
	if (effect == nullptr) {
		effect = context->dafaultEffect.Get();
	}
	ID2D1Brush* backgroundBrush = effect->backgroundColor.Get();
	if (backgroundBrush != nullptr)
	{
		float totalWidth = 0;

		for (UINT32 index = 0; index < glyphRun->glyphCount; index++)
		{
			totalWidth += glyphRun->glyphAdvances[index];
		}

		DWRITE_FONT_METRICS fontMetrics;
		glyphRun->fontFace->GetMetrics(&fontMetrics);
		float adjust = glyphRun->fontEmSize / fontMetrics.designUnitsPerEm;
		float ascent = adjust * fontMetrics.ascent;
		float descent = adjust * fontMetrics.descent;
		D2D1_RECT_F rect{ baselineOriginX,
			baselineOriginY - ascent,
			baselineOriginX + totalWidth,
			baselineOriginY + descent };

		target->FillRectangle(rect, backgroundBrush);
	}

	ID2D1Brush* brush = effect->textColor.Get();
	
	if (brush == nullptr) {
		return E_INVALIDARG;
	}
	target->DrawGlyphRun({baselineOriginX,baselineOriginY},glyphRun,brush,measuringMode);
	return S_OK;
}
HRESULT DWriteDrawer::GetPixelsPerDip(void * clientDrawingContext,
	_Out_ FLOAT * pixelsPerDip)
{
	DWriteDrawerContext * drawingContext =
		static_cast<DWriteDrawerContext *>(clientDrawingContext);

	float dpiX, dpiY;
	drawingContext->renderTarget->GetDpi(&dpiX, &dpiY);
	*pixelsPerDip = dpiX / 96;
	return S_OK;
}

HRESULT DWriteDrawer::GetCurrentTransform(void * clientDrawingContext,
	DWRITE_MATRIX * transform)
{
	DWriteDrawerContext * drawingContext =
		static_cast<DWriteDrawerContext *>(clientDrawingContext);
	drawingContext->renderTarget->GetTransform((D2D1_MATRIX_3X2_F *)transform);
	return S_OK;
}
HRESULT DWriteDrawer::IsPixelSnappingDisabled(
	void * clientDrawingContext,
	BOOL * isDisabled
) {
	*isDisabled=FALSE;
	return S_OK;
}
 void DWriteDrawer::Create(ID2D1Factory* factory, DWriteDrawer** render) {
	 auto r = new DWriteDrawer(factory);
	 r->AddRef();
	 *render = r;
}
HRESULT DWriteDrawer::DrawUnderline(
	void * clientDrawingContext,
	FLOAT  baselineOriginX,
	FLOAT  baselineOriginY,
	const DWRITE_UNDERLINE * underline,
	IUnknown * clientDrawingEffect
) {
	if (clientDrawingContext == nullptr) {
		return E_INVALIDARG;
	}
	auto context = static_cast<DWriteDrawerContext*>(clientDrawingContext);
	if (context->dafaultEffect == nullptr || context->renderTarget == nullptr) {
		return E_INVALIDARG;
	}
	auto* effect = static_cast<DWriteDrawerEffect*>(clientDrawingEffect);
	if (effect == nullptr) {
		effect = context->dafaultEffect.Get();
	}

	auto u = effect->underline.get();
	if (u != nullptr) {
		D2D_POINT_2F p0;
		D2D_POINT_2F p1;
		//+-1‚Íü‚ÉŒ„ŠÔ‚ðÝ‚¯‚é‚½‚ß
		if (underline->readingDirection == DWRITE_READING_DIRECTION_TOP_TO_BOTTOM|| underline->readingDirection==DWRITE_READING_DIRECTION_BOTTOM_TO_TOP) {
			p0 = { baselineOriginX + underline->offset, baselineOriginY+1 };
			p1 = { baselineOriginX + underline->offset, baselineOriginY + underline->width-1 };
		}
		else {
			p0 = { baselineOriginX+1, baselineOriginY + underline->offset };
			p1 = { baselineOriginX + underline->width-1, baselineOriginY + underline->offset };
		}
		switch (u->lineStyle) {
		case LineStyle_Solid:
			m_line->DrawSolidLine(context->renderTarget, p0, p1, u->lineColor.Get(), u->boldLine ? underline->thickness * 2 : underline->thickness);
			break;
		case LineStyle_Dot:
			m_line->DrawDOTLine(context->renderTarget, p0, p1, u->lineColor.Get(), u->boldLine ? underline->thickness * 2 : underline->thickness);
			break;
		case LineStyle_Dash:
			m_line->DrawDashLine(context->renderTarget, p0, p1, u->lineColor.Get(), u->boldLine ? underline->thickness * 2 : underline->thickness);
			break;
		case LineStyle_Squiggle:
		{
			if (underline->readingDirection == DWRITE_READING_DIRECTION_TOP_TO_BOTTOM || underline->readingDirection == DWRITE_READING_DIRECTION_BOTTOM_TO_TOP) {
				throw "NotImpl";
			}
			else
			{
				Microsoft::WRL::ComPtr<ID2D1PathGeometry> pathGeometry;
				FailToThrowHR(m_factory->CreatePathGeometry(&pathGeometry));
				Microsoft::WRL::ComPtr<ID2D1GeometrySink> geometrySink;
				FailToThrowHR(pathGeometry->Open(&geometrySink));
				float amplitude = 1 * underline->thickness;
				float period = 5 * underline->thickness;
				float xOffset = baselineOriginX;
				float yOffset = baselineOriginY + underline->offset;
				for (float t = 0; t < underline->width; t++)
				{
					float x = xOffset + t;
					auto angle =  static_cast<float>(pi*2)*std::fmod(x, period) / period;
					auto y = yOffset + amplitude * std::sin(angle);
					D2D1_POINT_2F pt = { x, y };

					if (t == 0)
						geometrySink->BeginFigure(pt, D2D1_FIGURE_BEGIN_HOLLOW);
					else
						geometrySink->AddLine(pt);
				}

				geometrySink->EndFigure(D2D1_FIGURE_END_OPEN);

				FailToThrowHR(geometrySink->Close());

				context->renderTarget->DrawGeometry(pathGeometry.Get(),
					u->lineColor.Get(),
					underline->thickness);
			}
		}
		break;
		}
	}
	return S_OK;
}

HRESULT DWriteDrawer::DrawStrikethrough(
	void * clientDrawingContext,
	FLOAT  baselineOriginX,
	FLOAT  baselineOriginY,
	const DWRITE_STRIKETHROUGH * strikethrough,
	IUnknown * clientDrawingEffect
) {
	if (clientDrawingContext == nullptr)
	{
		return E_INVALIDARG;
	}
	auto context = static_cast<DWriteDrawerContext*>(clientDrawingContext);
	if (context->dafaultEffect == nullptr || context->renderTarget == nullptr)
	{
		return E_INVALIDARG;
	}
	D2D1_POINT_2F start;

	D2D1_POINT_2F end;
	if (strikethrough->readingDirection == DWRITE_READING_DIRECTION_TOP_TO_BOTTOM || strikethrough->readingDirection == DWRITE_READING_DIRECTION_BOTTOM_TO_TOP)
	{
		start = { baselineOriginX + strikethrough->offset, baselineOriginY  };
		end = { baselineOriginX + strikethrough->offset,baselineOriginY + strikethrough->width };
	}
	else
	{
		start = { baselineOriginX, baselineOriginY + strikethrough->offset };
		end={ baselineOriginX + strikethrough->width,baselineOriginY + strikethrough->offset };

	}
	context->renderTarget->DrawLine(start, end, context->dafaultEffect->textColor.Get(),strikethrough->thickness);
	return S_OK;
}
HRESULT DWriteDrawer::DrawInlineObject(
	void * clientDrawingContext,
	FLOAT  originX,
	FLOAT  originY,
	IDWriteInlineObject * inlineObject,
	BOOL  isSideways,
	BOOL  isRightToLeft,
	IUnknown * clientDrawingEffect
) {
	return inlineObject->Draw(clientDrawingContext,
		this,
		originX,
		originY,
		isSideways,
		isRightToLeft,
		clientDrawingEffect);
}