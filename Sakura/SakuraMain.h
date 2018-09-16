#pragma once
#include <Windows.h>
#include <unordered_map>
#include <wrl.h>
#include "MenuWindow.h"
#include "ConsoleWindow.h"
namespace tignear::sakura {
	class Sakura {
	public:
		static constexpr const TCHAR* className = _T("tignear.sakura.Sakura");
		static constexpr const HMENU m_console_hmenu=(HMENU)0x01;
		static constexpr const HMENU m_menu_hmenu = (HMENU)0x02;
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static int Main(HINSTANCE hInstance,
			HINSTANCE hPrevInstance,
			LPTSTR lpCmdLine,
			int nCmdShow);
		static int Run();
		static void Size();
	private:
		static HWND m_sakura;
		static HINSTANCE appInstance;
		static std::unique_ptr<tignear::sakura::ConsoleWindow> m_console;
		static std::unique_ptr<tignear::sakura::MenuWindow> m_menu;
		static Microsoft::WRL::ComPtr<ITfThreadMgr> m_thread_mgr;
		static Microsoft::WRL::ComPtr<ID2D1Factory> m_d2d_factory;
		static Microsoft::WRL::ComPtr<IDWriteFactory> m_dwrite_factory;
		static TfClientId m_clientId;
		static Microsoft::WRL::ComPtr<ITfCategoryMgr> m_category_mgr;
		static Microsoft::WRL::ComPtr<ITfDisplayAttributeMgr> m_attribute_mgr;
		static std::unordered_map<std::string,std::shared_ptr<void>>  m_resource;
		static std::unordered_map<std::string, std::unique_ptr<ShellContextFactory>> m_factory;
		static std::unordered_map<uintptr_t, std::shared_ptr<cwnd::Context>> m_contexts;
		static win::dpi::Dpi m_dpi;
};
}
