#include "stdafx.h"
#include <commctrl.h>
#include "ConsoleWindow.h"
#include "FailToThrow.h"
using tignear::sakura::ShellContextFactory;
using tignear::sakura::ConsoleWindow;
using tignear::sakura::cwnd::Context;
using tignear::sakura::Config;
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
	Config&& config,
	std::function<ShellContextFactory*(std::string)> getFactoryFn,
	std::function<std::shared_ptr<void>(std::string)> getResourceFn
){
	auto r = std::make_unique<ConsoleWindow>();
	auto sinfo = config.shells[config.initshell];
	auto fname=std::get<1>(sinfo);
	auto f= getFactoryFn(fname);
	if (!f) {
		throw std::runtime_error("factory is not found!");
	}
	auto tareaW = w - m_scrollbar_width;
	auto tareaH = h - m_tab_width;
	r->m_console = std::make_shared<Context>(
		f->Create(
			ShellContextFactory::Information{
				tareaW ,
				tareaH,
				std::get<2>(sinfo),
				getResourceFn
			}
		)
	);
	r->m_hinst = hinst;
	FailToThrowB(RegisterConsoleWindowClass(hinst));
	r->m_parent_hwnd = pwnd;
	r->m_hwnd=CreateWindowEx(0, m_classname, NULL, WS_OVERLAPPED | WS_CHILD | WS_VISIBLE, x, y, w, h, pwnd,hmenu, hinst, r.get());
	r->m_scrollbar_column_hwnd =CreateWindowEx(0, WC_SCROLLBAR, NULL,WS_CHILD|WS_VISIBLE| SBS_VERT, tareaW,m_tab_width, m_scrollbar_width, tareaH,r->m_hwnd,m_hmenu_column_scrollbar,hinst,NULL);
	r->m_scrollbar_row_hwnd = CreateWindowEx(0, WC_SCROLLBAR, NULL, WS_CHILD | WS_VISIBLE | SBS_VERT,0, tareaH, tareaW, m_scrollbar_width, r->m_hwnd, m_hmenu_column_scrollbar, hinst, NULL);
	r->m_tab_hwnd = CreateWindowEx(0, WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 0, 0, w, m_tab_width,r->m_hwnd,m_hmenu_tab,hinst,NULL);
	SCROLLINFO sbinfo{};
	sbinfo.cbSize = sizeof(sbinfo);
	sbinfo.fMask = SIF_DISABLENOSCROLL | SIF_ALL;
	sbinfo.nMin = 0;
	sbinfo.nMax = 20;
	sbinfo.nPage = 10;
	SetScrollInfo(r->m_scrollbar_column_hwnd, SB_VERT, &sbinfo, TRUE);
	ConsoleWindowTextArea::Create(hinst,r->m_hwnd, 0, m_tab_width, tareaW, tareaH, m_hmenu_textarea, threadmgr, cid, cate_mgr, attr_mgr, d2d_f, dwrite_f,r->m_console, &(r->m_textarea));
	r->UpdateScrollBar();
	r->m_console->shell->AddLayoutChangeListener(std::bind(&ConsoleWindow::OnLayoutChange,std::ref(*r), std::placeholders::_1));
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
			SetWindowPos(self->m_textarea->GetHWnd(), 0, rect.left, rect.top, rect.right - rect.left - m_scrollbar_width, rect.bottom - rect.top, SWP_NOOWNERZORDER);

		}
		if (self->m_scrollbar_column_hwnd) {
			FailToThrowB(SetWindowPos(self->m_scrollbar_column_hwnd,0, rect.right-m_scrollbar_width, rect.top, m_scrollbar_width, rect.bottom - rect.top, SWP_NOOWNERZORDER));

		}
		break;
	}
	case WM_VSCROLL:
	switch(LOWORD(wParam)){
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
void ConsoleWindow::OnLayoutChange(tignear::sakura::ShellContext* shell) {
	PostMessage(m_hwnd, WM_UPDATE_SCROLLBAR, 0, 0);
}
void ConsoleWindow::UpdateScrollBar() {
	SCROLLINFO sbinfo{};
	sbinfo.cbSize = sizeof(sbinfo);
	sbinfo.fMask = SIF_DISABLENOSCROLL | SIF_ALL;
	sbinfo.nMin = 0;
	sbinfo.nPos = static_cast<UINT>( m_console->shell->GetViewStart());
	sbinfo.nMax = static_cast<UINT>(m_console->shell->GetLineCount());
	sbinfo.nPage = static_cast<UINT>( m_textarea->GetPageSize());
	SetScrollInfo(m_scrollbar_column_hwnd, SB_CTL, &sbinfo, FALSE);
	
	InvalidateRect(m_scrollbar_column_hwnd,NULL,TRUE);
}
//static fields
bool ConsoleWindow::m_registerstate = false;