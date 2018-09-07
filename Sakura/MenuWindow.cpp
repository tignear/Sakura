#include "stdafx.h"
#include <CommCtrl.h>
#include "MenuWindow.h"
#include "FailToThrow.h"
#include "ConsoleWindowContext.h"
#include "strconv.h"
using tignear::sakura::MenuWindow;
using tignear::sakura::Config;
using tignear::sakura::ShellContextFactory;
using tignear::sakura::cwnd::Context;
bool MenuWindow::RegisterMenuWindowClass(HINSTANCE hinst) {
	if (m_registerstate) return true;
	WNDCLASSEX wcex{};

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
std::unique_ptr<MenuWindow> MenuWindow::Create(HINSTANCE hinst,HWND parent,int x,int y,unsigned int w,HMENU hmenu,Config& conf, std::function<ShellContextFactory*(std::string)> getFactory, std::function<std::shared_ptr<void>(std::string)> getResource) {
	auto r = std::make_unique<MenuWindow>(conf,getFactory,getResource);
	if (!RegisterMenuWindowClass(hinst)) {
		r.reset();
		return r;
	}
	r->m_parent_hwnd = parent;
	r->m_hwnd = CreateWindowEx(0,m_classname,NULL,WS_VISIBLE|WS_CHILD,x,y,w,m_menu_height,parent,hmenu,hinst,r.get());
	r->m_tab_hwnd = CreateWindowEx(0, WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE| TCS_FIXEDWIDTH, 0, 0, w-m_menu_button_width, m_menu_height, r->m_hwnd, m_hmenu_tab, hinst, NULL);
	r->m_menu_button_hwnd = CreateWindowEx(0, WC_BUTTON, _T(""), WS_CHILD | WS_VISIBLE, w - m_menu_button_width, 0, m_menu_button_width, m_menu_height, r->m_hwnd, m_hmenu_menu, hinst, NULL);
	SendMessage(r->m_menu_button_hwnd, WM_SETFONT, (WPARAM)r->m_icon_font, TRUE);
	return r;
}
LRESULT CALLBACK MenuWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static MenuWindow* self=nullptr;
	switch (message)
	{
	case WM_CREATE:
		self=reinterpret_cast<MenuWindow*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
		break;
	case WM_SIZE:
	case WM_SIZING:
	{
		RECT rect{};
		GetClientRect(self->m_hwnd, &rect);
		SetWindowPos(self->m_tab_hwnd, 0,rect.left,rect.top,rect.right-rect.left-m_menu_button_width,rect.bottom-rect.top,SWP_NOZORDER);
		SetWindowPos(self->m_menu_button_hwnd, 0, rect.right-rect.left - m_menu_button_width, 0, m_menu_button_width, m_menu_height, SWP_NOZORDER);

		break;
	}
	default:
		return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}
HWND MenuWindow::GetHWnd() {
	return m_hwnd;
}
std::shared_ptr<Context> MenuWindow::GetCurrentContext(unsigned int w,unsigned int h) {
	if (m_new) {
		m_new = false;
		auto sinfo = m_config.shells.at(m_current_context_pos);
		auto c = std::make_shared<Context>(m_getFactory(std::get<1>(sinfo))->Create(ShellContextFactory::Information{ w,h,std::get<2>(sinfo),m_getResource }));
		m_contexts.push_back(c);
		m_current_context_pos =static_cast<int>(m_contexts.size() - 1);
		TCITEM item{};
		item.mask = TCIF_TEXT;
		item.dwState = TCIS_BUTTONPRESSED;
		auto title = c->shell->GetTitle();
#pragma warning(disable:4189)
#ifdef UNICODE
		auto r = title.empty() ? ansi_to_wide(std::get<0>(sinfo)) : std::wstring(title);
#else
		auto r = title.empty() ? std::get<0>(sinfo) : wide_to_ansi(std::wstring(title));
#endif 
		item.pszText = r.data();
		TabCtrl_InsertItem(m_tab_hwnd, m_current_context_pos,&item);
	}
	return m_contexts.at(m_current_context_pos);
	
}
//static 
bool MenuWindow::m_registerstate = false;