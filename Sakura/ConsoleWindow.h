#pragma once
#include <Windows.h>
#include <tchar.h>
#include<wrl.h>
#include "ConsoleWindowTextArea.h"
namespace tignear::sakura {
	class ConsoleWindow {
		static constexpr HMENU m_hmenu_textarea=(HMENU)0x01;
		static constexpr HMENU m_hmenu_scrollbar =(HMENU)0x02;

		Microsoft::WRL::ComPtr<ConsoleWindowTextArea> m_textarea;
		HWND m_parent_hwnd;
		HWND m_hwnd;
		HWND m_scrollbar_hwnd;
		HINSTANCE m_hinst;
		static bool m_registerstate;
		static bool RegisteConsoleWindowrClass(HINSTANCE hinst);
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		void OnCreate();
	public:
		static constexpr LPCTSTR m_classname=_T("ConsoleWindow");
		static std::unique_ptr<ConsoleWindow> Create(HINSTANCE, HWND, int x, int y, int w, int h, HMENU, ITfThreadMgr* threadmgr, TfClientId, ITfCategoryMgr* cate_mgr, ITfDisplayAttributeMgr* attr_mgr, ID2D1Factory*, IDWriteFactory*);
		void SetContext(std::shared_ptr<cwnd::Context>);
		HWND GetHWnd();
	};
}