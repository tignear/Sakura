#include "stdafx.h"
#include "FailToThrow.h"
#include "ConsoleWindow.h"
using tignear::sakura::ConsoleWindow;
using Microsoft::WRL::ComPtr;
using tignear::dwrite::TextBuilder;
using namespace tignear::tsf;
bool ConsoleWindow::RegisterConsoleWindowClass(HINSTANCE hinst) {
	if (m_registerState) return true;
	WNDCLASSEX wcex;

	ZeroMemory((LPVOID)&wcex, sizeof(WNDCLASSEX));

	//ウィンドウクラスを登録
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hinst;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = className;
	wcex.hIconSm = NULL;
	if (FAILED(RegisterClassEx(&wcex)))return false;
	return true;
}
LRESULT CALLBACK ConsoleWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static ConsoleWindow* self;
	if (self) {
		self->CaretUpdate();
	}
	switch (message)
	{
	case WM_CREATE:
		self = static_cast<ConsoleWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		break;
	case WM_TIMER:
		self->OnTimer();
		break;
	case WM_SETFOCUS:
		self->OnSetFocus();
		break;
	case WM_KEYDOWN:
		self->OnKeyDown(wParam);
		break;
	case WM_CHAR:
		self->OnChar(wParam);
		break;
	case WM_PAINT:
		self->OnPaint();
		return 0;
	case WM_SIZE:
		self->OnSize();
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		OutputDebugStringW(self->m_string.c_str());
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;

}
void ConsoleWindow::Init(int x, int y, int w, int h, HMENU m, ID2D1Factory* d2d_f, IDWriteFactory* dwrite_f) {
	FailToThrowHR(m_threadmgr->CreateDocumentMgr(&m_docmgr));
	FailToThrowHR(m_docmgr->CreateContext(m_clientId, 0, dynamic_cast<ITextStoreACP*>(this), &m_context, &m_edit_cookie));
	FailToThrowHR(m_context->GetProperty(GUID_PROP_ATTRIBUTE, &m_attr_prop));
	FailToThrowB(RegisterConsoleWindowClass(m_hinst));
	m_hwnd = CreateWindowEx(0, className, NULL, WS_OVERLAPPED | WS_CHILD | WS_VISIBLE, x, y, w, h, m_parentHwnd, m, m_hinst, this);
	m_d2d = Direct2DWithHWnd::Create(d2d_f, m_hwnd);
	tignear::tsf::TsfDWriteDrawer::Create(m_d2d->GetFactory(), &m_drawer);
	m_tbuilder = std::make_unique<TextBuilder>(dwrite_f,
		L"メイリオ",
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_SEMI_EXPANDED,
		static_cast<FLOAT>(72),
		L"ja-jp");
	FailToThrowHR(m_docmgr->Push(m_context.Get()));
	SetTimer(m_hwnd, CallAsyncTimerId, 60u, NULL);
}
void ConsoleWindow::OnSetFocus() {
	OutputDebugString(_T("ConsoleWindow::OnSetFocus\n"));
	if (FAILED(m_threadmgr->SetFocus(m_docmgr.Get()))) {
		OutputDebugString(_T("ConsoleWindow::OnSetFocus Fail\n"));
	}
}
void ConsoleWindow::OnSize() {
	if (m_d2d)
	{
		m_d2d->ReSize();
	}
	if (m_sink)
	{
		m_sink->OnLayoutChange(TS_LC_CHANGE, 0UL);
	}

}
void ConsoleWindow::OnChar(WPARAM wp) {
	TCHAR c = static_cast<TCHAR>(wp);
#ifdef UNICODE
	wchar_t wc = c;
#else
	wchar_t wc = std::btowc(c);
#endif
	CallWithAppLock(true, [this, wc]()->void {
		switch (wc) {
		case 0x08://back space
			if (!m_string.empty())
			{
				if (m_selection_start == m_selection_end)
				{
					if (m_selection_start == 0)
					{
						return;
					}
					m_selection_start--;
					m_selection_end--;
					m_string.erase(m_selection_start, 1);
				}
				else
				{
					m_string.erase(m_selection_start, m_selection_end - m_selection_start);
					m_selection_end = m_selection_start;
					m_active_sel_end = TS_AE_NONE;
				}
			}
			break;
		default:
			if (m_selection_start != m_selection_end)
			{
				m_string.replace(m_string.begin() + m_selection_start, m_string.begin() + m_selection_end, 1, wc);
				m_selection_start++;
				m_selection_end = m_selection_start;
				m_active_sel_end = TS_AE_NONE;
			}
			else
			{
				m_string.insert(m_string.begin() + m_selection_start, wc);
				m_selection_end++;
				m_selection_start++;
			}

		}
		m_sink->OnSelectionChange();
		UpdateText();
	});
}
void ConsoleWindow::OnTimer()
{
	CallAsync();
}
void ConsoleWindow::CaretUpdate() 
{
	PushAsyncCallQueue(false, [this]() {
		using namespace std::chrono;
		if (m_selection_start != m_selection_end)
		{
			if (m_caret_display)
			{
				m_caret_display = false;
				InvalidateRect(m_hwnd, NULL, FALSE);
			}
			return;
		}
		auto blinktime_mills = GetCaretBlinkTime();
		if (-1 == blinktime_mills)
		{
			return;
		}
		auto now = steady_clock::now();
		auto time = duration_cast<milliseconds>(now - m_caret_update_time);
		if (time.count() >= blinktime_mills)
		{
			m_caret_display = !m_caret_display;
			m_caret_update_time = now;
			InvalidateRect(m_hwnd, NULL, FALSE);
		}
	});
}
void ConsoleWindow::OnKeyDown(WPARAM param) {
	CallWithAppLock(true, [this]() {
		if (GetKeyState(VK_LEFT) & 0x80) 
		{
			if (GetKeyState(VK_SHIFT) & 0x80)
			{
				if (m_selection_start == m_selection_end)
				{
					if (m_selection_start == 0)
					{
						return;
					}
					m_selection_start--;
					m_active_sel_end = TS_AE_START;
				}
				else
				{
					switch (m_active_sel_end) {
					case TS_AE_NONE:
						if (m_selection_start == 0)
						{
							return;
						}
						m_selection_start--;
						m_active_sel_end = TS_AE_START;
						break;
					case TS_AE_START:
						if (m_selection_start == 0)
						{
							return;
						}
						m_selection_start--;
						break;
					case TS_AE_END:
						m_selection_end--;
						if (m_selection_start == m_selection_end)
						{
							m_active_sel_end = TS_AE_NONE;
						}
						break;
					}
				}
			}
			else
			{
				if (m_selection_start != m_selection_end) 
				{
					m_selection_end = m_selection_start;
					m_active_sel_end = TS_AE_NONE;
				}
				else if (m_selection_start == 0 || m_selection_end == 0)
				{
					return;
				}
				else
				{
					m_selection_start--;
					m_selection_end--;
				}
			}
			m_sink->OnSelectionChange();
			InvalidateRect(m_hwnd, NULL, FALSE);

		}
		else if (GetKeyState(VK_RIGHT) & 0x80)
		{
			if (GetKeyState(VK_SHIFT) & 0x80) 
			{
				if (m_selection_start == m_selection_end)
				{
					if (m_selection_end == m_string.length())
					{
						return;
					}
					m_selection_end++;
					m_active_sel_end = TS_AE_END;
				}
				else
				{
					switch (m_active_sel_end) {
					case TS_AE_NONE:
						if (m_string.length() == m_selection_end) 
						{
							return;
						}
						m_selection_end++;
						m_active_sel_end = TS_AE_END;
						break;
					case TS_AE_END:
						if (m_selection_end == m_string.length()) 
						{
							return;
						}
						m_selection_end++;
						break;
					case TS_AE_START:
						m_selection_start++;
						if (m_selection_start == m_selection_end)
						{
							m_active_sel_end = TS_AE_NONE;
						}
					}
				}
			}
			else
			{
				if (m_selection_start != m_selection_end)
				{
					m_selection_start = m_selection_end;
					m_active_sel_end = TS_AE_NONE;
				}
				else if (m_selection_end == m_string.length())
				{
					return;
				}
				else
				{
					m_selection_start++;
					m_selection_end++;
				}
			}
			m_sink->OnSelectionChange();
			InvalidateRect(m_hwnd, NULL, FALSE);
		}
		else if (GetKeyState(VK_DELETE)&0x80) {
			if (m_string.empty())
			{
				return;
			}
			if (m_selection_start == m_selection_end)
			{
				if (m_selection_end == m_string.length())
				{
					return;
				}
				m_string.erase(m_selection_start, 1);
			}
			else
			{
				m_string.erase(m_selection_start, m_selection_end - m_selection_start);
				m_selection_end = m_selection_start;
				m_active_sel_end = TS_AE_NONE;
			}
			InvalidateRect(m_hwnd, NULL, FALSE);
		}
	});
}
void ConsoleWindow::UpdateText() {
	InvalidateRect(m_hwnd, NULL, FALSE);
	//OutputDebugStringW((std::wstring(L"length:")+std::to_wstring(m_string.length())+std::wstring(L",acpStart:")+std::to_wstring(m_selection_start)+ std::wstring(L",acpend:") + std::to_wstring(m_selection_end)+std::wstring(L",inherim:")+std::to_wstring(m_InterimChar)+L"\n").c_str());
}
static constexpr const inline LineStyle convertLineStyle(TF_DA_LINESTYLE ls) {
	switch (ls) {
	case TF_LS_SOLID:
		return LineStyle_Solid;
	case TF_LS_DOT:
		return LineStyle_Dot;
	case TF_LS_DASH:
		return LineStyle_Dash;
	case TF_LS_SQUIGGLE:
		return LineStyle_Squiggle;
	default:
		throw "NotSupported";
	}
}
template <class R>
static inline ComPtr<R> convertColor(TF_DA_COLOR& color, ID2D1RenderTarget* t, R* ifNone) {
	ComPtr<R> r;
	switch (color.type) {
	case TF_CT_NONE:
		return ifNone;
	case TF_CT_COLORREF:
		t->CreateSolidColorBrush(D2D1::ColorF(color.cr, 1.0f), &r);
		return r;
	case TF_CT_SYSCOLOR:
		t->CreateSolidColorBrush(D2D1::ColorF(GetSysColor(color.nIndex), 1.0f), &r);
		return r;
	default:
		return ifNone;
	}
}
void ConsoleWindow::OnPaint() {

	CallWithAppLock(false, [this]() {
		PAINTSTRUCT pstruct;
		RECT rc;

		GetClientRect(m_hwnd, &rc);

		auto t = m_d2d->GetRenderTarget();

		t->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
		ComPtr<ID2D1SolidColorBrush> red;
		t->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Red, 1.0f),
			&red
		);
		ComPtr<ID2D1SolidColorBrush> black;
		t->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Black, 1.0f),
			&black
		);

		static auto clearColor = D2D1::ColorF(0xFEEEED, 1.0f);
		ComPtr<ID2D1SolidColorBrush> clearColorBrush;
		t->CreateSolidColorBrush(
			clearColor,
			&clearColorBrush
		);
		ComPtr<ID2D1SolidColorBrush> transparency;
		t->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.0f), &transparency);
		BeginPaint(m_hwnd, &pstruct);

		t->BeginDraw();
		t->Clear(clearColor);

		auto layout = m_tbuilder->CreateTextLayout(m_string, static_cast<FLOAT>(rc.right - rc.left), static_cast<FLOAT>(rc.bottom - rc.top));

		//draw caret
		//https://stackoverflow.com/questions/28057369/direct2d-createtextlayout-how-to-get-caret-coordinates
		if (m_caret_display) {
			DWRITE_HIT_TEST_METRICS hitTestMetrics;
			bool isTrailingHit = false;
			float caretX, caretY;
			FailToThrowHR(layout->HitTestTextPosition(
				m_selection_start,
				isTrailingHit,
				&caretX,
				&caretY,
				&hitTestMetrics
			));
			DWORD caretWidth = 1;
			FailToThrowB(SystemParametersInfo(SPI_GETCARETWIDTH, 0, &caretWidth, 0));
			DWORD halfCaretWidth = caretWidth / 2u;

			t->FillRectangle({
				caretX - halfCaretWidth,
				hitTestMetrics.top,
				caretX + (caretWidth - halfCaretWidth),
				hitTestMetrics.top + hitTestMetrics.height
				}, red.Get());
		}
		else if (m_selection_start != m_selection_end)
		{
			UINT32 count;
			layout->HitTestTextRange(m_selection_start, m_selection_end - m_selection_start, 0, 0, NULL, 0, &count);

			std::unique_ptr<DWRITE_HIT_TEST_METRICS[]> mats(new DWRITE_HIT_TEST_METRICS[count]);
			FailToThrowHR(layout->HitTestTextRange(m_selection_start, m_selection_end - m_selection_start, 0, 0, mats.get(), count, &count));

			for (auto i = 0UL; i < count; i++) 
			{
				t->FillRectangle({
					mats[i].left,
					mats[i].top,
					mats[i].left + mats[i].width,
					mats[i].top + mats[i].height
					}, red.Get());
			}

		}

		//draw string
		ComPtr<IEnumTfRanges> enumRanges;
		FailToThrowHR(m_attr_prop->EnumRanges(m_edit_cookie, &enumRanges, NULL));
		ComPtr<ITfRange> range;
		while (enumRanges->Next(1, &range, NULL) == S_OK) {
			VARIANT var;
			try {
				VariantInit(&var);
				if (!(m_attr_prop->GetValue(m_edit_cookie, range.Get(), &var) == S_OK && var.vt == VT_I4)) {
					continue;
				}
				GUID guid;
				FailToThrowHR(m_category_mgr->GetGUID((TfGuidAtom)var.lVal, &guid));
				ComPtr<ITfDisplayAttributeInfo> dispattrinfo;
				FailToThrowHR(m_attribute_mgr->GetDisplayAttributeInfo(guid, &dispattrinfo, NULL));
				TF_DISPLAYATTRIBUTE attr;
				dispattrinfo->GetAttributeInfo(&attr);
				//attrに属性が入っているので属性に基づいて描画させる
				ComPtr<TsfDWriteDrawerEffect> effect = new TsfDWriteDrawerEffect(
					convertColor(attr.crBk, t, transparency.Get()).Get(),
					convertColor(attr.crText, t, black.Get()).Get(),
					attr.lsStyle == TF_LS_NONE ? std::unique_ptr<TsfDWriteDrawerEffectUnderline>() : std::make_unique<TsfDWriteDrawerEffectUnderline>(convertLineStyle(attr.lsStyle),
					static_cast<bool>(attr.fBoldLine), 
					convertColor(attr.crLine, t, black.Get()).Get())
				);
				ComPtr<ITfRangeACP> rangeAcp;
				range.As(&rangeAcp);
				LONG start, length;
				if (FAILED(rangeAcp->GetExtent(&start, &length))) {
					continue;
				}
				DWRITE_TEXT_RANGE write_range{ static_cast<UINT32>(start), static_cast<UINT32>(length) };
				layout->SetDrawingEffect(effect.Get(), write_range);
				layout->SetUnderline(effect->underline ? TRUE : FALSE, write_range);
				VariantClear(&var);
			}
			catch (...)
			{
				VariantClear(&var);
				throw;
			}
		}

		ComPtr<TsfDWriteDrawerEffect> defaultEffect = new TsfDWriteDrawerEffect{
			transparency.Get(), 
			black.Get(),
			std::unique_ptr<TsfDWriteDrawerEffectUnderline>() 
		};
		auto context = std::make_unique<TsfDWriteDrawerContext>(t, defaultEffect.Get());

		FailToThrowHR(layout->Draw(context.get(), m_drawer.Get(), 0, 0));
		FailToThrowHR(t->EndDraw());

		EndPaint(m_hwnd, &pstruct);
	});
}

void ConsoleWindow::Create(HINSTANCE hinst, HWND pwnd, int x, int y, int w, int h, HMENU hmenu, ITfThreadMgr* threadmgr, TfClientId cid, ITfCategoryMgr* cate_mgr, ITfDisplayAttributeMgr* attr_mgr, ID2D1Factory* d2d_f, IDWriteFactory* dwrite_f, ConsoleWindow** pr) {
	auto r = new ConsoleWindow();
	r->AddRef();
	r->m_hinst = hinst;
	r->m_parentHwnd = pwnd;
	r->m_threadmgr = threadmgr;
	r->m_clientId = cid;
	r->m_category_mgr = cate_mgr;
	r->m_attribute_mgr = attr_mgr;
	r->m_selection_start = 0;
	r->m_selection_end = 0;
	r->m_caret_update_time = std::chrono::steady_clock::now();
	r->Init(x, y, w, h, hmenu, d2d_f, dwrite_f);
	*pr = r;
}
//static fields
bool ConsoleWindow::m_registerState = false;
