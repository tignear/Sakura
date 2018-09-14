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
std::unique_ptr<MenuWindow> MenuWindow::Create(
	HINSTANCE hinst,
	HWND parent,
	const tignear::win::dpi::Dpi& dpi,
	DIP x,DIP y,DIP w,
	HMENU hmenu,
	std::function<void()> contextUpdate,
	Config& conf,
	std::function<ShellContextFactory*(std::string)> getFactory,
	std::function<std::shared_ptr<void>(std::string)> getResource
) {
	auto ttf = stdex::tstring(win::GetExecutableFilePath());
	ttf += _T("\\fonts\\menu.ttf");
	auto cnt = AddFontResourceEx(ttf.c_str(), FR_PRIVATE, NULL);
	if (cnt == 0) {
		return std::unique_ptr < MenuWindow > ();
	}
	auto r = std::make_unique<MenuWindow>(dpi,contextUpdate,conf,getFactory,getResource);
	if (!RegisterMenuWindowClass(hinst)) {
		r.reset();
		return r;
	}
	r->m_parent_hwnd = parent;
	r->m_hwnd = CreateWindowEx(0,m_classname,NULL,WS_VISIBLE|WS_CHILD,dpi.Pixel(x),dpi.Pixel(y),dpi.Pixel(w),dpi.Pixel(m_menu_height),parent,hmenu,hinst,r.get());
	r->m_tab_hwnd = CreateWindowEx(0, WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE| TCS_FIXEDWIDTH, 0, 0,dpi.Pixel(w-m_menu_button_width), dpi.Pixel(m_menu_height), r->m_hwnd, m_hmenu_tab, hinst, NULL);
	r->m_menu_button_hwnd = CreateWindowEx(0, WC_BUTTON, _T(""), WS_CHILD | WS_VISIBLE,dpi.Pixel(w - m_menu_button_width), 0,dpi.Pixel(m_menu_button_width), dpi.Pixel(m_menu_height), r->m_hwnd, m_hmenu_menu_button, hinst, NULL);
	r->CreateAndSetFont();
	r->m_hmenu_menu = CreatePopupMenu();
	for (auto i = 0U; i < conf.shells.size();++i) {
		MENUITEMINFO mii = {};
		mii.cbSize = sizeof(decltype(mii));
		mii.fMask = MIIM_TYPE| MIIM_ID;
		mii.fType = MFT_STRING;
		mii.wID = i;
#ifdef UNICODE
		auto b = utf8_to_wide(std::get<0>(conf.shells[i]));
		mii.dwTypeData = b.data();
#else
		mii.dwTypeData = std::get<0>(conf.shells[i]).data();
#endif
		InsertMenuItem(r->m_hmenu_menu, 0, TRUE, &mii);

	}

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
		auto&& dpi = self->m_dpi;
		auto button_w_px = dpi.Pixel(m_menu_button_width);
		SetWindowPos(self->m_tab_hwnd, 0,rect.left,rect.top,rect.right-rect.left- button_w_px,rect.bottom-rect.top,SWP_NOZORDER);
		SetWindowPos(self->m_menu_button_hwnd, 0, rect.right-rect.left - button_w_px, 0, button_w_px,dpi.Pixel(m_menu_height), SWP_NOZORDER);
		break;
	}
	case WM_COMMAND:
	if(lParam==reinterpret_cast<LPARAM>(self->m_menu_button_hwnd)){
		RECT rect{};
		GetClientRect(self->m_hwnd, &rect);
		POINT point{rect.right,rect.bottom};
		ClientToScreen(self->m_hwnd,&point);
		TrackPopupMenu(self->m_hmenu_menu, TPM_RIGHTALIGN | TPM_TOPALIGN, point.x, point.y, NULL, self->m_hwnd, NULL);
		break;
	}else{
		auto s = LOWORD(wParam);
		self->m_new=true;
		self->m_current_context_pos = s;
		self->m_contextUpdate();
		break;
	}
	case WM_NOTIFY:
	{
		auto* p = reinterpret_cast<LPNMHDR>(lParam);
		if (p->hwndFrom==self->m_tab_hwnd) {
			self->m_current_context_pos = TabCtrl_GetCurSel(self->m_tab_hwnd);
			self->m_contextUpdate();
			break;
		}
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	default:
		return DefWindowProc(hwnd,message,wParam,lParam);
	}

	return 0;
}
HWND MenuWindow::GetHWnd() {
	return m_hwnd;
}
std::shared_ptr<Context> MenuWindow::GetCurrentContext(DIP w,DIP h) {
	if (m_new) {
		m_new = false;
		auto sinfo = m_config.shells.at(m_current_context_pos);
		auto c = std::make_shared<Context>(m_getFactory(std::get<1>(sinfo))->Create(ShellContextFactory::Information{ w,h,std::get<2>(sinfo),m_getResource }));
		m_contexts.push_back(c);
		m_current_context_pos =static_cast<int>(m_contexts.size()-1);
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
		TabCtrl_InsertItem(m_tab_hwnd, m_current_context_pos, &item);
	}
	TabCtrl_SetCurSel(m_tab_hwnd, m_current_context_pos);
	return m_contexts.at(m_current_context_pos);
}
void MenuWindow::OnDpiChange() {
	CreateAndSetFont();
}
void MenuWindow::CreateAndSetFont(){
	if (m_icon_font != NULL) {
		DeleteObject(m_icon_font);
	}
	LOGFONT lf{};
	TCHAR fname[] = _T("icomoon");
	lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
	lf.lfQuality = CLEARTYPE_QUALITY;
	lf.lfHeight = m_dpi.Pixel(m_menu_height);
	memcpy(lf.lfFaceName, fname, sizeof(fname));
	m_icon_font = CreateFontIndirect(&lf);
	SendMessage(m_menu_button_hwnd, WM_SETFONT, (WPARAM)m_icon_font, TRUE);
	SendMessage(m_tab_hwnd, WM_SETFONT, 0, TRUE);
}
//static 
bool MenuWindow::m_registerstate = false;