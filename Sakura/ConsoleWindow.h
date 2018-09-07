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
		static constexpr HMENU m_hmenu_column_scrollbar =(HMENU)0x02;
		static constexpr HMENU m_hmenu_row_scrollbar = (HMENU)0x03;
		static constexpr UINT m_scrollbar_width = 15;

		static constexpr UINT WM_UPDATE_SCROLLBAR = WM_USER + 0x0001;
		Microsoft::WRL::ComPtr<ConsoleWindowTextArea> m_textarea;
		HWND m_parent_hwnd;
		HWND m_hwnd;
		HWND m_tab_hwnd;
		HWND m_scrollbar_column_hwnd;
		HWND m_scrollbar_row_hwnd;
		HINSTANCE m_hinst;
		static bool m_registerstate;
		static bool RegisterConsoleWindowClass(HINSTANCE hinst);
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		void OnLayoutChange(ShellContext* shell);
		void UpdateScrollBar();
		std::shared_ptr<cwnd::Context> m_console;
		void SetConsoleContext(std::shared_ptr<cwnd::Context>);
		std::function<std::shared_ptr<cwnd::Context>(unsigned int w, unsigned int h)> m_getContext;
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
			std::function<std::shared_ptr<cwnd::Context>(unsigned int w,unsigned int h)> getContext);
		void ReGetConsoleContext();
		HWND GetHWnd();
	};
}