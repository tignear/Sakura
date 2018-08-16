#include "stdafx.h"
#include "strconv.h"
#include "split.h"
#include "BasicShellContext.h"
#include "BasicAttributeText.h"
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
	s->m_outbuf.assign(BUFFER_SIZE, '\0');
	//DWORD cnt;
	if(!ReadFile(
		s->m_out_pipe,
		s->m_outbuf.data(), 
		BUFFER_SIZE, 
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
	//OutputDebugStringA(s->m_outbuf.c_str());
	s->AddString(cp_to_wide(s->m_outbuf.c_str(),932,static_cast<int>(cnt)));
	return s->OutputWorker(s); ;
}
void BasicShellContext::InputKey(WPARAM keycode) {
	PostMessage(m_hwnd,WM_KEYDOWN,keycode,0);
}
void BasicShellContext::InputKey(WPARAM keycode, unsigned int count) {
	for (auto i = 0U; i < count; i++) {
		InputKey(keycode);
	}
}
void BasicShellContext::InputChar(WPARAM charcode) {
	// do nothing
	//PostMessage(m_hwnd, WM_CHAR, charcode, 0);
}
void BasicShellContext::InputString(std::wstring_view wstr) {
	for (auto c : wstr) {
		PostMessage(m_hwnd, WM_CHAR, c,0);
	}
}
void BasicShellContext::ConfirmString(std::wstring_view view) {
	AddString(view);
}
const std::list<std::list<AttributeText*>>& BasicShellContext::GetText() const{
	return m_text;
}
std::wstring_view BasicShellContext::GetString()const {
	return m_buffer;
}
void BasicShellContext::AddString(std::wstring_view str) {
	using tignear::stdex::split;
	//m_buffer.reserve(str.length());
	auto r = split < wchar_t, std::list < std::wstring_view >>(std::wstring_view{ str }, L"\r\n");
	auto temp = r.back();
	r.pop_back();
	for (auto e : r) {
		m_buffer += e;
		m_buffer += L"\r\n";
		auto atext = dynamic_cast<BasicAttributeText*>(m_text.back().back());
		atext->length(atext->length()+e.length() + 2);
		m_text.push_back({new BasicAttributeText(m_buffer,atext->startIndex()+atext->length(),0)});
		//OutputDebugStringW(e.c_str());
	}
	if (temp.empty()) {
		auto atext = m_text.back().back();
		m_text.push_back({ new BasicAttributeText(m_buffer,atext->startIndex()+atext->length(),0)});
	}
	else {
		m_buffer += temp;
		auto atext = dynamic_cast<BasicAttributeText*>(m_text.back().back());
		auto len = atext->length();
		auto len2=temp.length();
		atext->length(len+len2);
	}
}
unsigned int BasicShellContext::GetCursorX()const {
	return m_cursorX;
}
unsigned int BasicShellContext::GetCursorY()const {
	return m_cursorY;
}
uintptr_t BasicShellContext::AddTextChangeListener(std::function<void(ShellContext*)> f) const{
	auto key = reinterpret_cast<uintptr_t>(&f);
	m_text_change_listeners[key]=f;
	return key;
}
void BasicShellContext::RemoveTextChangeListener(uintptr_t key)const {
	m_text_change_listeners.erase(key);
}
uintptr_t BasicShellContext::AddCursorChangeListener(std::function<void(ShellContext*)>)const {
	return 0;
}
void BasicShellContext::RemoveCursorChangeListener(uintptr_t)const{}
//static fiels
std::atomic_uintmax_t BasicShellContext::m_process_count = 0;