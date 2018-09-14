#include "stdafx.h"
#include <string>
#include "FailToThrow.h"
#include "SakuraMain.h"
#include "IOCPMgr.h"
#include "BasicShellContextFactory.h"
#include "DefinedResource.h"
using tignear::sakura::Sakura;
using tignear::sakura::ConsoleWindow;
using tignear::sakura::MenuWindow;
using Microsoft::WRL::ComPtr;
using tignear::FailToThrowHR;
using tignear::sakura::iocp::IOCPMgr;
using tignear::sakura::ColorTable;
int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine,
	int nCmdShow)
{
	CoInitialize(NULL);
	auto r= Sakura::Main(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	CoUninitialize();
	return r;
}

LRESULT CALLBACK Sakura::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_SETFOCUS:
		if (m_console) {
			if (SetFocus(m_console->GetHWnd()) == NULL) {
				DestroyWindow(hWnd);
				break;
			}
		}
		break;

	case WM_SIZING:
	case WM_SIZE:
	{
		Size();
		break;

	}
	case WM_DPICHANGED:
	{
		m_dpi.SetDpi(LOWORD(wParam));
		LPRECT rect= reinterpret_cast<LPRECT>(lParam);
		SetWindowPos(hWnd,
			NULL,
			rect->left,
			rect->top,
			rect->right - rect->left,
			rect->bottom - rect->top,
			SWP_NOZORDER | SWP_NOACTIVATE);
		m_menu->OnDpiChange();
		m_console->OnDpiChange();

		break;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
void Sakura::Size() {
	RECT rect;

	GetClientRect(m_sakura, &rect);

	auto&& m_height_px = Sakura::m_dpi.Pixel(MenuWindow::m_menu_height);
	if (m_console) {
		SetWindowPos(m_console->GetHWnd(), 0, rect.left, rect.top + m_height_px, rect.right - rect.left, rect.bottom - rect.top - m_height_px, SWP_NOOWNERZORDER);
	}
	if (m_menu) {
		SetWindowPos(m_menu->GetHWnd(), 0, rect.left, rect.top, rect.right - rect.left, m_height_px, SWP_NOOWNERZORDER);
	}
}
int Sakura::Main(HINSTANCE hInstance,
	HINSTANCE,
	LPTSTR,
	int)
{
	appInstance = hInstance;
	WNDCLASS wc;
	setlocale(LC_CTYPE, "JPN");

	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = appInstance;
	wc.hCursor = NULL;
	wc.hIcon = NULL;
	wc.hbrBackground = 0;
	wc.lpszClassName = className;
	wc.lpszMenuName = NULL;

	if (!RegisterClass(&wc)) {
		return -1;
	}
	m_sakura=CreateWindowEx(
		0,      
		className,  
		_T("Sakura"), 
		WS_OVERLAPPEDWINDOW,      
		CW_USEDEFAULT,              
		CW_USEDEFAULT,            
		CW_USEDEFAULT,         
		CW_USEDEFAULT,          
		NULL,      
		NULL,       
		appInstance,  
		NULL      
	);

	FailToThrowHR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&m_d2d_factory)));

	FailToThrowHR(DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,__uuidof(IDWriteFactory),&m_dwrite_factory
	));

	FailToThrowHR(CoCreateInstance(CLSID_TF_ThreadMgr,
		nullptr,
		CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&m_thread_mgr)));

	FailToThrowHR(CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_category_mgr)));

	FailToThrowHR(CoCreateInstance(CLSID_TF_DisplayAttributeMgr, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_attribute_mgr)));
	//m_attribute_mgr->AddRef();

	m_thread_mgr->Activate(&m_clientId);


	RECT rect;
	GetClientRect(m_sakura, (LPRECT)&rect);
	auto iocpmgr = std::make_shared<IOCPMgr>();
	m_resource[resource::IOCPMgr] = iocpmgr;
	m_factory.emplace("BasicShellContextFactory", std::make_unique<BasicShellContextFactory>());

	auto config = LoadConfig("config.lua");
	auto ishell = config.shells[config.initshell];
	m_menu = MenuWindow::Create(
		hInstance,
		m_sakura,
		m_dpi,
		0,
		0,
		m_dpi.Dip(rect.right - rect.left),
		m_menu_hmenu,
		[]() {
			if (m_console) {
				Sakura::m_console->ReGetConsoleContext();
			}
		},
		config,
		[](std::string k) {
		return Sakura::m_factory.at(k).get();
		},
		[](std::string k) {
		return Sakura::m_resource.at(k);
	}
	);
	m_console = ConsoleWindow::Create(
		hInstance,
		m_sakura,
		m_dpi,
		0, 
		MenuWindow::m_menu_height, 
		static_cast<FLOAT>(rect.right - rect.left), 
		static_cast<FLOAT>(rect.bottom - rect.top - MenuWindow::m_menu_height),
		m_console_hmenu,
		m_thread_mgr.Get(),
		m_clientId,
		m_category_mgr.Get(),
		m_attribute_mgr.Get(),
		m_d2d_factory.Get(),
		m_dwrite_factory.Get(),
		std::function([](DIP w,DIP h) {
			return Sakura::m_menu->GetCurrentContext(w, h);
		})
	);

	ShowWindow(m_sakura, SW_SHOWDEFAULT);
	UpdateWindow(m_sakura);
	auto r= Run();
	m_thread_mgr->Deactivate();
	return r;
}
int Sakura::Run() {

	ComPtr<ITfKeystrokeMgr> keyMgr;
	ComPtr<ITfMessagePump> msgPump;

	if (FAILED(m_thread_mgr.As(&keyMgr)))
	{
		return 1;
	}
	if (FAILED(m_thread_mgr.As(&msgPump)))
	{
		return 1;
	}
	for (;;)
	{
		MSG msg;
		BOOL fResult;
		BOOL fEaten;
		BOOL focus;
		m_thread_mgr->IsThreadFocus(&focus);
		OutputDebugString(focus ? _T("") : _T("×"));
		try {
			if (FAILED(msgPump->GetMessage(&msg, 0, 0, 0, &fResult)))
			{
				return -1;
			}
			else if (msg.message == WM_KEYDOWN)
			{
				if (keyMgr->TestKeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
					keyMgr->KeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten)
				{
					continue;
				}
			}
			else if (msg.message == WM_KEYUP)
			{
				if (keyMgr->TestKeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
					keyMgr->KeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten)
				{
					continue;
				}
			}

			if (fResult == 0)
			{
				return static_cast<int>(msg.wParam);
			}
			else if (fResult == -1)
			{
				return -1;
			}

		}
		catch(...)
		{
			//MS_IMEはキーを長押しし続けるとTestKeyDownでアクセス違反を引き起こすので握りつぶす。
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
//static fields
HWND Sakura::m_sakura;
HINSTANCE Sakura::appInstance;
Microsoft::WRL::ComPtr<ITfThreadMgr> Sakura::m_thread_mgr;
TfClientId Sakura::m_clientId;
Microsoft::WRL::ComPtr<ID2D1Factory> Sakura::m_d2d_factory;
Microsoft::WRL::ComPtr<IDWriteFactory> Sakura::m_dwrite_factory;
std::unique_ptr<ConsoleWindow> Sakura::m_console;
std::unique_ptr<MenuWindow> Sakura::m_menu;
ComPtr<ITfCategoryMgr> Sakura::m_category_mgr;
ComPtr<ITfDisplayAttributeMgr> Sakura::m_attribute_mgr;
std::unordered_map<std::string, std::shared_ptr<void>> Sakura::m_resource;
std::unordered_map<std::string, std::unique_ptr<tignear::sakura::ShellContextFactory>> Sakura::m_factory;
tignear::win::dpi::Dpi Sakura::m_dpi= tignear::win::dpi::Dpi(GetDeviceCaps(GetDC(0), LOGPIXELSX));