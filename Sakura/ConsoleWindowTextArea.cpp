#include "stdafx.h"
#include <iterator>
#include "FailToThrow.h"
#include "ComHolder.h"
#include "ConsoleWindow.h"
using tignear::sakura::ConsoleWindowTextArea;
using Microsoft::WRL::ComPtr;
using tignear::dwrite::TextBuilder;
using namespace tignear::dwrite;
using tignear::sakura::ShellContext;

bool ConsoleWindowTextArea::RegisterConsoleWindowTextAreaClass(HINSTANCE hinst) {
	if (m_registerstate) return true;
	WNDCLASSEX wcex;

	ZeroMemory((LPVOID)&wcex, sizeof(WNDCLASSEX));

	//ÉEÉBÉìÉhÉEÉNÉâÉXÇìoò^
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc =WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hinst;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = m_className;
	wcex.hIconSm = NULL;
	if (FAILED(RegisterClassEx(&wcex)))return false;
	return true;
}


LRESULT CALLBACK ConsoleWindowTextArea::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static ConsoleWindowTextArea* self;
	if (self) {
		self->CaretUpdate();
		self->BlinkUpdate();
	}
	switch (message)
	{
	case WM_CREATE:
		self = static_cast<ConsoleWindowTextArea*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
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
		SetFocus(hwnd);
		break;

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;

}
HRESULT ConsoleWindowTextArea::GetTextExt(
	TsViewCookie vcView,
	LONG         acpStart,
	LONG         acpEnd,
	RECT         *prc,
	BOOL         *pfClipped
) {
	OutputDebugString(_T("TSF:GetTextExt\n"));

	if (!Lock().IsLock(false)) {
		return TS_E_NOLOCK;
	}
	if (acpStart == acpEnd)
	{
		return E_INVALIDARG;
	}

	if (acpStart > acpEnd)
	{
		std::swap(acpStart, acpEnd);
	}
	auto height = GetAreaDip().height;
	auto begin = m_console->shell->GetAll().begin();
	auto nowY = height;
	auto end = m_console->shell->GetView().end();
	FLOAT y = 0;
	FLOAT x = 0;
	if (end != begin) {
		--end;
		auto itr = end;
		for (; itr != begin; --itr) {
			ComPtr<IDWriteTextLayout1> layout = GetLayout(*itr);
			DWRITE_TEXT_METRICS met;
			layout->GetMetrics(&met);
			nowY -= met.height;
			if (nowY < 0.0f) {
				break;
			}
		}
		DWRITE_TEXT_METRICS met;
		auto endlayout = GetLayout(*end);
		endlayout->GetMetrics(&met);
		x = met.width + m_originX;
		if (nowY > 0) {
			y = height - nowY;
		}
		else {
			y = height - met.height;
		}
	}


	HRESULT hr;
	UINT32 count;
	auto layout = GetInputtingStringLayout();
	if (( hr= layout->HitTestTextRange(acpStart, acpEnd- acpStart, x, y, NULL, 0, &count))!=E_NOT_SUFFICIENT_BUFFER) {
		FailToThrowHR(hr);
	}
	auto mats = std::make_unique<DWRITE_HIT_TEST_METRICS[]>(count);
	FailToThrowHR(layout->HitTestTextRange(+ acpStart, acpEnd - acpStart, x, y, mats.get(), count, &count));
	LONG left = LONG_MAX, top = LONG_MAX, right = 0, bottom = 0;
	for (auto i = 0UL; i < count; ++i)
	{
		left = left < mats[i].left ? left : static_cast<LONG>(mats[i].left);
		top = top < mats[i].top ? top : static_cast<LONG>(mats[i].top);
		auto r = static_cast<LONG>(mats[i].left + mats[i].width);
		right = right > r ? right : r;
		auto b = static_cast<LONG>(mats[i].top + mats[i].height);
		bottom = bottom > b ? bottom : b;
	}
	RECT localrc{ left, top, right, bottom };
	*prc = localrc;
	MapWindowPoints(m_textarea_hwnd, 0, reinterpret_cast<POINT*>(prc), 2);
	*pfClipped = FALSE;
	return S_OK;
}
void ConsoleWindowTextArea::Init(DIP x, DIP y, DIP w, DIP h, HMENU m, ID2D1Factory* d2d_f, IDWriteFactory* dwrite_f,std::shared_ptr<tignear::sakura::cwnd::Context> console) {
	FailToThrowHR(m_threadmgr->CreateDocumentMgr(&m_docmgr));
	FailToThrowHR(m_docmgr->CreateContext(m_clientId, 0, dynamic_cast<ITextStoreACP*>(this), &m_context, &m_edit_cookie));
	FailToThrowHR(m_context->GetProperty(GUID_PROP_ATTRIBUTE, &m_attr_prop));
	FailToThrowHR(m_context->GetProperty(GUID_PROP_COMPOSING, &m_composition_prop));
	FailToThrowB(RegisterConsoleWindowTextAreaClass(m_hinst));
	auto&& dpi = m_dpi;
	m_textarea_hwnd = CreateWindowEx(0, m_className, NULL, WS_OVERLAPPED | WS_CHILD | WS_VISIBLE,dpi.Pixel(x), dpi.Pixel(y), dpi.Pixel(w), dpi.Pixel(h), m_parentHwnd, m, m_hinst, this);
	m_d2d = ConsoleWindowTextAreaDirect2DWithHWnd::Create(d2d_f, m_textarea_hwnd);
	tignear::dwrite::DWriteDrawer::Create(m_d2d->GetFactory(), &m_drawer);
	m_tbuilder = std::make_unique<TextBuilder>(dwrite_f,
		console->shell->DefaultFont().c_str(),
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		static_cast<FLOAT>(console->shell->FontSize()),
		L"ja-jp");
	SetConsoleContext(console);
	FailToThrowHR(m_docmgr->Push(m_context.Get()));
	SetTimer(m_textarea_hwnd, CallAsyncTimerId, 60u, NULL);
}
void ConsoleWindowTextArea::OnSetFocus() {
	OutputDebugString(_T("ConsoleWindow::OnSetFocus\n"));
	if (FAILED(m_threadmgr->SetFocus(m_docmgr.Get()))) {
		OutputDebugString(_T("ConsoleWindow::OnSetFocus Fail\n"));
	}
}
void ConsoleWindowTextArea::OnSize() {
	if (m_d2d)
	{
		m_d2d->ReSize();
	}
	if (Sink())
	{
		Sink()->OnLayoutChange(TS_LC_CHANGE, 0UL);
	}
	if (m_console) {
		m_console->shell->SetPageSize(GetPageSize());
		//CalcOrigin();

	}
}
void ConsoleWindowTextArea::OnChar(WPARAM wp) {
	m_console->shell->InputChar(wp);

	TCHAR c = static_cast<TCHAR>(wp);
#ifdef UNICODE
	wchar_t wc = c;
#else
	wchar_t wc = std::btowc(c);
#endif
	CallWithAppLock(true, [this, wc]()->void {
		InputtingString([this, wc](auto& str) {
			Selection([this, wc, &str](auto& start, auto& end, auto& ase, auto&) {
				switch (wc) {
				case '\b'://back space
				case VK_RETURN:
					return;
				default:
					if (start == end)
					{
						if (UseTerminalEchoBack()) {
							str.insert(str.begin() + start, wc);
							++start;
							++end;
						}
					}
					else
					{
						str.replace(str.begin() + start, str.begin() + end, 1, wc);
						++start;
						end = start;
						ase = TS_AE_NONE;
					}
				}

			});
		});
		Sink()->OnSelectionChange();
		UpdateText();
	});
}
void ConsoleWindowTextArea::OnTimer()
{
	CallAsync();
}
void ConsoleWindowTextArea::CaretUpdate()
{
	PushAsyncCallQueue(false, [this]() {
		using namespace std::chrono;
		if (SelectionStart() != SelectionEnd())
		{
			if (m_caret_display)
			{
				m_caret_display = false;
				InvalidateRect(m_textarea_hwnd, NULL, FALSE);
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
			InvalidateRect(m_textarea_hwnd, NULL, FALSE);
		}
	});
}
void ConsoleWindowTextArea::BlinkUpdate() {
	using namespace std::chrono;
	auto now = steady_clock::now();
	{
	auto time = duration_cast<milliseconds>(now - m_slow_blink_update_time);
	if (time.count() >= 800) {
		m_slow_blink_display = !m_slow_blink_display;
		m_slow_blink_update_time = now;
	}
	}
	auto time = duration_cast<milliseconds>(now - m_fast_blink_update_time);
	if (time.count() >= 300) {
		m_fast_blink_display = !m_fast_blink_display;
		m_fast_blink_update_time = now;
	}
}
void ConsoleWindowTextArea::Selection(std::function<void(LONG&,LONG&,TsActiveSelEnd&,bool&)> f) {
	f(m_console->textarea_context.inputarea_selection_start,m_console->textarea_context.inputarea_selection_end,m_console->textarea_context.selend,m_console->textarea_context.interim_char);
};
void ConsoleWindowTextArea::Selection(std::function<void(LONG&, LONG&)> f) {
	f(m_console->textarea_context.inputarea_selection_start, m_console->textarea_context.inputarea_selection_end);
}

void ConsoleWindowTextArea::InputtingString(std::function<void(std::wstring&)> f) {
	f(m_console->textarea_context.input_string);
};
const std::wstring& ConsoleWindowTextArea::InputtingString()const {
	return m_console->textarea_context.input_string;
}
LONG ConsoleWindowTextArea::SelectionStart()const {
	return m_console->textarea_context.inputarea_selection_start;
}
LONG ConsoleWindowTextArea::SelectionEnd()const {
	return m_console->textarea_context.inputarea_selection_end;
}
TsActiveSelEnd ConsoleWindowTextArea::ActiveSelEnd()const {
	return m_console->textarea_context.selend;
}
bool ConsoleWindowTextArea::InterimChar()const {
	return m_console->textarea_context.interim_char;
}
bool ConsoleWindowTextArea::UseTerminalEchoBack() {
	return m_console->shell->UseTerminalEchoBack();
}
void ConsoleWindowTextArea::OnDpiChange() {
	m_d2d->GetRenderTarget()->SetDpi(static_cast<FLOAT>(m_dpi.GetDpi()), static_cast<FLOAT>(m_dpi.GetDpi()));
}
void ConsoleWindowTextArea::OnKeyDown(WPARAM param) {

	CallWithAppLock(true, [this, param]() {
		InputtingString([this, param](auto&str) {
			Selection([this,param,&str](auto& start, auto& end, auto& ase, auto& interim) {
			

				if (GetKeyState(VK_LEFT) & 0x80)
				{
					if (GetKeyState(VK_SHIFT) & 0x80)
					{
						if (start == end)
						{
							if (start == 0)
							{
								return;
							}
							--start;
							ase = TS_AE_START;
						}
						else
						{
							switch (ase) {
							case TS_AE_NONE:
								if (start == 0)
								{
									return;
								}
								--start;
								ase = TS_AE_START;
								break;
							case TS_AE_START:
								if (start == 0)
								{
									return;
								}
								--start;
								break;
							case TS_AE_END:
								--end;
								if (start == end)
								{
									ase= TS_AE_NONE;
								}
								break;
							}
						}
					}
					else
					{
						if (start !=end)
						{
							auto cnt = end -start;
							if (ase == TS_AE_START) {
								m_console->shell->InputKey(VK_LEFT, cnt);
							}
							end = start;
							ase = TS_AE_NONE;
						}
						else if (start == 0 || end== 0)
						{
							m_console->shell->InputKey(VK_LEFT);
							return;
						}
						else
						{
							--start;
							--end;
							m_console->shell->InputKey(VK_LEFT);
						}
					}
					Sink()->OnSelectionChange();
					InvalidateRect(m_textarea_hwnd, NULL, FALSE);
				}
				else if (GetKeyState(VK_RIGHT) & 0x80)
				{
					if (GetKeyState(VK_SHIFT) & 0x80)
					{
						if (start == end)
						{
							if (end == str.length())
							{
								return;
							}
							++end;
							ase= TS_AE_END;
						}
						else
						{
							switch (ase) {
							case TS_AE_NONE:
								if (str.length() == end)
								{
									return;
								}
								++end;
								ase = TS_AE_END;
								break;
							case TS_AE_END:
								if (end == str.length())
								{
									return;
								}
								++end;
								break;
							case TS_AE_START:
								++start;
								if (start ==end)
								{
									ase = TS_AE_NONE;
								}
							}
						}
					}
					else
					{
						if (start != end)
						{
							auto cnt = end - start;
							if (ase == TS_AE_END) {
								m_console->shell->InputKey(VK_RIGHT, cnt);
							}
							start= end;
							ase= TS_AE_NONE;
						}
						else if (end == str.length())
						{
							m_console->shell->InputKey(VK_RIGHT);
							return;
						}
						else
						{
							++start;
							++end;
							m_console->shell->InputKey(VK_RIGHT);
						}
					}
					Sink()->OnSelectionChange();
					InvalidateRect(m_textarea_hwnd, NULL, FALSE);
				}
				else if (GetKeyState(VK_DELETE) & 0x80) {
					if (str.empty())
					{
						m_console->shell->InputKey(VK_DELETE);
						return;
					}
					if (start == end)
					{
						if (end == str.length())
						{
							return;
						}
						if (UseTerminalEchoBack()) {
							str.erase(start, 1);
							m_console->shell->InputKey(VK_RIGHT);
							m_console->shell->InputKey(VK_BACK);
						}

					}
					else
					{
						auto cnt = end - start;
						str.erase(start, cnt);
						if (ase == TS_AE_END) {
							m_console->shell->InputKey(VK_RIGHT, cnt);
						}
						m_console->shell->InputKey(VK_BACK, cnt);

						end = start;
						ase = TS_AE_NONE;
					}
					InvalidateRect(m_textarea_hwnd, NULL, FALSE);
				}
				else if (GetKeyState(VK_BACK) & 0x80) {
					if (!str.empty())
					{
						if (start== end)
						{
							if (start == 0)
							{
								return;
							}
							if (UseTerminalEchoBack()) {
								--start;
								--end;
								str.erase(start, 1);
								m_console->shell->InputKey(VK_BACK);
							}

						}
						else
						{
							auto cnt = end -start;
							str.erase(start, cnt);
							end = SelectionStart();
							if (ActiveSelEnd() == TS_AE_END) {
								m_console->shell->InputKey(VK_RIGHT, cnt);
							}
							m_console->shell->InputKey(VK_BACK, cnt);
							ase = TS_AE_NONE;
						}
					}
					else {
						m_console->shell->InputKey(VK_BACK);
					}
					InvalidateRect(m_textarea_hwnd, NULL, FALSE);
				}
				else if (GetKeyState(VK_RETURN) & 0x80) {
					if (UseTerminalEchoBack()) {
						str += L'\n';
					}
					ConfirmCommand();
					m_console->shell->InputKey(VK_RETURN);
					InvalidateRect(m_textarea_hwnd, NULL, FALSE);
				}
				else {
					m_console->shell->InputKey(param);
					InvalidateRect(m_textarea_hwnd, NULL, FALSE);
				}
			});
		});
	});
}
void ConsoleWindowTextArea::ConfirmCommand() {
	if (UseTerminalEchoBack()) {
		m_console->shell->Lock();
		m_console->shell->ConfirmString(InputtingString());
		m_console->shell->Unlock();

	}
	Selection([](auto&start,auto&end) {
		start = 0;
		end = 0;
	});

	auto oldEnd = static_cast<LONG>(InputtingString().length());
	InputtingString([](auto& str) {
		str.clear();
	});
	TS_TEXTCHANGE change{ 0,oldEnd,0 };
	Sink()->OnTextChange(0, &change);
}
void ConsoleWindowTextArea::UpdateText() {
	ResetInputtingStringLayout();
	PostMessage(m_parentHwnd, ConsoleWindow::WM_UPDATE_SCROLLBAR, 0, 0);
	InvalidateRect(m_textarea_hwnd, NULL, FALSE);
}
void ConsoleWindowTextArea::UpdateText(ShellContext* c, std::vector<ShellContext::TextUpdateInfoLine> vec) {
	auto end = c->GetAll().end();
	for (auto&& e : vec) {
		if (e.status() == ShellContext::TextUpdateStatus::ERASE) {
			auto layout=GetLayout(e.line());
			DWRITE_TEXT_METRICS met{};
			layout->GetMetrics(&met);
			if (m_text_width <= met.width) {
				m_text_width = 0;
				for (auto&& e2:c->GetAll()) {
					met = {};
					GetLayout(e2)->GetMetrics(&met);
					if (e.line() != e2 && m_text_width < met.width) {

					}
				}
			}
		}
		else {
			auto layout = BuildLayout(e.line());
			e.line().resource() = std::make_shared<com::ComHolder<IDWriteTextLayout1>>(layout);
			DWRITE_TEXT_METRICS met{};
			layout->GetMetrics(&met);
			if (m_text_width < met.width) {
				m_text_width = met.width;
			}
		}
	}
	PostMessage(m_parentHwnd, ConsoleWindow::WM_UPDATE_SCROLLBAR, 0, 0);
	InvalidateRect(m_textarea_hwnd, NULL, FALSE);
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
		std::terminate();
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
Microsoft::WRL::ComPtr<IDWriteTextLayout1> ConsoleWindowTextArea::GetInputtingStringLayout() {
	if (m_d2d->inputtingstring_layout) {
		return m_d2d->inputtingstring_layout;
	}
	auto l= BuildInputtingStringLayout();
	m_d2d->inputtingstring_layout = l;
	return l;
}
void ConsoleWindowTextArea::ResetInputtingStringLayout() {
	m_d2d->inputtingstring_layout.Reset();
}
Microsoft::WRL::ComPtr<IDWriteTextLayout1> ConsoleWindowTextArea::BuildInputtingStringLayout() {
	auto&& ftext = InputtingString();
	auto rc = GetAreaDip();
	auto&& t = m_d2d->GetRenderTarget();
	ComPtr<IDWriteTextLayout1> layout = m_tbuilder->CreateTextLayout(ftext, rc.width, rc.height);
	{
		layout->SetCharacterSpacing(0, 0, 0, { 0,static_cast<UINT32>(ftext.length()) });
		layout->SetPairKerning(false, { 0,static_cast<UINT32>(ftext.length()) });
		layout->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, m_linespacing, m_baseline);
		layout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	}
	//inputting string
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
			//attrÇ…ëÆê´Ç™ì¸Ç¡ÇƒÇ¢ÇÈÇÃÇ≈ëÆê´Ç…äÓÇ√Ç¢Çƒï`âÊÇ≥ÇπÇÈ
			ComPtr<DWriteDrawerEffect> effect = new DWriteDrawerEffect(
				convertColor(attr.crBk, t, m_d2d->transparency.Get()).Get(),
				convertColor(attr.crText, t, m_d2d->textColor.Get()).Get(),
				attr.lsStyle == TF_LS_NONE ? std::unique_ptr<DWriteDrawerEffectUnderline>() : std::make_unique<DWriteDrawerEffectUnderline>(convertLineStyle(attr.lsStyle),
					static_cast<bool>(attr.fBoldLine),
					convertColor(attr.crLine, t, m_d2d->textColor.Get()).Get())
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
	return layout;
}
Microsoft::WRL::ComPtr<IDWriteTextLayout1> ConsoleWindowTextArea::GetLayout(ShellContext::attrtext_line& l) {
	if (l.resource()&& std::static_pointer_cast<com::ComHolder<IDWriteTextLayout1>>(l.resource())->ptr) {
		return std::static_pointer_cast<com::ComHolder<IDWriteTextLayout1>>(l.resource())->ptr;
	}
	l.resource() = std::make_shared<com::ComHolder<IDWriteTextLayout1>>(BuildLayout(l));
	return std::static_pointer_cast<com::ComHolder<IDWriteTextLayout1>>(l.resource())->ptr;
}
Microsoft::WRL::ComPtr<IDWriteTextLayout1> ConsoleWindowTextArea::BuildLayout(ShellContext::attrtext_line& l) {
	auto&& t = m_d2d->GetRenderTarget();
	auto rc = GetAreaDip();

	std::wstring ftext;
	auto&& end = l.end();
	for (auto itr = l.begin(); itr != end; ++itr) {
		ftext += itr->textW();
	}
	ComPtr<IDWriteTextLayout1> layout = m_tbuilder->CreateTextLayout(ftext,rc.width, rc.height);
	{
		layout->SetCharacterSpacing(0, 0, 0, { 0,static_cast<UINT32>(ftext.length()) });
		layout->SetPairKerning(false, { 0,static_cast<UINT32>(ftext.length()) });
		layout->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, m_linespacing, m_baseline);
		layout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	}

	int32_t strcnt = 0;
	for (auto itr = l.begin(); itr != l.end(); ++itr) {
		auto nstrcnt = strcnt + itr->length();
		DWRITE_TEXT_RANGE range{ static_cast<UINT32>(strcnt),static_cast<UINT32>(itr->length()) };
		if (itr->bold()) {
			layout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
		}
		if (itr->italic()) {
			layout->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, range);
		}
		if (itr->underline()) {
			layout->SetUnderline(true, range);
		}
		if (itr->crossed_out()) {
			layout->SetStrikethrough(true, range);
		}
		ComPtr<ID2D1SolidColorBrush> bgbrush;
		t->CreateSolidColorBrush(D2D1::ColorF(itr->backgroundColor()), &bgbrush);
		ComPtr<ID2D1SolidColorBrush> frbrush;
		auto fralpha = 1.0f;

		if ((itr->blink() == ansi::Blink::Fast && !m_fast_blink_display) || (itr->blink() == ansi::Blink::Slow && !m_slow_blink_display)) {
			fralpha = 0;
		}
		else if (itr->faint()) {
			fralpha = 0.75f;
		}
		t->CreateSolidColorBrush(D2D1::ColorF(itr->textColor(), fralpha), &frbrush);
		ComPtr<DWriteDrawerEffect> effect = new DWriteDrawerEffect(
			bgbrush.Get(),
			frbrush.Get(),
			itr->underline() ? std::make_unique<DWriteDrawerEffectUnderline>(
				LineStyle_Solid,
				false,
				frbrush.Get()) : std::unique_ptr<DWriteDrawerEffectUnderline>()
		);
		layout->SetDrawingEffect(effect.Get(), range);
		strcnt = nstrcnt;
	}
	return layout;
}

