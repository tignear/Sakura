#include "pch.h"
#include <ConsoleReadShellContextCommon.h>
#include <tstring.h>
#include <thread>
#include <GetHwndFromPid.h>
#include <tchar.h>
#include <iostream>
#include <nonsugar.hpp>
#include <string>
#include <Windows.h>
#include <cassert>
#include <MessageQueue.h>
using namespace tignear::sakura::conread;
struct WinEvent {
	HWINEVENTHOOK hWinEventHook;
	DWORD event;
	HWND hwnd;
	LONG idObject;
	LONG idChild;
	DWORD idEventThread;
	DWORD dwmsEventTime;
};
namespace {
	tignear::thread::MessageQueue<WinEvent> g_events;
	tignear::sakura::conread::MappingView g_view;
	HWND g_child;
	HWND g_parent_hwnd;
	WPARAM g_id;
	//SHORT g_beginY = 0;
	//SHORT g_size = 0;
	enum class Args {
		HELP, CMD, CURRENT_DIR,/* MUTEX,*/ PID, SHELLCONTEXT_PTR,/*FONT_SIZE*/
	};

	void ProcessEvent(HANDLE outbuf, const WinEvent& eve) {
		using namespace tignear::sakura::conread::exe2context;
		switch (eve.event) {
		case EVENT_CONSOLE_CARET:
		{
			auto x = LOWORD(eve.idChild);
			auto y = HIWORD(eve.idChild);
			g_view.info()->cursorX = x;
			g_view.info()->cursorY = y;
			PostMessage(g_parent_hwnd, static_cast<UINT>(UPDATE_CARET), g_id, x << 16 | y);
			break;
		}
		case EVENT_CONSOLE_UPDATE_SIMPLE:
		{
			uint64_t x = LOWORD(eve.idObject);
			uint64_t y = HIWORD(eve.idObject);
			SHORT c = LOWORD(eve.idChild);
			SHORT attr = HIWORD(eve.idChild);
			auto i = y * g_view.info()->allocateWidth + x;
			g_view.buf()[i] = c;
			g_view.attribute()[i] = attr;
			PostMessage(g_parent_hwnd, static_cast<UINT>(UPDATE_SIMPLE), g_id, static_cast<uint64_t>(attr) << 48 | static_cast<uint64_t>(c) << 32 | static_cast<uint64_t>(x) << 16 | y);
			break;
		}
		case EVENT_CONSOLE_UPDATE_REGION:
		{
			uint64_t xs = LOWORD(eve.idObject);
			uint64_t ys = HIWORD(eve.idObject);
			uint64_t xe = LOWORD(eve.idChild);
			uint64_t ye = HIWORD(eve.idChild);
			for (SHORT line = static_cast<SHORT>(ys); line <= ye; ++line) {
				auto i = line * g_view.info()->allocateWidth;
				auto w = g_view.info()->width;
				DWORD read;
				ReadConsoleOutputCharacterW(outbuf, g_view.buf()+i, w, { 0 ,line }, &read);
				ReadConsoleOutputAttribute(outbuf, g_view.attribute()+i, w, { 0,line }, &read);
			}
			PostMessage(g_parent_hwnd, static_cast<UINT>(UPDATE_REGION), g_id, xe << 48 | ye << 32 | xs << 16 | ys);
			break;
		}
		case EVENT_CONSOLE_UPDATE_SCROLL:
		{
			auto vertical = eve.idChild;
			if (vertical == 0) {
				break;
			}
			PostMessage(g_parent_hwnd, static_cast<UINT>(UPDATE_SCROLL), g_id, vertical);
			break;
		}

		default:
			break;
		}
	}
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	using namespace context2exe;
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case INPUT_KEY://Input key
	{
		INPUT_RECORD rec[2];
		rec[0].EventType = KEY_EVENT;
		rec[0].Event.KeyEvent.bKeyDown = TRUE;
		rec[0].Event.KeyEvent.wRepeatCount = 1;
		rec[0].Event.KeyEvent.wVirtualScanCode = (lParam >> 16) & 0xff;
		rec[0].Event.KeyEvent.dwControlKeyState = GetKeyState(VK_SHIFT) ? SHIFT_PRESSED : 0;//TODO
		rec[0].Event.KeyEvent.uChar.UnicodeChar = 0;//static_cast<WCHAR>(MapVirtualKeyW(static_cast<UINT>(wParam), MAPVK_VK_TO_CHAR));
		rec[0].Event.KeyEvent.wVirtualKeyCode = static_cast<WORD>(wParam);
		memcpy(&rec[1], &rec[0], sizeof(rec[1]));
		rec[1].Event.KeyEvent.bKeyDown = FALSE;
		rec[1].Event.KeyEvent.wRepeatCount = 0;
		DWORD write;
		WriteConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), &rec[0], 2, &write);
		break;
	}
	case INPUT_CHAR://input char
	{
		INPUT_RECORD rec[2];
		rec[0].EventType = KEY_EVENT;
		rec[0].Event.KeyEvent.bKeyDown = TRUE;
		rec[0].Event.KeyEvent.wRepeatCount = 1;
		rec[0].Event.KeyEvent.dwControlKeyState = GetKeyState(VK_SHIFT) ? SHIFT_PRESSED : 0;//TODO
		rec[0].Event.KeyEvent.uChar.UnicodeChar = static_cast<WCHAR>(wParam);
		rec[0].Event.KeyEvent.wVirtualScanCode = (lParam >> 16) & 0xff;
		rec[0].Event.KeyEvent.wVirtualKeyCode = lParam ? static_cast<WORD>(MapVirtualKeyW((lParam >> 16) & 0xff, MAPVK_VSC_TO_VK_EX)) : 0;
		memcpy(&rec[1], &rec[0], sizeof(rec[1]));
		rec[1].Event.KeyEvent.bKeyDown = FALSE;
		rec[1].Event.KeyEvent.wRepeatCount = 0;
		DWORD write;
		WriteConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), &rec[0], 2, &write);
		break;
	}
	case WM_CLOSE:
		g_events.postMessage({});
		return DefWindowProc(hwnd, message, wParam, lParam);
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}


