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
using namespace tignear::sakura;

HWND g_child;
SHORT g_beginY=0;
SHORT g_size=0;
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_APP+2://Input key
	{
		INPUT_RECORD rec[2];
		rec[0].EventType = KEY_EVENT;
		rec[0].Event.KeyEvent.bKeyDown = TRUE;
		rec[0].Event.KeyEvent.wRepeatCount = 1;
		rec[0].Event.KeyEvent.wVirtualScanCode = (lParam >> 16) & 0xff;
		rec[0].Event.KeyEvent.dwControlKeyState = GetKeyState(VK_SHIFT) ? SHIFT_PRESSED : 0;//TODO
		rec[0].Event.KeyEvent.uChar.UnicodeChar = 0;//static_cast<WCHAR>(MapVirtualKeyW(static_cast<UINT>(wParam), MAPVK_VK_TO_CHAR));
		rec[0].Event.KeyEvent.wVirtualKeyCode =static_cast<WORD>(wParam);
		memcpy(&rec[1], &rec[0], sizeof(rec[1]));
		rec[1].Event.KeyEvent.bKeyDown = FALSE;
		rec[1].Event.KeyEvent.wRepeatCount = 0;
		DWORD write;
		WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &rec[0], 2, &write);
		break;
	}
	case  WM_APP + 3://input char
	{
		INPUT_RECORD rec[2];
		rec[0].EventType = KEY_EVENT;
		rec[0].Event.KeyEvent.bKeyDown = TRUE;
		rec[0].Event.KeyEvent.wRepeatCount = 1;
		rec[0].Event.KeyEvent.dwControlKeyState = GetKeyState(VK_SHIFT) ? SHIFT_PRESSED : 0;//TODO
		rec[0].Event.KeyEvent.uChar.UnicodeChar = static_cast<WCHAR>(wParam);
		rec[0].Event.KeyEvent.wVirtualScanCode = (lParam >> 16) & 0xff;
		rec[0].Event.KeyEvent.wVirtualKeyCode = lParam ? static_cast<WORD>(MapVirtualKeyW((lParam >> 16)&0xff, MAPVK_VSC_TO_VK_EX)) : 0;
		memcpy(&rec[1], &rec[0], sizeof(rec[1]));
		rec[1].Event.KeyEvent.bKeyDown = FALSE;
		rec[1].Event.KeyEvent.wRepeatCount = 0;
		DWORD write;
		WriteConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), &rec[0], 2, &write);
		break;
	}
	case WM_APP+1:
	{
		switch (wParam) {
		case 0:
			g_beginY =static_cast<SHORT>(lParam);
			break;
		case 1://SetPageSize
			g_size = static_cast<SHORT>(lParam);
			break;
		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
	}
	
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}
enum class Args {
	HELP, CMD, CURRENT_DIR,/* MUTEX,*/ PID, SHELLCONTEXT_PTR,/*FONT_SIZE*/
};
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE,
	LPSTR,
	int) {
	using namespace nonsugar;
	FreeConsole();

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

		static TCHAR szWindowClass[] = _T("tignear.sakura.ConsoleReader");
		static TCHAR szTitle[] = _T("ConsoleReader");

		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = NULL;
		wcex.lpfnWndProc = WndProc;
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

		//UIPI
		if (!(ChangeWindowMessageFilterEx(hwnd, WM_APP + 3, MSGFLT_ALLOW, NULL) &&
			ChangeWindowMessageFilterEx(hwnd, WM_APP + 2, MSGFLT_ALLOW, NULL)&&
			ChangeWindowMessageFilterEx(hwnd,WM_APP+1,MSGFLT_ALLOW,NULL))) {
			return EXIT_FAILURE;
		}
		auto parent=tignear::win32::GetHwndFromProcess(opts.get<Args::PID>());
		auto id = opts.get<Args::SHELLCONTEXT_PTR>();
		SendMessage(parent, WM_APP+1,id, reinterpret_cast<LPARAM>(hwnd));

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

		DWORD exitcode = 0;
		auto th_out = std::thread([hprocess,console_outbuf_handle,parent,id,hwnd,&exitcode]{
			//SHORT nowpos=0;
			MappingView view;
			unsigned int num = 0;
			while (WaitForSingleObject(hprocess, 100)==WAIT_TIMEOUT) {
				CONSOLE_SCREEN_BUFFER_INFOEX info;
				info.cbSize = sizeof(info);
				if (!GetConsoleScreenBufferInfoEx(console_outbuf_handle, &info)) {

					exitcode = EXIT_FAILURE;
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					return;
				}

				if (!view || info.dwSize.X > view.info()->allocateWidth || g_size > view.info()->allocateHeight) {
					auto name = std::wstring(L"tignear.sakura.ConsoleReadShellContext.") + std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(num);

					view=CreateMappingViewW(name.c_str(), info.dwSize.X,info.srWindow.Bottom-info.srWindow.Top);
					view.info()->viewBeginY = g_beginY;
					view.info()->viewSize =g_size;
					view.info()->width = info.dwSize.X;
					view.info()->height = info.dwSize.Y;
					view.info()->cursorX = info.dwCursorPosition.X;
					view.info()->cursorY = info.dwCursorPosition.Y;
					GetConsoleTitleW(view.info()->title, MappingInfo::TITLE_LENGTH);
					for (short i = 0; i < view.info()->viewSize; ++i) {
						DWORD read_char;
						ReadConsoleOutputCharacterW(console_outbuf_handle, view.buf() + i * view.info()->allocateWidth, view.info()->width, { 0,view.info()->viewBeginY + i }, &read_char);
						DWORD read_attr;
						ReadConsoleOutputAttribute(console_outbuf_handle, view.attribute() + i * view.info()->allocateWidth, view.info()->width, { 0,view.info()->viewBeginY + 1 }, &read_attr);
					}
					SendMessage(parent,WM_APP+2,id,num);
					SendMessage(parent, WM_APP + 3, id,0);
					++num;
					continue;
				}
				
				view.info()->viewBeginY = g_beginY;
				view.info()->viewSize = g_size;
				view.info()->width = info.dwSize.X;
				view.info()->height = info.dwSize.Y;
				view.info()->cursorX = info.dwCursorPosition.X;
				view.info()->cursorY = info.dwCursorPosition.Y;
				GetConsoleTitleW(view.info()->title,MappingInfo::TITLE_LENGTH);	
				for (short i =0; i < view.info()->viewSize; ++i) {
					DWORD read_char;
					ReadConsoleOutputCharacterW(console_outbuf_handle, view.buf() + i * view.info()->allocateWidth, view.info()->width, {0,view.info()->viewBeginY+i},&read_char);
					DWORD read_attr;
					ReadConsoleOutputAttribute(console_outbuf_handle, view.attribute() + i * view.info()->allocateWidth, view.info()->width, { 0,view.info()->viewBeginY + 1 }, &read_attr);
				}
				
				PostMessage(parent, WM_APP + 3, id, 0);
			}
			if (!GetExitCodeProcess(hprocess, &exitcode)) {
				CloseHandle(hprocess);
				exitcode = EXIT_FAILURE;
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				return;
			}
			CloseHandle(hprocess);

			PostMessage(hwnd, WM_CLOSE, 0, 0);
		});
		
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			DispatchMessage(&msg);
		}
		TerminateProcess(hprocess,EXIT_FAILURE);
		th_out.join();
		return exitcode;
	}
	catch (error const &e) {
		std::cerr << e.message() << std::endl;
		return EXIT_FAILURE;
	}
}