#pragma once
#include <Windows.h>
#include <tchar.h>
#include<wrl.h>
#include "ConsoleWindowTextArea.h"
#include "ShellContextFactory.h"
#include "SakuraConfig.h"
namespace tignear::sakura {
	class ConsoleWindow {
		static constexpr HMENU m_hmenu_textarea=(HMENU)0x01;
		static constexpr HMENU m_hmenu_scrollbar =(HMENU)0x02;
		static constexpr UINT m_scrollbar_width = 15;
		static constexpr UINT WM_UPDATE_SCROLLBAR = WM_USER + 0x0001;
		Microsoft::WRL::ComPtr<ConsoleWindowTextArea> m_textarea;
		HWND m_parent_hwnd;
		HWND m_hwnd;
		HWND m_scrollbar_hwnd;
		HWND m_tab_hwnd;
		HINSTANCE m_hinst;
		static bool m_registerstate;
		static bool RegisterConsoleWindowClass(HINSTANCE hinst);
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		void OnLayoutChange(ShellContext* shell);
		void UpdateScrollBar();
		std::unordered_map<std::string, std::unique_ptr<tignear::sakura::ShellContextFactory>> m_factory;
		Config m_config;
		std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<void>>> m_resource;
		std::shared_ptr<cwnd::Context> m_console;
	public:
		static constexpr LPCTSTR m_classname=_T("ConsoleWindow");
		static std::unique_ptr<ConsoleWindow> Create(HINSTANCE,
			HWND, 
			int x, int y,unsigned int w,unsigned int h,
			HMENU,
			ITfThreadMgr* threadmgr, 
			TfClientId,
			ITfCategoryMgr* cate_mgr,
			ITfDisplayAttributeMgr* attr_mgr,
			ID2D1Factory*,
			IDWriteFactory*,
			Config&&,
			std::function<tignear::sakura::ShellContextFactory*(std::string)>,
			std::function<std::shared_ptr<void>(std::string)> getResourceFn);
		HWND GetHWnd();
	};
}