void CALLBACK WinEventProc(
	HWINEVENTHOOK hWinEventHook,
	DWORD event,
	HWND hwnd,
	LONG idObject,
	LONG idChild,
	DWORD idEventThread,
	DWORD dwmsEventTime
) {
	g_events.postMessage(
		{
			hWinEventHook,
			event,
			hwnd,
			idObject,
			idChild,
			idEventThread,
			dwmsEventTime
		}
	);
}
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE,
	LPSTR,
	int) {
	using namespace nonsugar;
	const auto cmd = command<Args>("sakura-DefaultShellContext-ConsoleReader")
		.flag<Args::HELP>({ 'h' }, { "help" }, "", "produce help message", flag_type::exclusive)
		.flag<Args::CURRENT_DIR, std::string>({ 'c' }, { "currentDir" }, "CURRENT_DIR", "set current directory", "")
		//.flag<Args::FONT_SIZE, double>({}, {"fontSize"},"FONT_SIZE","")
		.argument<Args::CMD, std::string>("CMD")
		//.argument<Args::MUTEX, std::string>("MUTEX")
		.argument<Args::PID, DWORD>("PID")
		.argument<Args::SHELLCONTEXT_PTR, uintptr_t>("SHELLCONTEXT_PTR");
	try {
		auto const opts = parse(__argc, __argv, cmd);
		if (opts.has<Args::HELP>()) {
			std::cout << usage(cmd);
			return EXIT_SUCCESS;
		}
		FreeConsole();

		static TCHAR szWindowClass[] = _T("tignear.sakura.ConsoleReader");
		static TCHAR szTitle[] = _T("ConsoleReader");

		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = NULL;
		wcex.lpfnWndProc = &WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = szWindowClass;
		wcex.hIconSm = NULL;

		if (!RegisterClassEx(&wcex))
		{
			return EXIT_FAILURE;
		}
		//CreateWindow
		auto hwnd = CreateWindow(
			szWindowClass,
			szTitle,
			NULL,
			CW_USEDEFAULT, CW_USEDEFAULT,
			0, 0,
			NULL,
			NULL,
			hInstance,
			NULL
		);
		if (!hwnd)
		{
			return EXIT_FAILURE;
		}
		if (!SetWinEventHook(EVENT_CONSOLE_CARET, EVENT_CONSOLE_END_APPLICATION, NULL, &WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT| WINEVENT_SKIPOWNPROCESS)) {
			PostMessage(hwnd, WM_CLOSE, 0, 0);
		}

		//UIPI
		if (!(ChangeWindowMessageFilterEx(hwnd, WM_APP + 3, MSGFLT_ALLOW, NULL) &&
			ChangeWindowMessageFilterEx(hwnd, WM_APP + 2, MSGFLT_ALLOW, NULL)&&
			ChangeWindowMessageFilterEx(hwnd,WM_APP+1,MSGFLT_ALLOW,NULL))) {
			return EXIT_FAILURE;
		}
		g_parent_hwnd=tignear::win32::GetHwndFromProcess(opts.get<Args::PID>());
		g_id = opts.get<Args::SHELLCONTEXT_PTR>();

		//Create Process
		auto cmdstr = opts.get<Args::CMD>();
		auto cdir = opts.get<Args::CURRENT_DIR>();

		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;
		PROCESS_INFORMATION pi{};
		STARTUPINFO si{};
		si.cb = sizeof(si);
		si.dwFlags =  STARTF_USESHOWWINDOW;
		si.hStdError = NULL;
		si.hStdOutput = NULL;
		si.hStdInput = NULL;
		si.wShowWindow = SW_HIDE;
		auto len = cmdstr.length();
		auto cmdca = std::make_unique<TCHAR[]>(len + 1);
#pragma warning(disable:4996)
		cmdstr.copy(cmdca.get(), len);
#pragma warning(default:4996)
		if (!CreateProcessA(NULL, cmdca.get(), &sa, &sa, TRUE, CREATE_NEW_CONSOLE|CREATE_NEW_PROCESS_GROUP, NULL, cdir.empty() ? NULL : cdir.c_str(), &si, &pi)) {
			auto le = GetLastError();
			OutputDebugStringW(std::to_wstring(le).c_str());
			return EXIT_FAILURE;
		}

		auto hprocess = pi.hProcess;
		CloseHandle(pi.hThread);
		while (WaitForSingleObject(hprocess, 200) == WAIT_TIMEOUT) {
			g_child = tignear::win32::GetHwndFromProcess(pi.dwProcessId);

			if (g_child) {
				if (!AttachConsole(pi.dwProcessId)) {
					return EXIT_FAILURE;
				}
				break;
			}
		}
		auto console_outbuf_handle=GetStdHandle(STD_OUTPUT_HANDLE);



		CONSOLE_SCREEN_BUFFER_INFOEX csbie{};
		csbie.cbSize = sizeof(csbie);
		GetConsoleScreenBufferInfoEx(console_outbuf_handle, &csbie);
		auto w = csbie.dwSize.X;
		auto h = csbie.dwSize.Y;
		g_view = CreateMappingViewW((std::wstring(L"tignear.sakura.ConsoleReadShellContext.") + std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(0)).c_str(), w, h);
		auto* info = g_view.info();

		info->width = w;
		info->height = h;
		info->cursorX = csbie.dwCursorPosition.X;
		info->cursorY = csbie.dwCursorPosition.Y;
		info->viewSize = h;
		auto aw = info->allocateWidth;
		for (SHORT line = 0; line < h; ++line) {
			DWORD read=0;
			auto index = line * aw;
			
			ReadConsoleOutputCharacterW(console_outbuf_handle, g_view.buf()+index, w, {0,line },&read);
			ReadConsoleOutputAttribute(console_outbuf_handle, g_view.attribute()+index, w, { 0,line }, &read);
		}
		PostMessage(g_parent_hwnd, exe2context::CREATE_VIEW, g_id, 0);
		std::thread worker([console_outbuf_handle, hwnd]() {


			while (true)
			{
				auto future = g_events.message();
				auto&& e = future.get();
				if (!e.event) {
					return;
				}
				ProcessEvent(console_outbuf_handle, std::move(e));
			}


		});


		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			DispatchMessage(&msg);
		}
		TerminateProcess(hprocess,EXIT_FAILURE);
		worker.join();
		return EXIT_SUCCESS;
	}
	catch (error const &e) {
		std::cerr << e.message() << std::endl;
		return EXIT_FAILURE;
	}
}