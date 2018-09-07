#pragma once
#include "SakuraConfig.h"
#include "ShellContextFactory.h"
#include "ConsoleWindowContext.h"
#include "ExecutableFilePath.h"
#include "tstring.h"
#include <deque>
#include <memory>
namespace tignear::sakura {
class MenuWindow {
	static const constexpr HMENU m_hmenu_tab = (HMENU)0x01;
	static const constexpr HMENU m_hmenu_menu = (HMENU)0x02;

	static const constexpr LPCTSTR m_classname = _T("tignear.sakura.MenuWindow");
	static const constexpr unsigned int  m_menu_button_width=20;
	static bool m_registerstate;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static bool RegisterMenuWindowClass(HINSTANCE hinst);
	std::deque<std::shared_ptr<cwnd::Context>> m_contexts;
	const std::function<ShellContextFactory*(std::string)> m_getFactory;
	const std::function<std::shared_ptr<void>(std::string)> m_getResource;

	const Config& m_config;
	HFONT m_icon_font;
	HWND m_parent_hwnd;
	HWND m_hwnd;
	HWND m_tab_hwnd;
	HWND m_menu_button_hwnd;
	bool m_new;
	int m_current_context_pos;
public:
	MenuWindow(Config& config,std::function<ShellContextFactory*(std::string)> getFactory,  std::function<std::shared_ptr<void>(std::string)> getResource):
		m_getFactory(getFactory),
		m_getResource(getResource),
		m_config(config),
		m_new(true)
	{
		auto ttf = stdex::tstring(win::GetExecutableFilePath());
		ttf += _T("\\fonts\\menu.ttf");
		auto r=AddFontResourceEx(ttf.c_str(), FR_PRIVATE, NULL);
		if (r == 0) {
			throw std::runtime_error("font resource adding fail");
		}
		LOGFONT lf{};
		TCHAR fname[] = _T("icomoon");
		lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
		lf.lfQuality = CLEARTYPE_QUALITY;
		memcpy(lf.lfFaceName,fname,sizeof(fname));
		m_icon_font=CreateFontIndirect(&lf);
	}
	~MenuWindow() {
		auto ttf = stdex::tstring(win::GetExecutableFilePath());
		ttf += _T("\\fonts\\menu.ttf");
		RemoveFontResourceEx(ttf.c_str(),FR_PRIVATE,NULL);
	}
	static constexpr UINT m_menu_height = 20;
	static std::unique_ptr<MenuWindow> Create(HINSTANCE hinst,
		HWND parent, 
		int x, int y, unsigned int w, 
		HMENU hmenu,
		Config& conf,
		std::function<ShellContextFactory*(std::string)>,
		std::function<std::shared_ptr<void>(std::string)> getResource);
	HWND GetHWnd();
	std::shared_ptr<cwnd::Context> GetCurrentContext(unsigned int width,unsigned int height);
};
}
