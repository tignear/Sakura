#include "stdafx.h"
#include <future>
#include "split.h"
#include "BasicShellContext.h"
#include "GetHwndFromPid.h"
using namespace tignear::sakura;
using namespace tignear::stdex;
using std::shared_ptr;
using std::make_shared;
using iocp::IOCPInfo;
using tignear::win32::GetHwndFromProcess;
shared_ptr<BasicShellContext> BasicShellContext::Create(tstring cmdstr, shared_ptr<iocp::IOCPMgr> iocpmgr) {
	auto r = make_shared<BasicShellContext>(iocpmgr);
	if (r->Init(cmdstr))
	{
		if (!r->IOWorkerStart(r)) {
			r.reset();
			return r;
		}
		return r;
	}
	else
	{
		r.reset();
		return r;
	}

}
bool BasicShellContext::Init(tstring cmdstr) {

	//http://yamatyuu.net/computer/program/sdk/base/cmdpipe1/index.html
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	tstring out_pipename = _T("\\\\.\\pipe\\tignear.sakura.BasicShellContext.out.");
#ifdef UNICODE
	auto str_thread_id = std::to_wstring(GetCurrentThreadId());
	auto str_process_cnt = std::to_wstring(m_process_count);
#else
	auto str_thread_id = std::to_string(GetCurrentThreadId());
	auto str_process_cnt = std::to_string(m_process_count);
#endif
	out_pipename += str_thread_id;
	out_pipename += _T(".");
	out_pipename += str_process_cnt;

	m_process_count++;
	m_out_pipe = CreateNamedPipe(out_pipename.c_str(), PIPE_ACCESS_INBOUND| FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE, 2, BUFFER_SIZE, BUFFER_SIZE, 1000, &sa);
	if (m_out_pipe == INVALID_HANDLE_VALUE) {
		//auto le = GetLastError();
		return false;
	}
	auto out_client_pipe = CreateFile(out_pipename.c_str(),
		GENERIC_WRITE,
		0,
		&sa,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if (out_client_pipe == INVALID_HANDLE_VALUE) {
		return false;
	}
	PROCESS_INFORMATION pi{};
	STARTUPINFO si{};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES| STARTF_USESHOWWINDOW;
	si.hStdOutput = out_client_pipe;
	si.hStdError = out_client_pipe;
	si.hStdInput = NULL;
	si.wShowWindow = SW_HIDE;
	auto len = cmdstr.length();
	auto cmd = std::make_unique<TCHAR[]>(len+1);
#pragma warning(disable:4996)
	cmdstr.copy(cmd.get(), len);
#pragma warning(default:4996)
	m_iocpmgr->Attach(m_out_pipe);

	if (!CreateProcess(NULL, cmd.get(), &sa, &sa, TRUE, CREATE_NEW_CONSOLE, NULL, _T("C:\\users\\tignear"), &si, &pi)) {
		auto le=GetLastError();
		OutputDebugStringW(std::to_wstring(le).c_str());
		return false;
	}
	CloseHandle(out_client_pipe);
	CloseHandle(pi.hThread);
	m_childProcess = pi.hProcess;
	Sleep(2000);
	m_hwnd=GetHwndFromProcess(pi.dwProcessId);
	return true;
}

bool BasicShellContext::IOWorkerStart(shared_ptr<BasicShellContext> s) {
	return OutputWorker(s);
}
bool BasicShellContext::OutputWorker(shared_ptr<BasicShellContext> s) {
	auto info = new IOCPInfo{
		{},
		[s](DWORD readCnt) {
			s->OutputWorkerHelper(readCnt,s);
		}
	};
	memset(s->m_outbuf, 0, sizeof(s->m_outbuf));
	//DWORD cnt;
	if(!ReadFile(
		s->m_out_pipe,
		s->m_outbuf, 
		BUFFER_SIZE-1, 
		NULL,
		&info->overlapped
	)){
		auto le = GetLastError();
		if (le != ERROR_IO_PENDING) {
			return false;
		}
	}
	return true;
}
bool BasicShellContext::OutputWorkerHelper(DWORD cnt,shared_ptr<BasicShellContext> s) {
	wchar_t cs[BUFFER_SIZE];
#pragma warning(disable:4996)
	mbstowcs(cs, s->m_outbuf, BUFFER_SIZE);
#pragma warning(default:4996)
	OutputDebugStringW(cs);
	OutputDebugString(_T("\n"));
	s->AddString(cs);
	return s->OutputWorker(s); ;
}
void BasicShellContext::InputKey(WPARAM keycode) {
	PostMessage(m_hwnd,WM_KEYDOWN,keycode,0);
	//PostMessage(m_hwnd, WM_KEYUP, keycode, 0);
}
void BasicShellContext::InputChar(WPARAM charcode) {
	PostMessage(m_hwnd, WM_CHAR, charcode, 0);
}
void BasicShellContext::InputString(std::wstring wstr) {
	for (auto c : wstr) {
		PostMessage(m_hwnd, WM_CHAR, c,0);
	}
}
std::list<LineText> BasicShellContext::GetText() {
	return m_text;
}
void BasicShellContext::AddString(std::wstring str) {
	auto r=tignear::stdex::split<wchar_t,std::vector < std::wstring>> (str, L"\n");
	if (m_text.empty()) {
		AttributeText at;
		at.text = r[0];
		m_text.push_back({at});
		m_text.push_back({ {} });
	}
	else
	{
		m_text.back().back().text += r[0];
		m_text.push_back({ {} });
	}
	for (auto i = 1U; i < r.size(); i++) {
		m_text.back().back().text += r[i];
		m_text.push_back({ {} });
	}
}
unsigned int BasicShellContext::GetCursorX() {
	return 0U;
}
unsigned int BasicShellContext::GetCursorY() {
	return 0U;
}
//static fiels
std::atomic_uintmax_t BasicShellContext::m_process_count = 0;