/*Microsoft::WRL::ComPtr<IDWriteTextLayout1> ConsoleWindowTextArea::BuildLayout()  {
	auto rc = GetAreaDip();
	auto&& t = m_d2d->GetRenderTarget();
	std::wstring ftext;
	

	for (auto&& l : (m_console->shell->GetAll())) {
		for (auto itr = l.begin(); itr != l.end(); ++itr) {
			ftext += itr->textW();
		}
	}

	m_lengthShell = static_cast<UINT32>(ftext.length());
	ftext += InputtingString();

	ComPtr<IDWriteTextLayout1> layout = m_tbuilder->CreateTextLayout(ftext, rc.width, rc.height);
	{
		layout->SetCharacterSpacing(0, 0, 0, { 0,static_cast<UINT32>(ftext.length()) });
		layout->SetPairKerning(false, { 0,static_cast<UINT32>(ftext.length()) });
		layout->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, m_linespacing, m_baseline);
		layout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	}
	

	{
		//LockHolder lock(*(m_console->shell));

		//draw string
		//shell string
		int32_t strcnt = 0;
		for (auto&& l : (m_console->shell->GetAll())) {
			for (auto itr = l.begin(); itr != l.end(); ++itr) {
				auto nstrcnt = strcnt + itr->length();
				DWRITE_TEXT_RANGE range{ static_cast<UINT32>(strcnt),static_cast<UINT32>(itr->length()) };
				if (itr->bold()) {
					layout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
				}
				if (itr->italic()) {
					layout->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, range);
				}
				if (itr->underline()) {
					layout->SetUnderline(true, range);
				}
				if (itr->crossed_out()) {
					layout->SetStrikethrough(true, range);
				}
				ComPtr<ID2D1SolidColorBrush> bgbrush;
				t->CreateSolidColorBrush(D2D1::ColorF(itr->backgroundColor()), &bgbrush);
				ComPtr<ID2D1SolidColorBrush> frbrush;
				auto fralpha = 1.0f;

				if ((itr->blink() == ansi::Blink::Fast && !m_fast_blink_display) || (itr->blink() == ansi::Blink::Slow && !m_slow_blink_display)) {
					fralpha = 0;
				}
				else if (itr->faint()) {
					fralpha = 0.75f;
				}
				t->CreateSolidColorBrush(D2D1::ColorF(itr->textColor(), fralpha), &frbrush);
				ComPtr<DWriteDrawerEffect> effect = new DWriteDrawerEffect(
					bgbrush.Get(),
					frbrush.Get(),
					itr->underline() ? std::make_unique<DWriteDrawerEffectUnderline>(
						LineStyle_Solid,
						false,
						frbrush.Get()) : std::unique_ptr<DWriteDrawerEffectUnderline>()
				);
				layout->SetDrawingEffect(effect.Get(), range);
				strcnt = nstrcnt;
			}
		}

	}


}*/
void ConsoleWindowTextArea::DrawShellText() {
	auto height =GetAreaDip().height;
	auto begin = m_console->shell->GetAll().begin();
	auto nowY = height;
	auto end = m_console->shell->GetView().end();
	if (end == begin) {
		return;
	}
	auto itr = end;
	--itr;
	for (; itr != begin; --itr) {
		ComPtr<IDWriteTextLayout1> layout = GetLayout(*itr);
		DWRITE_TEXT_METRICS met;
		layout->GetMetrics(&met);
		nowY -= met.height;
		if (nowY < 0.0f) {
			break;
		}
	}

	ComPtr<DWriteDrawerEffect> defaultEffect = new DWriteDrawerEffect{
		m_d2d->transparency.Get(),
		m_d2d->textColor.Get(),
		std::unique_ptr<DWriteDrawerEffectUnderline>()
	};
	auto t = m_d2d->GetRenderTarget();
	auto context = std::make_unique<DWriteDrawerContext>(t, defaultEffect.Get());
	if (nowY > 0.0f) {
		nowY = 0;
	}
	DWRITE_TEXT_METRICS met;
	{
		for (; itr != std::prev(end); ++itr) {
			ComPtr<IDWriteTextLayout1> layout = GetLayout(*itr);
			met = {};
			layout->GetMetrics(&met);
			layout->Draw(context.get(), m_drawer.Get(), m_originX, nowY);
			nowY += met.height;
		}

		ComPtr<IDWriteTextLayout1> layout = GetLayout(*itr);
		met = {};
		layout->GetMetrics(&met);
		layout->Draw(context.get(), m_drawer.Get(), m_originX, nowY);
	}
	auto&& inputtingstring_layout = GetInputtingStringLayout();
	
	//draw caret
	//https://stackoverflow.com/questions/28057369/direct2d-createtextlayout-how-to-get-caret-coordinates
	if (m_caret_display) {
		DWRITE_HIT_TEST_METRICS hitTestMetrics;
		bool isTrailingHit = false;
		float caretX, caretY;
		FailToThrowHR(inputtingstring_layout->HitTestTextPosition(
			SelectionStart(),
			isTrailingHit,
			&caretX,
			&caretY,
			&hitTestMetrics
		));
		DWORD caretWidth = 1;
		FailToThrowB(SystemParametersInfo(SPI_GETCARETWIDTH, 0, &caretWidth, 0));
		DWORD halfCaretWidth = caretWidth / 2u;

		t->FillRectangle({
			caretX - halfCaretWidth + m_originX+ met.width,
			hitTestMetrics.top + nowY,
			caretX + (caretWidth - halfCaretWidth) + m_originX+ met.width,
			hitTestMetrics.top + hitTestMetrics.height + nowY
			}, m_d2d->red.Get());
	}
	else if (SelectionStart() != SelectionEnd())
	{
		UINT32 count;
		inputtingstring_layout->HitTestTextRange(SelectionStart(), SelectionEnd() - SelectionStart(), m_originX+met.width, nowY, NULL, 0, &count);

		std::unique_ptr<DWRITE_HIT_TEST_METRICS[]> mats(new DWRITE_HIT_TEST_METRICS[count]);
		FailToThrowHR(inputtingstring_layout->HitTestTextRange(SelectionStart(), SelectionEnd() - SelectionStart(), m_originX + met.width, nowY, mats.get(), count, &count));

		for (auto i = 0UL; i < count; ++i)
		{
			t->FillRectangle({
				mats[i].left ,
				mats[i].top ,
				mats[i].left + mats[i].width ,
				mats[i].top + mats[i].height
				}, m_d2d->red.Get());
		}
	}
	inputtingstring_layout->Draw(context.get(), m_drawer.Get(), m_originX + met.width, nowY);

}
void ConsoleWindowTextArea::OnPaint() {

	CallWithAppLock(false, [this]() {
		LockHolder lock(*(m_console->shell));
		PAINTSTRUCT pstruct;
		auto t = m_d2d->GetRenderTarget();

		t->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

		BeginPaint(m_textarea_hwnd, &pstruct);

		t->BeginDraw();
		t->Clear(m_d2d->clearColor);

		DrawShellText();



		/**/

		FailToThrowHR(t->EndDraw());

		EndPaint(m_textarea_hwnd, &pstruct);
	});
}
void ConsoleWindowTextArea::SetConsoleContext(std::shared_ptr<tignear::sakura::cwnd::Context> console) {
	if (m_console) {
		m_console->shell->RemoveLayoutChangeListener(m_layout_change_listener_removekey);
		m_console->shell->RemoveTextChangeListener(m_text_change_listener_removekey);
	}

	m_console =console;
	m_layout_change_listener_removekey = m_console->shell->AddLayoutChangeListener([this](ShellContext*,auto x,auto y) {
		InvalidateRect(this->GetHWnd(), NULL, FALSE);
	});
	m_text_change_listener_removekey = m_console->shell->AddTextChangeListener([this](ShellContext* c,auto&& text) {
		UpdateText(c,text);
	});
	
	m_tbuilder->UpdateFontName(m_console->shell->DefaultFont().c_str());
	m_tbuilder->UpdateFontSize(static_cast<FLOAT>(m_console->shell->FontSize()));
	auto format=m_tbuilder->GetTextFormat();
	ComPtr<IDWriteFontCollection> collection;
	format->GetFontCollection(&collection);
	UINT32 font_index;
	BOOL font_exit = FALSE;
	collection->FindFamilyName(m_console->shell->DefaultFont().c_str(), &font_index, &font_exit);
	FailToThrowB(font_exit);
	ComPtr<IDWriteFontFamily> font_family;
	collection->GetFontFamily(font_index, &font_family);
	ComPtr<IDWriteFont> font;
	font_family->GetFirstMatchingFont(format->GetFontWeight(), format->GetFontStretch(), format->GetFontStyle(), &font);
	DWRITE_FONT_METRICS metrics;
	font->GetMetrics(&metrics);
	FLOAT ratio = format->GetFontSize() / (float)metrics.designUnitsPerEm;
	m_linespacing = (metrics.ascent + metrics.descent + metrics.lineGap) * ratio;
	m_baseline = metrics.ascent*ratio;
	m_console->shell->SetPageSize(GetPageSize());
	SetFocus(m_textarea_hwnd);
}
void ConsoleWindowTextArea::Create(HINSTANCE hinst,HWND pwnd, const tignear::win::dpi::Dpi& dpi, DIP x, DIP y, DIP w, DIP h, HMENU hmenu, ITfThreadMgr* threadmgr, TfClientId cid, ITfCategoryMgr* cate_mgr, ITfDisplayAttributeMgr* attr_mgr, ID2D1Factory* d2d_f, IDWriteFactory* dwrite_f, std::shared_ptr<tignear::sakura::cwnd::Context> console,ConsoleWindowTextArea** pr) {
	auto r = new ConsoleWindowTextArea(dpi);
	r->AddRef();
	r->m_hinst = hinst;
	r->m_parentHwnd = pwnd;
	r->m_threadmgr = threadmgr;
	r->m_clientId = cid;
	r->m_category_mgr = cate_mgr;
	r->m_attribute_mgr = attr_mgr;
	r->m_caret_update_time = std::chrono::steady_clock::now();


	r->Init(x, y, w, h, hmenu, d2d_f, dwrite_f,console);
	*pr = r;
}
//static fields
bool ConsoleWindowTextArea::m_registerstate = false;
