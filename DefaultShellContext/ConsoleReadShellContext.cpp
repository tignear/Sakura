#include "stdafx.h"
#include <ProcessTree.h>
#include <shellapi.h>
#include "ConsoleReadShellContext.h"
namespace tignear::sakura {
	using namespace stdex;
	std::wstring_view ConsoleReadShellContext::GetStringAtLineCount(int lc) {
		if (!m_view) {
			return std::wstring_view(m_nodata_text);
		}
		auto flc = lc - m_view.info()->viewBeginY;
		if (flc>=0&&flc<m_view.info()->viewSize) {
			return std::wstring_view(m_view.buf() + m_view.info()->allocateWidth*flc, m_view.info()->width);
		}
		return std::wstring_view(m_nodata_text);
	}
	std::shared_ptr<ConsoleReadShellContext> ConsoleReadShellContext::Create(stdex::tstring exe, stdex::tstring cmd, LPVOID , stdex::tstring ,std::wstring font,double fontsize) {
		try {
			auto r = std::make_shared<ConsoleReadShellContext>(exe, cmd, font, fontsize);
			r->m_close_watch_thread = std::thread([r]() {
				WaitForSingleObject(r->m_child_process, INFINITE);
				for (auto&& e : r->m_exit_listeners) {
					e.second(r.get());
				}
			});
			return r;
		}
		catch(std::runtime_error e){

			return std::shared_ptr<ConsoleReadShellContext>();
		}

	}
	void ConsoleReadShellContext::InputKey(WPARAM keycode,LPARAM lp) {
		if (keycode > 0x39) {
			return;
		}
		PostMessage(m_child_hwnd, WM_APP+2,keycode, lp);
	}
	void ConsoleReadShellContext::InputKey(WPARAM keycode, unsigned int count) {
		for (auto i = 0U; i < count; ++i) {
			InputKey(keycode,static_cast<LPARAM>(0));
		}
	}
	void ConsoleReadShellContext::InputChar(WPARAM charcode,LPARAM lp) {
		PostMessage(m_child_hwnd, WM_APP + 3, charcode, lp);
	}
	void ConsoleReadShellContext::InputString(std::wstring_view wstr) {
		for (auto&& e : wstr) {
			InputChar(e,0);
		}
	}
	void ConsoleReadShellContext::ConfirmString(std::wstring_view) {
		//do nothing
	}
	ShellContext::attrtext_document& ConsoleReadShellContext::GetAll() {
		return m_doc_view;
	}
	ShellContext::attrtext_document& ConsoleReadShellContext::GetView() {
		return m_doc_view;
	}
	std::wstring_view ConsoleReadShellContext::GetTitle()const {
		return m_view?m_view.info()->title:L"";
	}
	size_t ConsoleReadShellContext::GetLineCount()const {
		return m_view?m_view.info()->height:0;
	}
	size_t ConsoleReadShellContext::GetViewCount()const {
		return m_view?m_view.info()->viewSize:0;
	}
	void ConsoleReadShellContext::SetPageSize(size_t psize) {
		SendMessage(m_child_hwnd, WM_APP + 1, 1, psize);
	}
	size_t ConsoleReadShellContext::GetViewStart()const {
		return m_view ? m_view.info()->viewBeginY : 0;
	}
	ConsoleReadShellContext::attrtext_line_impl& ConsoleReadShellContext::GetCursorY(){
		if (m_view) {
			return Lines(m_view.info()->cursorY);
		}
		else {
			if (m_lines.empty()) {
				m_lines.emplace_back(this, static_cast<unsigned short>(0));
			}
			return m_lines.at(0);
		}
	}
	ShellContext::attrtext_line_iterator ConsoleReadShellContext::GetCursorYItr() {
		return ShellContext::attrtext_line_iterator(std::make_unique<attrtext_line_iterator_impl>(this,m_view.info()->cursorY));
	}
	size_t ConsoleReadShellContext::GetCursorXWStringPos()const {
		return m_view?m_view.info()->cursorX:0;
	}
	void ConsoleReadShellContext::SetViewStart(size_t s) {
		SendMessage(m_child_hwnd, WM_APP + 1, 0, s);
	}
	uintptr_t ConsoleReadShellContext::AddTextChangeListener(std::function<void(ShellContext*, std::vector<TextUpdateInfoLine>)> fn)const {
		auto k =reinterpret_cast<uintptr_t>(&fn);
		m_text_change_listeners[k] = fn;
		return k;
	}
	void ConsoleReadShellContext::RemoveTextChangeListener(uintptr_t k)const {
		m_text_change_listeners.erase(k);
	}
	uintptr_t ConsoleReadShellContext::AddLayoutChangeListener(std::function<void(ShellContext*, bool, bool)> fn)const {
		auto k = reinterpret_cast<uintptr_t>(&fn);
		m_layout_change_listeners[k] = fn;
		return k;
	}
	void ConsoleReadShellContext::RemoveLayoutChangeListener(uintptr_t k)const {
		m_layout_change_listeners.erase(k);
	}
	uintptr_t ConsoleReadShellContext::AddExitListener(std::function<void(ShellContext*)> fn)const {
		auto k = reinterpret_cast<uintptr_t>(&fn);
		m_exit_listeners[k] = fn;
		return k;
	}
	void ConsoleReadShellContext::RemoveExitListener(uintptr_t k)const {
		m_exit_listeners.erase(k);
	}
	void ConsoleReadShellContext::Lock() {
		auto id = std::this_thread::get_id();
		if (m_lock_holder != id) {
			WaitForSingleObject(m_win_mutex, INFINITE);
			m_lock_holder = id;
			++m_lock_count;
		}
		else
		{
			++m_lock_count;
		}

	}
	void ConsoleReadShellContext::Unlock() {
		if (m_lock_holder == std::this_thread::get_id()) {
			--m_lock_count;
			if (m_lock_count == 0) {
				ReleaseMutex(m_win_mutex);
			}
		}
	}
	void ConsoleReadShellContext::Resize(UINT , UINT ) {

	}
	uint32_t ConsoleReadShellContext::BackgroundColor()const {
		return 0x000000;
	}
	uint32_t ConsoleReadShellContext::FrontColor()const {
		return 0xFFFFFF;
	}
	const std::wstring& ConsoleReadShellContext::DefaultFont()const {
		return m_default_font;
	}
	double ConsoleReadShellContext::FontSize()const {
		return m_fontsize;
	}
	bool ConsoleReadShellContext::UseTerminalEchoBack()const {
		return false;
	}
	void ConsoleReadShellContext::Terminate() {
		win32::TerminateProcessTree(win32::ProcessTree(m_child_pid));
		if (m_close_watch_thread.joinable()) {
			m_close_watch_thread.join();
		}
	}
	LRESULT ConsoleReadShellContext::OnMessage(UINT msg,LPARAM lp) {
		if(WM_APP+2==msg){
			auto name = std::wstring(L"tignear.sakura.ConsoleReadShellContext.") + std::to_wstring(GetProcessId(m_child_process)) + L"." + std::to_wstring(lp);
			m_view = OpenMappingViewW(name.c_str());
		}
		else if (WM_APP + 3 == msg) {
			{
				std::lock_guard lock(m_update_watch_mutex);
				m_update = true;
			}
			m_update_watch_variable.notify_all();
		}
		return 0;
	}
}