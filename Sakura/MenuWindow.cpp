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
INT MenuWindow::GetTabIndexFromId(LPARAM id) {
	auto cnt = TabCtrl_GetItemCount(m_tab_hwnd);
	for (auto i = 0; i < cnt; ++i) {
		TC_ITEM item{};
		item.mask = TCIF_PARAM;
		TabCtrl_GetItem(m_tab_hwnd, i, &item);
		if (id == item.lParam) {
			return i;
		}
	}
	return -1;
}
void MenuWindow::RemoveTab(LPARAM id) {
	auto i = GetTabIndexFromId(id);
	if (i == TabCtrl_GetCurSel(m_tab_hwnd)) {
		auto cnew = std::max(0, i - 1);
		TabCtrl_SetCurSel(m_tab_hwnd,cnew);
		TC_ITEM item{};
		item.mask = TCIF_PARAM;
		TabCtrl_GetItem(m_tab_hwnd, cnew, &item);
		m_current_context_ptr = item.lParam;
	}
	TabCtrl_DeleteItem(m_tab_hwnd, i);
}
std::unique_ptr<MenuWindow> MenuWindow::Create(
	HINSTANCE hinst,
	HWND parent,
	const tignear::win::dpi::Dpi& dpi,
	DIP x,DIP y,DIP w,
	HMENU hmenu,
	const std::unordered_map<uintptr_t, std::shared_ptr<Context>>& contexts,
	std::function<void(std::shared_ptr<Context>)> newContxet,
	std::function<void()> contextUpdate,
	Config& conf,
	std::function<ShellContextFactory*(std::string_view)> getFactory,
	std::function<std::shared_ptr<void>(std::string_view)> getResource
) {
	auto ttf = win::GetModuleFilePath(NULL);
	ttf += _T("\\fonts\\menu.ttf");
	auto cnt = AddFontResourceExW(ttf.c_str(), FR_PRIVATE, NULL);
	if (cnt == 0) {
		return std::unique_ptr < MenuWindow > ();
	}
	auto r = std::make_unique<MenuWindow>(dpi,contexts, newContxet,contextUpdate,conf,getFactory,getResource);
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
		InsertMenuItem(r->m_hmenu_menu, i, TRUE, &mii);

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
		self->m_new_shell =self-> m_config.shells.at(s);
		self->m_contextUpdate();
		break;
	}
	case WM_NOTIFY:
	{
		auto* p = reinterpret_cast<LPNMHDR>(lParam);
		if (p->hwndFrom==self->m_tab_hwnd) {
			TCITEM select;
			select.mask = TCIF_PARAM;
			auto cur = TabCtrl_GetCurSel(self->m_tab_hwnd);
			if (!TabCtrl_GetItem(self->m_tab_hwnd, cur, &select)) {
				break;
			}
			self->m_current_context_ptr = static_cast<WPARAM>(select.lParam);
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
	if (m_new_shell) {
		auto new_shell = m_new_shell.value();
		auto c = std::make_shared<Context>(m_getFactory(std::get<1>(new_shell))->Create(ShellContextFactory::Information{w,h,std::get<2>(new_shell),m_getResource }));
		m_new_shell = std::nullopt;
		if (!c->shell) {
			throw std::runtime_error("create context failed");
		}
		m_current_context_ptr =reinterpret_cast<LPARAM>(c->shell.get());
		m_newContext(c);
		c->shell->AddExitListener([this](auto* e) {
			auto k = reinterpret_cast<uintptr_t>(e);
			RemoveTab(k);
			m_contextUpdate();
		});
		TCITEM item{};
		item.mask = TCIF_TEXT|TCIF_PARAM;
		item.dwState = TCIS_BUTTONPRESSED;
		item.lParam = m_current_context_ptr;
		auto title = c->shell->GetTitle();
#ifdef UNICODE
		auto r = title.empty() ? ansi_to_wide(std::get<0>(new_shell)) : std::wstring(title);
#else
		auto r = title.empty() ? std::get<0>(sinfo) : wide_to_ansi(std::wstring(title));
#endif 
		item.pszText = r.data();
		auto index = TabCtrl_GetItemCount(m_tab_hwnd);
		TabCtrl_InsertItem(m_tab_hwnd, index, &item);
		TabCtrl_SetCurSel(m_tab_hwnd, index);
	}
	auto r=m_contexts.find(m_current_context_ptr);
	if (r == m_contexts.end()) {
		return std::shared_ptr<Context>();
	}
	return r->second;
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
