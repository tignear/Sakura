#include "stdafx.h"
#include <commctrl.h>
#include "ConsoleWindow.h"
#include "FailToThrow.h"
using tignear::sakura::ShellContextFactory;
using tignear::sakura::ConsoleWindow;
using tignear::sakura::cwnd::Context;
using tignear::sakura::Config;
using tignear::sakura::cwnd::Context;
std::unique_ptr<ConsoleWindow> ConsoleWindow::Create(
	HINSTANCE hinst,
	HWND pwnd,
	int x, int y,unsigned int w, unsigned int h,
	HMENU hmenu, 
	ITfThreadMgr* threadmgr, 
	TfClientId cid, 
	ITfCategoryMgr* cate_mgr, 
	ITfDisplayAttributeMgr* attr_mgr, 
	ID2D1Factory* d2d_f, 
	IDWriteFactory* dwrite_f, 
	std::function<std::shared_ptr<Context>(unsigned int w, unsigned int h)> getContext
){
	auto r = std::make_unique<ConsoleWindow>();
	auto tareaW = w - m_scrollbar_width;
	auto tareaH = h-m_scrollbar_width;
	r->m_getContext = getContext;
	r->m_console=getContext(tareaW,tareaH);
	r->m_hinst = hinst;
	FailToThrowB(RegisterConsoleWindowClass(hinst));
	r->m_parent_hwnd = pwnd;
	r->m_hwnd=CreateWindowEx(0, m_classname, NULL, WS_OVERLAPPED | WS_CHILD | WS_VISIBLE, x, y, w, h, pwnd,hmenu, hinst, r.get());
	r->m_scrollbar_column_hwnd =CreateWindowEx(0, WC_SCROLLBAR, NULL,WS_CHILD|WS_VISIBLE| SBS_VERT, tareaW,0, m_scrollbar_width, tareaH,r->m_hwnd,m_hmenu_column_scrollbar,hinst,NULL);
	r->m_scrollbar_row_hwnd = CreateWindowEx(0, WC_SCROLLBAR, NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ,0, tareaH, tareaW, m_scrollbar_width, r->m_hwnd, m_hmenu_column_scrollbar, hinst, NULL);
	SCROLLINFO sbinfo{};
	sbinfo.cbSize = sizeof(sbinfo);
	sbinfo.fMask = SIF_DISABLENOSCROLL | SIF_ALL;
	sbinfo.nMin = 0;
	sbinfo.nMax = 20;
	sbinfo.nPage = 10;
	SetScrollInfo(r->m_scrollbar_column_hwnd, SB_VERT, &sbinfo, TRUE);
	ConsoleWindowTextArea::Create(hinst,r->m_hwnd, 0, 0, tareaW, tareaH, m_hmenu_textarea, threadmgr, cid, cate_mgr, attr_mgr, d2d_f, dwrite_f,r->m_console, &(r->m_textarea));
	r->UpdateScrollBar();
	r->m_console->shell->AddLayoutChangeListener(std::bind(&ConsoleWindow::OnLayoutChange,std::ref(*r), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	return r;
 }
bool ConsoleWindow::RegisterConsoleWindowClass(HINSTANCE hinst) {
	if (m_registerstate) return true;
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
	wcex.lpszClassName = m_classname;
	wcex.hIconSm = NULL;
	if (FAILED(RegisterClassEx(&wcex)))return false;
	m_registerstate = true;
	return true;
}
LRESULT CALLBACK ConsoleWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static ConsoleWindow* self;
	switch (message)
	{
	case WM_CREATE:
		self=static_cast<ConsoleWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		break;
	case WM_SETFOCUS:
		if (self->m_textarea) {
			if (SetFocus(self->m_textarea->GetHWnd()) == NULL) {
				break;
			}
		}
		break;
	case WM_SIZING:
	case WM_SIZE:
	{			
		RECT rect;
		GetClientRect(self->GetHWnd(), &rect);
		if (self->m_textarea) {
			SetWindowPos(self->m_textarea->GetHWnd(), 0, rect.left, rect.top, rect.right - rect.left - m_scrollbar_width, rect.bottom - rect.top-m_scrollbar_width, SWP_NOOWNERZORDER);
		}
		if (self->m_scrollbar_column_hwnd) {
			FailToThrowB(SetWindowPos(self->m_scrollbar_column_hwnd,0, rect.right-m_scrollbar_width, rect.top, m_scrollbar_width, rect.bottom - rect.top-m_scrollbar_width, SWP_NOOWNERZORDER));
		}
		if (self->m_scrollbar_row_hwnd) {
			FailToThrowB(SetWindowPos(self->m_scrollbar_row_hwnd, 0, rect.left, rect.bottom - rect.top - m_scrollbar_width, rect.right - rect.left - m_scrollbar_width, m_scrollbar_width, SWP_NOOWNERZORDER));
		}
		break;
	}
	case WM_VSCROLL:
		switch (LOWORD(wParam)) {
		case SB_THUMBTRACK:
		{
			SCROLLINFO info{};
			info.cbSize = sizeof(info);
			info.fMask = SIF_TRACKPOS;
			GetScrollInfo(self->m_scrollbar_column_hwnd, SB_CTL, &info);
			self->m_textarea->SetViewPosition(info.nTrackPos);
			break;
		}
		default:
		{
			SCROLLINFO info{};
			info.cbSize = sizeof(info);
			info.fMask = SIF_POS;
			GetScrollInfo(self->m_scrollbar_column_hwnd, SB_CTL, &info);
			self->m_textarea->SetViewPosition(info.nPos);
			break;
		}

		}
		break;
	case WM_HSCROLL:
		switch (LOWORD(wParam)) {
		case SB_THUMBTRACK:
		{
			SCROLLINFO info{};
			info.cbSize = sizeof(info);
			info.fMask = SIF_TRACKPOS;
			GetScrollInfo(self->m_scrollbar_row_hwnd, SB_CTL, &info);
			self->m_textarea->SetOriginX(static_cast<FLOAT>(-info.nTrackPos));
			break;
		}
		default:
		{
			SCROLLINFO info{};
			info.cbSize = sizeof(info);
			info.fMask = SIF_POS;
			GetScrollInfo(self->m_scrollbar_row_hwnd, SB_CTL, &info);
			self->m_textarea->SetOriginX(static_cast<FLOAT>(-info.nPos));
			break;
		}
		}
		self->UpdateScrollBar();
		break;
	case WM_UPDATE_SCROLLBAR:
		self->UpdateScrollBar();
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}
HWND ConsoleWindow::GetHWnd() {
	return m_hwnd;
}
void ConsoleWindow::OnLayoutChange(tignear::sakura::ShellContext* shell,bool ,bool) {
	PostMessage(m_hwnd, WM_UPDATE_SCROLLBAR, 0, 0);
}
void ConsoleWindow::UpdateScrollBar() {
	SCROLLINFO sbinfo_c{};
	sbinfo_c.cbSize = sizeof(sbinfo_c);
	sbinfo_c.fMask = SIF_DISABLENOSCROLL | SIF_ALL;
	sbinfo_c.nMin = 0;
	sbinfo_c.nPos = static_cast<UINT>( m_console->shell->GetViewStart());
	sbinfo_c.nMax = static_cast<UINT>(m_console->shell->GetLineCount());
	sbinfo_c.nPage = static_cast<UINT>( m_textarea->GetPageSize());
	SetScrollInfo(m_scrollbar_column_hwnd, SB_CTL, &sbinfo_c, TRUE);
	SCROLLINFO sbinfo_r{};
	sbinfo_r.cbSize = sizeof(sbinfo_r);
	sbinfo_r.fMask = SIF_DISABLENOSCROLL | SIF_ALL;
	sbinfo_r.nMin = 0;
	sbinfo_r.nPos = static_cast<UINT>(-(m_textarea->GetOriginX()+0.5f));
	sbinfo_r.nMax = static_cast<UINT>(m_textarea->GetTextWidthDip()+0.5f);
	sbinfo_r.nPage = static_cast<UINT>(m_textarea->GetAreaDip().height+0.5f);
	SetScrollInfo(m_scrollbar_row_hwnd, SB_CTL, &sbinfo_r, TRUE);
}
void ConsoleWindow::SetConsoleContext(std::shared_ptr<tignear::sakura::cwnd::Context> c) {
	m_console = c;
	m_textarea->SetConsoleContext(c);
}
void ConsoleWindow::ReGetConsoleContext() {
	RECT rect;
	GetClientRect(m_textarea->GetHWnd(), &rect);
	SetConsoleContext(m_getContext(rect.right-rect.left,rect.bottom-rect.top));
}
//static fields
bool ConsoleWindow::m_registerstate = false;