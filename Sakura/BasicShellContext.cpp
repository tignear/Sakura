#include "stdafx.h"
#include <future>
#include "BasicShellContext.h"
using namespace tignear::sakura;
using namespace tignear::stdex;
using std::shared_ptr;
using std::make_shared;
using iocp::IOCPInfo;
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
#pragma warning(disable:4189)

	//http://yamatyuu.net/computer/program/sdk/base/cmdpipe1/index.html
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	tstring out_pipename = _T("\\\\.\\pipe\\tignear.sakura.BasicShellContext.out.");
	tstring in_pipename = _T("\\\\.\\pipe\\tignear.sakura.BasicShellContext.in.");
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

	in_pipename += str_thread_id;
	in_pipename += _T(".");
	in_pipename += str_process_cnt;
	m_process_count++;
	m_out_pipe = CreateNamedPipe(out_pipename.c_str(), PIPE_ACCESS_INBOUND| FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE, 2, BUFFER_SIZE, BUFFER_SIZE, 1000, &sa);
	if (m_out_pipe == INVALID_HANDLE_VALUE) {
		auto le = GetLastError();
		return false;
	}
	auto m_out_client_pipe = CreateFile(out_pipename.c_str(),
		GENERIC_WRITE,
		0,
		&sa,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if (m_out_client_pipe == INVALID_HANDLE_VALUE) {
		auto le = GetLastError();
		return false;
	}

	m_in_pipe = CreateNamedPipe(in_pipename.c_str(), PIPE_ACCESS_OUTBOUND|FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE, 2, BUFFER_SIZE, BUFFER_SIZE, 1000, &sa);
	if (m_in_pipe == INVALID_HANDLE_VALUE) {
		auto le = GetLastError();
		return false;
	}
	auto m_in_client_pipe = CreateFile(in_pipename.c_str(),
		GENERIC_READ,
		0,
		&sa,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);
	if (m_in_client_pipe == INVALID_HANDLE_VALUE) {
		auto le = GetLastError();
		return false;
	}
	PROCESS_INFORMATION pi{};
	STARTUPINFO si{};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = m_out_client_pipe;
	si.hStdError = m_out_client_pipe;
	si.hStdInput = m_in_client_pipe;
	si.wShowWindow = SW_HIDE;
	auto len = cmdstr.length();
	auto cmd = std::make_unique<TCHAR[]>(len+1);
#pragma warning(disable:4996)
	cmdstr.copy(cmd.get(), len);
#pragma warning(default:4996)
	m_iocpmgr->Attach(m_out_pipe);

	if (!CreateProcess(NULL, cmd.get(), &sa, &sa, TRUE, 0, NULL, NULL, &si, &pi)) {
		auto le=GetLastError();
		
		return false;
	}
	CloseHandle(pi.hThread);
	m_childProcess = pi.hProcess;
	return true;
}
void BasicShellContext::AddString(tstring str) {
	buffer += str;
	OutputDebugString(buffer.c_str());
	OutputDebugString(_T("\n"));
}
bool BasicShellContext::IOWorkerStart(shared_ptr<BasicShellContext> s) {
	OutputWorker(s);
	return true;
}
void BasicShellContext::OutputWorker(shared_ptr<BasicShellContext> s) {
	auto info = new IOCPInfo{
		{},
		[s](DWORD readCnt) {
#ifdef UNICODE
		wchar_t cs[BUFFER_SIZE];
#pragma warning(disable:4996)
		mbstowcs(cs, s->m_outbuf, BUFFER_SIZE);
#pragma warning(default:4996)
#else
		char* cs = s->m_outbuf;
#endif
		s->AddString(cs);
		s->OutputWorker(s);
	}
	};

	auto ol2 = new OVERLAPPED{};
	if(!ReadFile(
		s->m_out_pipe,
		s->m_outbuf, 
		BUFFER_SIZE*sizeof(char), 
		NULL,
		&info->overlapped
	)){
		auto le = GetLastError();
		return;
	}
	OutputDebugStringA(m_outbuf);
}
//static fiels
std::atomic_uintmax_t BasicShellContext::m_process_count = 0;