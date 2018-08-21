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
		OutputDebugStringW(self->InputtingString().c_str());
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;

}
void ConsoleWindow::Init(int x, int y, int w, int h, HMENU m, ID2D1Factory* d2d_f, IDWriteFactory* dwrite_f) {
	FailToThrowHR(m_threadmgr->CreateDocumentMgr(&m_docmgr));
	FailToThrowHR(m_docmgr->CreateContext(m_clientId, 0, dynamic_cast<ITextStoreACP*>(this), &m_context, &m_edit_cookie));
	FailToThrowHR(m_context->GetProperty(GUID_PROP_ATTRIBUTE, &m_attr_prop));
	FailToThrowHR(m_context->GetProperty(GUID_PROP_COMPOSING, &m_composition_prop));
	FailToThrowB(RegisterConsoleWindowClass(m_hinst));
	m_hwnd = CreateWindowEx(0, className, NULL, WS_OVERLAPPED | WS_CHILD | WS_VISIBLE, x, y, w, h, m_parentHwnd, m, m_hinst, this);
	m_d2d = Direct2DWithHWnd::Create(d2d_f, m_hwnd);
	tignear::tsf::TsfDWriteDrawer::Create(m_d2d->GetFactory(), &m_drawer);
	m_tbuilder = std::make_unique<TextBuilder>(dwrite_f,
		L"Cica",
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_SEMI_EXPANDED,
		static_cast<FLOAT>(16),
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
	m_console->shell->InputChar(wp);

	TCHAR c = static_cast<TCHAR>(wp);
#ifdef UNICODE
	wchar_t wc = c;
#else
	wchar_t wc = std::btowc(c);
#endif
	CallWithAppLock(true, [this, wc]()->void {
		switch (wc) {
		case '\b'://back space
		case VK_RETURN:
			return;
		default:
			if (SelectionStart() == SelectionEnd())
			{
				if (UseTerminalEchoBack()) {
					InputtingString().insert(InputtingString().begin() + SelectionStart(), wc);
					SelectionStart()++;
					SelectionEnd()++;
				}

			}
			else
			{
				InputtingString().replace(InputtingString().begin() + SelectionStart(), InputtingString().begin() + SelectionEnd(), 1, wc);
				SelectionStart()++;
				SelectionEnd()=SelectionStart();
				ActiveSelEnd()=TS_AE_NONE;
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
		if (SelectionStart() != SelectionEnd())
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
LONG& ConsoleWindow::SelectionStart() {
	return m_console->inputarea_selection_start;
}
LONG& ConsoleWindow::SelectionEnd() {
	return m_console->inputarea_selection_end;
}
TsActiveSelEnd& ConsoleWindow::ActiveSelEnd() {
	return m_console->selend;
}
std::wstring& ConsoleWindow::InputtingString() {
	return m_console->input_string;
}
bool& ConsoleWindow::InterimChar() {
	return m_console->interim_char;
}
bool ConsoleWindow::UseTerminalEchoBack() {
	return m_console->use_terminal_echoback;
}
void ConsoleWindow::OnKeyDown(WPARAM param) {

	CallWithAppLock(true, [this,param]() {
		if (GetKeyState(VK_LEFT) & 0x80) 
		{
			if (GetKeyState(VK_SHIFT) & 0x80)
			{
				if (SelectionStart() == SelectionEnd())
				{
					if (SelectionStart() == 0)
					{
						return;
					}
					SelectionStart()--;
					ActiveSelEnd()= TS_AE_START;
				}
				else
				{
					switch (ActiveSelEnd()) {
					case TS_AE_NONE:
						if (SelectionStart() == 0)
						{
							return;
						}
						SelectionStart() -- ;
						ActiveSelEnd()=TS_AE_START;
						break;
					case TS_AE_START:
						if (SelectionStart() == 0)
						{
							return;
						}
						SelectionStart()--;
						break;
					case TS_AE_END:
						SelectionEnd()--;
						if (SelectionStart() == SelectionEnd())
						{
							ActiveSelEnd()= TS_AE_NONE;
						}
						break;
					}
				}
			}
			else
			{
				if (SelectionStart() != SelectionEnd()) 
				{
					auto cnt = SelectionEnd() - SelectionStart();
					if (ActiveSelEnd() == TS_AE_START) {
						m_console->shell->InputKey(VK_LEFT, cnt);
					}
					SelectionEnd()=SelectionStart();
					ActiveSelEnd()=TS_AE_NONE;
				}
				else if (SelectionStart() == 0 || SelectionEnd() == 0)
				{
					return;
				}
				else
				{
					SelectionStart()--;
					SelectionEnd() --;
					m_console->shell->InputKey(VK_LEFT);
				}
			}
			m_sink->OnSelectionChange();
			InvalidateRect(m_hwnd, NULL, FALSE);
		}
		else if (GetKeyState(VK_RIGHT) & 0x80)
		{
			if (GetKeyState(VK_SHIFT) & 0x80) 
			{
				if (SelectionStart() == SelectionEnd())
				{
					if (SelectionEnd() == InputtingString().length())
					{
						return;
					}
					SelectionEnd()++;
					ActiveSelEnd()=TS_AE_END;
				}
				else
				{
					switch (ActiveSelEnd()) {
					case TS_AE_NONE:
						if (InputtingString().length() == SelectionEnd())
						{
							return;
						}
						SelectionEnd()++;
						ActiveSelEnd()=TS_AE_END;
						break;
					case TS_AE_END:
						if (SelectionEnd() == InputtingString().length())
						{
							return;
						}
						SelectionEnd()++;
						break;
					case TS_AE_START:
						SelectionStart()++;
						if (SelectionStart() == SelectionEnd())
						{
							ActiveSelEnd()=TS_AE_NONE;
						}
					}
				}
			}
			else
			{
				if (SelectionStart() != SelectionEnd())
				{
					auto cnt = SelectionEnd() - SelectionStart();
					if (ActiveSelEnd() == TS_AE_END) {
						m_console->shell->InputKey(VK_RIGHT,cnt);
					}
					SelectionStart()=SelectionEnd();
					ActiveSelEnd()=TS_AE_NONE;
					//do not send key
				}
				else if (SelectionEnd() == InputtingString().length())
				{
					//do not send key
					return;
				}
				else
				{
					SelectionStart() ++; 
					SelectionEnd() ++;
					m_console->shell->InputKey(VK_RIGHT);
				}
			}
			m_sink->OnSelectionChange();
			InvalidateRect(m_hwnd, NULL, FALSE);
		}
		else if (GetKeyState(VK_DELETE)&0x80) {
			if (InputtingString().empty())
			{
				return;
			}
			if (SelectionStart() == SelectionEnd())
			{
				if (SelectionEnd() == InputtingString().length())
				{
					return;
				}
				if (UseTerminalEchoBack()) {
					InputtingString().erase(SelectionStart(), 1);
					m_console->shell->InputKey(VK_RIGHT);
					m_console->shell->InputKey(VK_BACK);
				}

			}
			else
			{
				auto cnt = SelectionEnd() - SelectionStart();
				InputtingString().erase(SelectionStart(), cnt);
				if (ActiveSelEnd() == TS_AE_END) {
					m_console->shell->InputKey(VK_RIGHT, cnt);
				}
				m_console->shell->InputKey(VK_BACK,cnt);

				SelectionEnd()=SelectionStart();
				ActiveSelEnd()=TS_AE_NONE;
			}
			InvalidateRect(m_hwnd, NULL, FALSE);
		}
		else if (GetKeyState(VK_BACK) & 0x80) {
			if (!InputtingString().empty())
			{
				if (SelectionStart() == SelectionEnd())
				{
					if (SelectionStart() == 0)
					{
						return;
					}
					if (UseTerminalEchoBack()) {
						SelectionStart()--;
						SelectionEnd()--;
						InputtingString().erase(SelectionStart(), 1);
						m_console->shell->InputKey(VK_BACK);
					}

				}
				else
				{
					auto cnt = SelectionEnd() - SelectionStart();
					InputtingString().erase(SelectionStart(), cnt);
					SelectionEnd() = SelectionStart();
					if (ActiveSelEnd() == TS_AE_END) {
						m_console->shell->InputKey(VK_RIGHT, cnt);
					}
					m_console->shell->InputKey(VK_BACK, cnt);
					ActiveSelEnd() = TS_AE_NONE;
				}
			}
			InvalidateRect(m_hwnd, NULL, FALSE);
		}
		else if (GetKeyState(VK_RETURN)&0x80) {
			if (UseTerminalEchoBack()) {
				InputtingString() += L'\n';
			}
			ConfirmCommand();
			m_console->shell->InputKey(VK_RETURN);
			InvalidateRect(m_hwnd, NULL, FALSE);
		}
		else {
			m_console->shell->InputKey(param);
			InvalidateRect(m_hwnd, NULL, FALSE);
		}
	});
}
void ConsoleWindow::ConfirmCommand() {
	if (UseTerminalEchoBack()) {
		m_console->shell->ConfirmString(InputtingString());
	}
	SelectionStart() = 0;
	SelectionEnd() = 0;
	auto oldEnd = static_cast<LONG>(InputtingString().length());
	InputtingString().clear();
	TS_TEXTCHANGE change{ 0,oldEnd,0 };
	m_sink->OnTextChange(0, &change);
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
		ComPtr<ID2D1SolidColorBrush> textColor;
		t->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Black, 1.0f),
			&textColor
		);

		static auto clearColor = D2D1::ColorF(D2D1::ColorF::LightPink, 1.0f);
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
		std::wstring shellstr;
		auto end = m_console->shell->GetViewTextEnd();
		auto endb = end;
		endb--;
		for (auto itr = m_console->shell->GetViewTextBegin(); itr !=end; itr++) {
			for (auto elem:(*itr)) {
				shellstr += elem.textW();
				auto w= elem.textW();
				OutputDebugStringW(std::wstring(w).c_str());
			}
		}

		auto lengthShell = static_cast<UINT32>(shellstr.length());
		std::wstring ftext;
		ftext.reserve(shellstr.length() + InputtingString().length());
		ftext += shellstr;
		ftext += InputtingString();
		auto layout = m_tbuilder->CreateTextLayout(ftext, static_cast<FLOAT>(rc.right - rc.left), static_cast<FLOAT>(rc.bottom - rc.top));

		//draw caret
		//https://stackoverflow.com/questions/28057369/direct2d-createtextlayout-how-to-get-caret-coordinates
		if (m_caret_display) {
			DWRITE_HIT_TEST_METRICS hitTestMetrics;
			bool isTrailingHit = false;
			float caretX, caretY;
			FailToThrowHR(layout->HitTestTextPosition(
				lengthShell+SelectionStart(),
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
		else if (SelectionStart() != SelectionEnd())
		{
			UINT32 count;
			layout->HitTestTextRange(SelectionStart(), SelectionEnd() - SelectionStart(), 0, 0, NULL, 0, &count);

			std::unique_ptr<DWRITE_HIT_TEST_METRICS[]> mats(new DWRITE_HIT_TEST_METRICS[count]);
			FailToThrowHR(layout->HitTestTextRange(lengthShell+SelectionStart(), SelectionEnd() -SelectionStart(), 0, 0, mats.get(), count, &count));

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
					convertColor(attr.crText, t, textColor.Get()).Get(),
					attr.lsStyle == TF_LS_NONE ? std::unique_ptr<TsfDWriteDrawerEffectUnderline>() : std::make_unique<TsfDWriteDrawerEffectUnderline>(convertLineStyle(attr.lsStyle),
					static_cast<bool>(attr.fBoldLine), 
					convertColor(attr.crLine, t, textColor.Get()).Get())
				);
				ComPtr<ITfRangeACP> rangeAcp;
				range.As(&rangeAcp);
				LONG start, length;
				if (FAILED(rangeAcp->GetExtent(&start, &length))) {
					continue;
				}
				DWRITE_TEXT_RANGE write_range{ static_cast<UINT32>(start+lengthShell), static_cast<UINT32>(length) };
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
			textColor.Get(),
			std::unique_ptr<TsfDWriteDrawerEffectUnderline>() 
		};
		auto context = std::make_unique<TsfDWriteDrawerContext>(t, defaultEffect.Get());

		FailToThrowHR(layout->Draw(context.get(), m_drawer.Get(), 0, 0));


		FailToThrowHR(t->EndDraw());

		EndPaint(m_hwnd, &pstruct);
	});
}
void ConsoleWindow::SetConsoleContext(std::shared_ptr<ConsoleContext> console) {
	m_console = console;
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
	r->m_caret_update_time = std::chrono::steady_clock::now();
	r->Init(x, y, w, h, hmenu, d2d_f, dwrite_f);
	*pr = r;
}
//static fields
bool ConsoleWindow::m_registerState = false;
