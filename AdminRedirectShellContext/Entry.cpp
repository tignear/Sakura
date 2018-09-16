#include "pch.h"
#include <atomic>
#include <thread>
#include <DefinedMessage.h>
#include <GetHWndFrompid.h>
#include <tchar.h>
#include <Windows.h>
#include <iostream>
#include <nonsugar.hpp>
enum class Args {
	HELP,CMD,CURRENT_DIR,PIPE_NAME,PID,SHELLCONTEXT_PTR
};
std::atomic<HWND> g_child=0;
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		return PostMessage(g_child,message,wParam,lParam);
	case WM_CHAR:
		return PostMessage(g_child, message, wParam, lParam);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE,
	LPSTR,
	int)
{
	using namespace nonsugar;
	const auto cmd = command<Args>("sakura-DefaultShellContext-AdminRedirector")
		.flag<Args::HELP>({ 'h' }, { "help" }, "", "produce help message", flag_type::exclusive)
		.flag<Args::CURRENT_DIR, std::string>({ 'c' }, { "currentDir" }, "CURRENT_DIR","set current directory","")
		.argument<Args::CMD,std::string>("CMD")
		.argument<Args::PIPE_NAME, std::string>("PIPENAME")
		.argument<Args::PID,DWORD>("PID")
		.argument<Args::SHELLCONTEXT_PTR,uintptr_t>("SHELLCONTEXT_PTR");
	try {
		auto const opts = parse(__argc, __argv, cmd);
		if (opts.has<Args::HELP>()) {
			std::cout << usage(cmd);
			return EXIT_SUCCESS;
		}

		static TCHAR szWindowClass[] = _T("tignear.sakura.AdminRedirectShellContext");
		static TCHAR szTitle[] = _T("AdminRedirectShellContext");

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
		if (!(ChangeWindowMessageFilterEx(hwnd, WM_CHAR, MSGFLT_ALLOW, NULL) &&
			ChangeWindowMessageFilterEx(hwnd, WM_KEYDOWN, MSGFLT_ALLOW, NULL))) {
			return EXIT_FAILURE;
		}
		//Create Process
		auto cmdstr= opts.get<Args::CMD>();
		auto pipename = opts.get<Args::PIPE_NAME>();
		auto cdir = opts.get<Args::CURRENT_DIR>();

		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;
		auto out_client_pipe = CreateFileA(pipename.c_str(),
			GENERIC_WRITE | GENERIC_READ,
			0,
			&sa,
			OPEN_EXISTING,
			NULL,
			NULL);
		PROCESS_INFORMATION pi{};
		STARTUPINFO si{};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.hStdError = out_client_pipe;
		si.hStdOutput = out_client_pipe;
		si.wShowWindow = SW_HIDE;
		auto len = cmdstr.length();
		auto cmdca = std::make_unique<TCHAR[]>(len + 1);
#pragma warning(disable:4996)
		cmdstr.copy(cmdca.get(), len);
#pragma warning(default:4996)

		if (!CreateProcessA(NULL, cmdca.get(), &sa, &sa, TRUE, CREATE_NEW_CONSOLE, NULL, cdir.empty() ? NULL : cdir.c_str(), &si, &pi)) {
			auto le = GetLastError();
			OutputDebugStringW(std::to_wstring(le).c_str());
			return EXIT_FAILURE;
		}
		auto hprocess = pi.hProcess;
		CloseHandle(pi.hThread);
		CloseHandle(out_client_pipe);
		auto th=std::thread(
			[
				hwnd,
				hprocess,
				parent= tignear::win32::GetHwndFromProcess(opts.get<Args::PID>()),
				pid=pi.dwProcessId,param= opts.get<Args::SHELLCONTEXT_PTR>()
			]()->void {
			while (WaitForSingleObject(hprocess, 200) == WAIT_TIMEOUT) {
				g_child = tignear::win32::GetHwndFromProcess(pid);
				if (g_child) {
					SendMessage(parent, WM_SHELLCONTEXTMESSAGE, param, reinterpret_cast<LPARAM>(hwnd));
					break;
				}
			}
			if (WaitForSingleObject(hprocess, INFINITE) != WAIT_OBJECT_0) {
				PostQuitMessage(EXIT_FAILURE);
				return;
			}
			DWORD exitCode;
			if (!GetExitCodeProcess(hprocess, &exitCode)) {
				CloseHandle(hprocess);
				PostQuitMessage(EXIT_FAILURE);
				return;
			}
			CloseHandle(hprocess);
			PostQuitMessage(exitCode);
		});
		th.detach();
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			//TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return (int)msg.wParam;


	}
	catch (error const &e) {
		std::cerr << e.message() <<std::endl;
		return EXIT_FAILURE;
	}


}