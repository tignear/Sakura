#include "stdafx.h"
#include <iostream>
#include <nonsugar.hpp>
#include <Windows.h>
#include <thread>
enum class Args {
	HELP, CMD, CURRENT_DIR, IN_PIPE_NAME,OUT_PIPE_NAME
};
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}
//https://github.com/Microsoft/console/blob/master/samples/ConPTY/EchoCon/EchoCon/EchoCon.cpp#L91
bool SetUpPesudoConsole(COORD size,std::string in_pipename, std::string out_pipename,HPCON* hpcon) {
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	auto in_client_pipe = CreateFileA(in_pipename.c_str(),
		GENERIC_READ| GENERIC_WRITE,
		0,
		&sa,
		OPEN_EXISTING,
		NULL,
		NULL);
	if (in_client_pipe == INVALID_HANDLE_VALUE) {
		return false;
	}
	auto out_client_pipe = CreateFileA(out_pipename.c_str(),
		GENERIC_READ|GENERIC_WRITE,
		0,
		&sa,
		OPEN_EXISTING,
		NULL,
		NULL);
	if (out_client_pipe == INVALID_HANDLE_VALUE) {
		return false;
	}
	auto r= SUCCEEDED(CreatePseudoConsole(size,  in_client_pipe,out_client_pipe, 0, hpcon));
	CloseHandle(in_client_pipe);
	CloseHandle(out_client_pipe);
	return r;
}
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE ,
	LPSTR ,
	int 
) {
	using namespace nonsugar;
	const auto cmd = command<Args>("sakura-ConPtyShellContext-PseudoConsole")
		.flag<Args::HELP>({ 'h' }, { "help" }, "", "produce help message", flag_type::exclusive)
		.flag<Args::CURRENT_DIR, std::string>({ 'c' }, { "currentDir" }, "CURRENT_DIR", "set current directory", "")
		.argument<Args::CMD, std::string>("CMD")
		.argument<Args::IN_PIPE_NAME, std::string>("IN_PIPENAME")
		.argument<Args::OUT_PIPE_NAME, std::string>("OUT_PIPENAME");
	try {
		auto const opts = parse(__argc, __argv, cmd);
		if (opts.has<Args::HELP>()) {
			std::cout << usage(cmd);
			return EXIT_SUCCESS;
		}
		static TCHAR szWindowClass[] = _T("tignear.sakura.ConPtyShellContext");
		static TCHAR szTitle[] = _T("ConPtyShellContext");
		FreeConsole();
		AllocConsole();
		CONSOLE_SCREEN_BUFFER_INFOEX csbie{};
		csbie.cbSize = sizeof(csbie);
		GetConsoleScreenBufferInfoEx(
			GetStdHandle(STD_OUTPUT_HANDLE),
			&csbie
		);
		HPCON con;
		if (!SetUpPesudoConsole(csbie.dwMaximumWindowSize, opts.get<Args::IN_PIPE_NAME>(), opts.get<Args::OUT_PIPE_NAME>(), &con)) {
			return EXIT_FAILURE;
		}

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
		auto cmdstr = opts.get<Args::CMD>();

		auto cdir = opts.get<Args::CURRENT_DIR>();

		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;

		PROCESS_INFORMATION pi{};
		STARTUPINFOEXA sie{};
		sie.StartupInfo.cb = sizeof(sie);
		//sie.StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
		sie.StartupInfo.wShowWindow = SW_HIDE;

		size_t attrListSize{};


		// Get the size of the thread attribute list.
		InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);

		// Allocate a thread attribute list of the correct size
		sie.lpAttributeList =
			reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(malloc(attrListSize));

		// Initialize thread attribute list
		if (sie.lpAttributeList
			&& InitializeProcThreadAttributeList(sie.lpAttributeList, 1, 0, &attrListSize))
		{
			// Set Pseudo Console attribute
			if (FAILED(UpdateProcThreadAttribute(
				sie.lpAttributeList,
				0,
				PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
				con,
				sizeof(HPCON),
				NULL,
				NULL))) {
				return EXIT_FAILURE;
			}
		}
		auto len = cmdstr.length();
		auto cmdca = std::make_unique<CHAR[]>(len + 1);
#pragma warning(disable:4996)
		cmdstr.copy(cmdca.get(), len);
#pragma warning(default:4996)

		if (!CreateProcessA(NULL, cmdca.get(), &sa, &sa, TRUE, EXTENDED_STARTUPINFO_PRESENT, NULL, cdir.empty() ? NULL : cdir.c_str(),& sie.StartupInfo, &pi)) {
			auto le = GetLastError();
			OutputDebugStringW(std::to_wstring(le).c_str());
			return EXIT_FAILURE;
		}
		auto hprocess = pi.hProcess;
		CloseHandle(pi.hThread);

		DWORD exitcode = 0;
		auto th = std::thread(
			[
				hwnd,
				hprocess,
				pid = pi.dwProcessId,
				&exitcode
			]()->void {
			if (WaitForSingleObject(hprocess, INFINITE) != WAIT_OBJECT_0) {
				exitcode = EXIT_FAILURE;
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				return;
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
		th.join();
		//ClosePseudoConsole(con);
		return exitcode;

	}
	catch (error const &e) {
		std::cerr << e.message() << std::endl;
		return EXIT_FAILURE;
	}
}