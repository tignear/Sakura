#pragma once
#include <ModuleFilePath.h>
#include <deque>
#include <memory>
#include <optional>
#include "SakuraConfig.h"
#include "ShellContextFactory.h"
#include "ConsoleWindowContext.h"
#include "tstring.h"
#include "Dpi.h"

namespace tignear::sakura {
class MenuWindow {
	static const constexpr HMENU m_hmenu_tab = (HMENU)0x01;
	static const constexpr HMENU m_hmenu_menu_button = (HMENU)0x02;
	static const constexpr LPCTSTR m_classname = _T("tignear.sakura.MenuWindow");
	static const constexpr DIP  m_menu_button_width=20;
	static bool m_registerstate;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static bool RegisterMenuWindowClass(HINSTANCE hinst);
	const std::unordered_map<uintptr_t,std::shared_ptr<cwnd::Context>>& m_contexts;
	const std::function<void(std::shared_ptr<cwnd::Context>)> m_newContext;
	const std::function<void()> m_contextUpdate;
	const std::function<ShellContextFactory*(std::string)> m_getFactory;
	const std::function<std::shared_ptr<void>(std::string)> m_getResource;
	const Config& m_config;
	const win::dpi::Dpi& m_dpi;
	HFONT m_icon_font;
	HWND m_parent_hwnd;
	HWND m_hwnd;
	HWND m_tab_hwnd;
	HWND m_menu_button_hwnd;
	HMENU m_hmenu_menu;
	std::optional<std::tuple<std::string, std::string, LuaIntf::LuaRef>> m_new_shell;
	uintptr_t m_current_context_ptr;
	void CreateAndSetFont();
	INT GetTabIndexFromId(LPARAM id);
	void RemoveTab(LPARAM id);
public:
	MenuWindow(const win::dpi::Dpi& dpi,const std::unordered_map<uintptr_t, std::shared_ptr<cwnd::Context>>& contexts,std::function<void(std::shared_ptr<cwnd::Context>)> newContext,std::function<void()> contextUpdate,Config& config,std::function<ShellContextFactory*(std::string)> getFactory,  std::function<std::shared_ptr<void>(std::string)> getResource):
		m_dpi(dpi),
		m_contexts(contexts),
		m_newContext(newContext),
		m_contextUpdate(contextUpdate),
		m_getFactory(getFactory),
		m_getResource(getResource),
		m_config(config),
		m_new_shell(config.initshell),
		m_icon_font(NULL)
	{

	}
	~MenuWindow() {
		if (m_icon_font != NULL) {
			DeleteObject(m_icon_font);
		}
		auto ttf = win::GetModuleFilePath(NULL);
		ttf += _T("\\fonts\\menu.ttf");
		RemoveFontResourceExW(ttf.c_str(),FR_PRIVATE,NULL);
	}
	static constexpr DIP m_menu_height = 20;
	static std::unique_ptr<MenuWindow> Create(HINSTANCE hinst,
		HWND parent, 
		const win::dpi::Dpi& dpi,
		DIP x, DIP y, DIP w,
		HMENU hmenu,
		const std::unordered_map<uintptr_t, std::shared_ptr<cwnd::Context>>& contexts,
		std::function<void(std::shared_ptr<cwnd::Context>)>,
		std::function<void()> contextUpdate,
		Config& conf,
		std::function<ShellContextFactory*(std::string_view)>,
		std::function<std::shared_ptr<void>(std::string_view)> getResource);
	HWND GetHWnd();
	std::shared_ptr<cwnd::Context> GetCurrentContext(DIP width,DIP height);

	void OnDpiChange();
};
}
