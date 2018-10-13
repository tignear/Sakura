#include "stdafx.h"
#include <strconv.h>
#include <ansi/AttributeText.h>
#include "split.h"
#include "BasicShellContext.h"
using namespace tignear;
using namespace sakura;
using namespace stdex;
using namespace ansi;
using std::shared_ptr;
using std::make_shared;
using iocp::IOCPInfo;



bool BasicShellContext::OutputWorker(shared_ptr<BasicShellContext> s) {
	auto info = new IOCPInfo{
		{},
		[w=std::weak_ptr(s)](DWORD readCnt) {
			auto s=w.lock();
			if (!s) {
				return;
			}
			OutputWorkerHelper(readCnt,s);
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
	s->m_outbuf.resize(cnt);
	s->AddString(s->m_outbuf);
	return OutputWorker(s);
}
void BasicShellContext::InputKey(WPARAM keycode,LPARAM lp) {
	if (!m_hwnd) {
		return;
	}
	PostMessage(m_hwnd,WM_KEYDOWN,keycode,lp);
}
void BasicShellContext::InputKey(WPARAM keycode, unsigned int count) {
	for (auto i = 0U; i < count; ++i) {
		InputKey(keycode,static_cast<LPARAM>(1));
	}
}
void BasicShellContext::InputChar(WPARAM charcode,LPARAM) {
	if (!m_hwnd) {
		return;
	}
	if (charcode <= 127) {
		return;
	}
	PostMessage(m_hwnd, WM_CHAR, charcode, 1);
}
void BasicShellContext::InputString(std::wstring_view wstr) {
	for (auto c : wstr) {
		InputChar(c,0);
	}
}
void BasicShellContext::ConfirmString(std::wstring_view view) {
	std::lock_guard lock(m_lock);
	m_document.Insert(std::wstring(view));
}
void BasicShellContext::AddString(std::string_view str) {
	std::lock_guard lock(m_lock);
	ansi::parseA(str, *this);
}

uintptr_t BasicShellContext::AddTextChangeListener(std::function<void(ShellContext*,std::vector<TextUpdateInfoLine>)> f) const{
	std::lock_guard lock(m_lock);
	auto key = reinterpret_cast<uintptr_t>(&f);
	m_text_change_listeners[key]=f;
	return key;
}
void BasicShellContext::RemoveTextChangeListener(uintptr_t key)const {
	std::lock_guard lock(m_lock);
	m_text_change_listeners.erase(key);
}
uintptr_t BasicShellContext::AddLayoutChangeListener(std::function<void(ShellContext*,bool,bool)> f)const {
	std::lock_guard lock(m_lock);
	auto key = reinterpret_cast<uintptr_t>(&f);
	m_layout_change_listeners[key] = f;
	return key;
}
void BasicShellContext::RemoveLayoutChangeListener(uintptr_t key)const{
	std::lock_guard lock(m_lock);
	m_layout_change_listeners.erase(key);
}
uintptr_t BasicShellContext::AddExitListener(std::function<void(ShellContext*)> f)const {
	std::lock_guard lock(m_lock);
	auto key = reinterpret_cast<uintptr_t>(&f);
	m_exit_listeners[key] = f;
	return key;
}
void BasicShellContext::RemoveExitListener(uintptr_t key)const {
	std::lock_guard lock(m_lock);
	m_exit_listeners.erase(key);
}
std::wstring::size_type BasicShellContext::GetViewCount()const {
	//std::lock_guard lock(m_lock);
	return m_document.GetViewCount();
}
void BasicShellContext::SetPageSize(size_t count) {
	std::lock_guard lock(m_lock);
	m_document.SetPageSize(count);
}
size_t  BasicShellContext::GetViewStart()const {
	std::lock_guard lock(m_lock);
	return m_document.GetViewPosition();
}
void  BasicShellContext::SetViewStart(size_t s) {
	std::lock_guard lock(m_lock);
	m_document.SetViewPosition(s);
}
std::wstring_view BasicShellContext::GetTitle()const {
	std::lock_guard lock(m_lock);
	return m_title;
}

void BasicShellContext::Lock() {
	m_lock.lock();
}
void BasicShellContext::Unlock() {
	m_lock.unlock();
}
LRESULT BasicShellContext::OnMessage(UINT,LPARAM) {
	return 0;
}
size_t BasicShellContext::GetLineCount()const {
	//std::lock_guard lock(m_lock);
	return m_document.GetLineCount();
}
void BasicShellContext::NotifyLayoutChange(bool x,bool y) {
	for (auto&& f : m_layout_change_listeners) {
		f.second(this,x,y);
	}
}
void BasicShellContext::NotifyTextChange(std::vector<TextUpdateInfoLine> v) {
	for (auto&& f : m_text_change_listeners) {
		f.second(this,v);
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
uint32_t BasicShellContext::BackgroundColor()const {
	return m_bg_color;
}
uint32_t BasicShellContext::FrontColor()const {
	return m_fr_color;
}
const std::wstring& BasicShellContext::DefaultFont()const {
	return m_fontmap.at(m_document.GetDefaultAttribute().font);
}
BasicShellContextLineText& BasicShellContext::GetCursorY(){
	std::lock_guard lock(m_lock);
	return m_document.GetCursorY();
}
ShellContext::attrtext_line_iterator BasicShellContext::GetCursorYItr(){
	return m_document.GetCursorYItr();
}

size_t BasicShellContext::GetCursorXWStringPos()const {
	std::lock_guard lock(m_lock);
	return m_document.GetCursorXWStringPos();
}
ShellContext::attrtext_document& BasicShellContext::GetAll() {
	return m_document.GetAll();
}
ShellContext::attrtext_document& BasicShellContext::GetView() {
	return m_document.GetView();
}
//static fields
std::atomic_uintmax_t BasicShellContext::m_process_count = 0;