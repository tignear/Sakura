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
shared_ptr<BasicShellContext> BasicShellContext::Create(
	tstring cmdstr,
	shared_ptr<iocp::IOCPMgr> iocpmgr,
	unsigned int codepage,
	std::unordered_map<unsigned int,uint32_t> colorsys,
	std::unordered_map<unsigned int, uint32_t> color256,
	bool use_terminal_echoback,
	std::vector<std::wstring> fontmap,
	double fontsize,
	const Options& opt
) {
	Attribute attr{};
	attr.frColor.type = ColorType::ColorSystem;
	attr.frColor.color_system = 30;
	attr.bgColor.type = ColorType::ColorSystem;
	attr.bgColor.color_system = 47;
	auto r = make_shared<BasicShellContext>(iocpmgr,codepage,colorsys, color256, use_terminal_echoback, fontmap,fontsize,attr);
	r->SetSystemColor(colorsys);
	r->Set256Color(color256);
	if (r->Init(cmdstr,opt))
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
bool BasicShellContext::Init(tstring cmdstr,const Options& opt) {
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
	if (opt.use_count_chars) {
		si.dwFlags |= STARTF_USECOUNTCHARS;
		si.dwXCountChars = opt.x_count_chars;
		si.dwXCountChars = opt.y_count_chars;
	}
	if (opt.use_size) {
		si.dwXSize = opt.width;
		si.dwYSize = opt.height;
	}
	auto len = cmdstr.length();
	auto cmd = std::make_unique<TCHAR[]>(len+1);
#pragma warning(disable:4996)
	cmdstr.copy(cmd.get(), len);
#pragma warning(default:4996)
	m_iocpmgr->Attach(m_out_pipe);

	if (!CreateProcess(NULL, cmd.get(), &sa, &sa, TRUE, CREATE_NEW_CONSOLE, opt.environment,opt.current_directory, &si, &pi)) {
		auto le=GetLastError();
		OutputDebugStringW(std::to_wstring(le).c_str());
		return false;
	}
	CloseHandle(out_client_pipe);
	CloseHandle(pi.hThread);
	m_childProcess = pi.hProcess;
	std::thread th([this,pid=pi.dwProcessId,hProcess=pi.hProcess]() {
		while ((!m_hwnd)&& WaitForSingleObject(hProcess,200)== WAIT_TIMEOUT) {
			m_hwnd = GetHwndFromProcess(pid);
		}
		WaitForSingleObject(hProcess, INFINITE);
		NotifyExit();
	});
	th.detach();
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
	s->AddString(cp_to_wide(s->m_outbuf.c_str(),s->m_codepage,cnt));
	return s->OutputWorker(s);
}
void BasicShellContext::InputKey(WPARAM keycode) {
	if (!m_hwnd) {
		return;
	}
	PostMessage(m_hwnd,WM_KEYDOWN,keycode,0);
}
void BasicShellContext::InputKey(WPARAM keycode, unsigned int count) {
	for (auto i = 0U; i < count; ++i) {
		InputKey(keycode);
	}
}
void BasicShellContext::InputChar(WPARAM charcode) {
	if (!m_hwnd) {
		return;
	}
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
	std::lock_guard lock(m_lock);
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
uintptr_t BasicShellContext::AddLayoutChangeListener(std::function<void(ShellContext*)> f)const {
	auto key = reinterpret_cast<uintptr_t>(&f);
	m_layout_change_listeners[key] = f;
	return key;
}
void BasicShellContext::RemoveLayoutChangeListener(uintptr_t key)const{
	m_layout_change_listeners.erase(key);
}
uintptr_t BasicShellContext::AddExitListener(std::function<void(ShellContext*)> f)const {
	auto key = reinterpret_cast<uintptr_t>(&f);
	m_exit_listeners[key] = f;
	return key;
}
void BasicShellContext::RemoveExitListener(uintptr_t key)const {
	m_exit_listeners.erase(key);
}
std::wstring::size_type BasicShellContext::GetViewCount()const {
	return m_document.GetViewCount();
}
void BasicShellContext::SetPageSize(size_t count) {
	m_document.SetPageSize(count);
}
size_t  BasicShellContext::GetViewStart()const {
	return m_document.GetViewPosition();
}
void  BasicShellContext::SetViewStart(size_t s) {
	m_document.SetViewPosition(s);
}
std::wstring_view BasicShellContext::GetTitle()const {
	return m_title;
}
void BasicShellContext::Set256Color(const std::unordered_map<unsigned int, uint32_t>& table256) {
	m_document.Set256ColorTable(table256);
}
void BasicShellContext::Set256Color(const std::unordered_map<unsigned int, uint32_t>&& table256) {
	m_document.Set256ColorTable(table256);
}
void BasicShellContext::SetSystemColor(const std::unordered_map<unsigned int, uint32_t>& tablesys) {
	m_document.SetSystemColorTable(tablesys);
}
void BasicShellContext::SetSystemColor(const std::unordered_map<unsigned int, uint32_t>&& tablesys) {
	m_document.SetSystemColorTable(std::move(tablesys));
}
void BasicShellContext::Lock() {
	m_lock.lock();
}
void BasicShellContext::Unlock() {
	m_lock.unlock();
}
size_t BasicShellContext::GetLineCount()const {
	return m_document.GetLineCount();
}
void BasicShellContext::NotifyLayoutChange() {
	for (auto&& f : m_layout_change_listeners) {
		f.second(this);
	}
}
void BasicShellContext::NotifyTextChange() {
	for (auto&& f : m_text_change_listeners) {
		f.second(this);
	}
}
void BasicShellContext::NotifyExit() {
	for (auto&& f : m_exit_listeners) {
		f.second(this);
	}
}
void BasicShellContext::Resize(UINT w,UINT h) {
	if (m_hwnd) {
		SetWindowPos(m_hwnd, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW | SWP_ASYNCWINDOWPOS);

	}
}
double BasicShellContext::FontSize()const {
	return m_fontsize;
};
bool BasicShellContext::UseTerminalEchoBack()const {
	return m_use_terminal_echoback;
};
const std::wstring& BasicShellContext::DefaultFont()const {
	return m_fontmap.at(m_document.GetDefaultAttribute().font);
}
const ShellContext::attrtext_document& BasicShellContext::GetAll()const {
	return m_document.GetAll();
}
const ShellContext::attrtext_document& BasicShellContext::GetView()const {
	return m_document.GetView();
}
//static fields
std::atomic_uintmax_t BasicShellContext::m_process_count = 0;