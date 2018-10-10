#pragma once
#include <Windows.h>
#include <unordered_map>
#include <wrl.h>
#include "PluginLoader.h"
#include "MenuWindow.h"
#include "ConsoleWindow.h"
namespace tignear::sakura {
	class Sakura {
	public:
		static constexpr const TCHAR* className = _T("tignear.sakura.Sakura");
		static constexpr const HMENU m_console_hmenu=(HMENU)0x01;
		static constexpr const HMENU m_menu_hmenu = (HMENU)0x02;
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		int Main(HINSTANCE hInstance,
			HINSTANCE hPrevInstance,
			LPTSTR lpCmdLine,
			int nCmdShow);
		int Run();
		void Size();
		Sakura():m_dpi(win::dpi::Dpi(GetDeviceCaps(GetDC(0), LOGPIXELSX))) {}
	private:
		HINSTANCE appInstance;
		win::dpi::Dpi m_dpi;
		HWND m_hwnd;
		PluginManager  m_plugin_mgr;
		std::unordered_map<uintptr_t, std::shared_ptr<cwnd::Context>> m_contexts;
		std::unordered_map<std::string_view, std::shared_ptr<void>>  m_resource;
		std::unordered_map<std::string_view, std::unique_ptr<ShellContextFactory>> m_factory;
		std::unique_ptr<tignear::sakura::ConsoleWindow> m_console;
		std::unique_ptr<tignear::sakura::MenuWindow> m_menu;

		Microsoft::WRL::ComPtr<ITfThreadMgr> m_thread_mgr;
		Microsoft::WRL::ComPtr<ID2D1Factory> m_d2d_factory;
		Microsoft::WRL::ComPtr<IDWriteFactory> m_dwrite_factory;
		TfClientId m_clientId;
		Microsoft::WRL::ComPtr<ITfCategoryMgr> m_category_mgr;
		Microsoft::WRL::ComPtr<ITfDisplayAttributeMgr> m_attribute_mgr;
	public:
		static Sakura* m_sakura;

};
}
