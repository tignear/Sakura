#include "stdafx.h"
#include "strconv.h"
#include "split.h"
#include "BasicShellContext.h"
#include "ansi/AttributeText.h"
#include "GetHwndFromPid.h"
using namespace tignear;
using namespace sakura;
using namespace stdex;
using namespace ansi;
using std::shared_ptr;
using std::make_shared;
using iocp::IOCPInfo;
using tignear::win32::GetHwndFromProcess;
shared_ptr<BasicShellContext> BasicShellContext::Create(tstring cmdstr, shared_ptr<iocp::IOCPMgr> iocpmgr,unsigned int codepage,std::unordered_map<unsigned int,uint32_t> colorsys, std::unordered_map<unsigned int, uint32_t> color256) {
	auto r = make_shared<BasicShellContext>(iocpmgr,codepage);
	r->SetSystemColor(colorsys);
	r->Set256Color(color256);
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

	++m_process_count;

	m_out_pipe = CreateNamedPipe(out_pipename.c_str(), PIPE_ACCESS_INBOUND| PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE, 2, BUFFER_SIZE, BUFFER_SIZE, 1000, &sa);
	if (m_out_pipe == INVALID_HANDLE_VALUE) {
		//auto le = GetLastError();
		return false;
	}
	auto out_client_pipe = CreateFile(out_pipename.c_str(),
		GENERIC_WRITE|GENERIC_READ,
		0,
		&sa,
		OPEN_EXISTING,
		NULL,
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
	s->AddString(cp_to_wide(s->m_outbuf.c_str(),s->m_codepage,cnt));
	return s->OutputWorker(s);
}
void BasicShellContext::InputKey(WPARAM keycode) {

	PostMessage(m_hwnd,WM_KEYDOWN,keycode,0);
}
void BasicShellContext::InputKey(WPARAM keycode, unsigned int count) {
	for (auto i = 0U; i < count; ++i) {
		InputKey(keycode);
	}
}
void BasicShellContext::InputChar(WPARAM charcode) {
	/*if (0x08 == charcode) {
		return;
	}*/
	if (charcode <= 127) {
		return;
	}
	PostMessage(m_hwnd, WM_CHAR, charcode, 0);
}
void BasicShellContext::InputString(std::wstring_view wstr) {
	for (auto c : wstr) {
		InputChar(c);
	}
}
void BasicShellContext::ConfirmString(std::wstring_view view) {
	AddString(view);
}
void BasicShellContext::AddString(std::wstring_view str) {
	ansi::parseW(str, *this);
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
std::wstring::size_type BasicShellContext::GetViewLineCount()const {
	return m_viewline_count;
}
void BasicShellContext::SetViewLineCount(std::wstring::size_type count) {
	m_viewline_count = count;
}
std::wstring_view BasicShellContext::GetTitle()const {
	return m_title;
}
void BasicShellContext::Set256Color(const std::unordered_map<unsigned int, uint32_t>& table256) {
	m_256_color_table = table256;
}
void BasicShellContext::Set256Color(const std::unordered_map<unsigned int, uint32_t>&& table256) {
	m_256_color_table = std::move(table256);
}
void BasicShellContext::SetSystemColor(const std::unordered_map<unsigned int, uint32_t>& tablesys) {
	m_system_color_table = tablesys;
}
void BasicShellContext::SetSystemColor(const std::unordered_map<unsigned int, uint32_t>&& tablesys) {
	m_system_color_table = std::move(tablesys);
}
void BasicShellContext::Lock() {
	m_lock.lock();
}
void BasicShellContext::Unlock() {
	m_lock.unlock();
}
BasicShellContext::attrtext_iterator BasicShellContext::begin()const {
	return attrtext_iterator(std::make_unique<BasicShellContext::attrtext_iterator_impl>(m_viewstartY_itr, m_text.cend()));
}
BasicShellContext::attrtext_iterator BasicShellContext::end() const{
	return attrtext_iterator(std::make_unique<BasicShellContext::attrtext_iterator_impl>(m_text.cend(), m_text.cend()));
}
//static fields
std::atomic_uintmax_t BasicShellContext::m_process_count = 0;