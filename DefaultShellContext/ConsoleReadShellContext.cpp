#include "stdafx.h"
#include <ProcessTree.h>
#include <shellapi.h>
#include <algorithm>
#include "ConsoleReadShellContext.h"
namespace tignear::sakura::conread {
	using namespace stdex;
	void ConsoleReadShellContext::Worker() {
		using namespace exe2context;
		using namespace exe2context;
		while (true) {
			auto future = m_message.message();
			auto eve = future.get();
			if (eve.msg == 0) {
				return;
			}
			switch (eve.msg)
			{
			case UPDATE_CARET:
				for (auto&& fn : m_layout_change_listeners) {
					fn.second(this,true,true);
				}
				break;
			case UPDATE_SIMPLE:
			{
				auto y = eve.lp & 0xFFFF;
				auto v= this->GetStringAtLineCount((int)y);
				Lines(y).update();
				for (auto fn : m_text_change_listeners) {
					fn.second(this, { TextUpdateInfoLine(std::make_unique<TextUpdateInfoLineImpl>(TextUpdateStatus::MODIFY,m_lines,y)) });
				}
				break;

			}
			case UPDATE_REGION:
			{
				auto ys = eve.lp & 0xFFFF;
				auto ye = eve.lp>>32 & 0xFFFF;
				std::vector<TextUpdateInfoLine> info;
				info.reserve(ye - ys+1);
				for (auto y = ys; y <= ye; ++y) {
					Lines(y).update();
					info.emplace_back(std::make_unique<TextUpdateInfoLineImpl>(TextUpdateStatus::MODIFY, m_lines, y));
				}
				for (auto fn : m_text_change_listeners) {
					fn.second(this, info);
				}
				break;
			}
			default:
				continue;
			}
		}
	}
	std::wstring_view ConsoleReadShellContext::GetStringAtLineCount(int lc)const {
		if (!m_view) {
			return std::wstring_view();
		}
		auto flc = lc - m_view.info()->viewBeginY;
		if (flc>=0&&flc<m_view.info()->viewSize) {
			auto f = m_view.info()->allocateWidth*flc;
			auto sv = std::wstring_view(m_view.buf() + f, m_view.info()->width);

			size_t rmc = sv.find_last_not_of(L'\0');
			if (std::wstring_view::npos == rmc) {
				return std::wstring_view();
			}
			sv.remove_suffix(sv.size() - rmc - 1);


			if (std::wstring_view::npos == sv.find_first_not_of(' ')) {
				return std::wstring_view();
			}

			return sv;
		}
		return std::wstring_view();
	}
	stdex::array_view<WORD> ConsoleReadShellContext::GetAttributesAtLineCount(int lc)const {
		if (!m_view) {
			return stdex::array_view<WORD>();
		}
		auto flc = lc - m_view.info()->viewBeginY;
		if (flc >= 0 && flc < m_view.info()->viewSize) {
			return stdex::array_view<WORD>(m_view.attribute() + m_view.info()->allocateWidth*flc,static_cast<size_t>(m_view.info()->width));
		}
		return stdex::array_view<WORD>();

	}
	//std::vector<WORD> 
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
		PostMessage(m_child_hwnd, context2exe::INPUT_KEY,keycode, lp);
	}
	void ConsoleReadShellContext::InputKey(WPARAM keycode, unsigned int count) {
		for (auto i = 0U; i < count; ++i) {
			InputKey(keycode,static_cast<LPARAM>(0));
		}
	}
	void ConsoleReadShellContext::InputChar(WPARAM charcode,LPARAM lp) {
		PostMessage(m_child_hwnd, context2exe::INPUT_CHAR, charcode, lp);
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
		if (!m_view) {
			return 0;
		}
		size_t beg = m_view.info()->viewBeginY;

		return std::clamp(m_pagesize, beg, beg + m_view.info()->viewSize);
	}
	void ConsoleReadShellContext::SetPageSize(size_t psize) {
		m_pagesize = psize;
		//SendMessage(m_child_hwnd, context2exe::UPDATE_SIZE, context2exe::update_size::SET_PAGESIZE, psize);
	}
	size_t ConsoleReadShellContext::GetViewStart()const {
		if (!m_view) {
			return 0;
		}
		size_t beg = m_view.info()->viewBeginY;
		return std::clamp(m_view_start,beg,beg+m_view.info()->viewSize);
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
		if (!m_view) {
			return 0;
		}
		auto y =  m_view.info()->cursorY;
		if (m_lines.size() > y) {
			const auto& e = m_lines.at(y);
			size_t ret=0;
			for (auto&& e2 : e.attr_text()) {
				ret += e2.length();
			}
			return ret;
		}
		else {
			return 0;
		}
	}
	void ConsoleReadShellContext::SetViewStart(size_t s) {
		m_view_start = s;
		//SendMessage(m_child_hwnd, context2exe::UPDATE_SIZE, context2exe::update_size::SET_BEGIN, s);
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
		m_message.postMessage({});
		if (m_close_watch_thread.joinable()) {
			m_close_watch_thread.join();
		}
		if (m_worker_thread.joinable()) {
			m_worker_thread.join();
		}
	}
	LRESULT ConsoleReadShellContext::OnMessage(UINT msg,LPARAM lp) {
		using namespace exe2context;

		if (!(msg >= static_cast<UINT>(CREATE_VIEW) && msg <= static_cast<UINT>(END_OF_MESSAGE))) {
			return FALSE;
		}
		switch (msg) {
		case CREATE_VIEW:
		{
			auto name = std::wstring(L"tignear.sakura.ConsoleReadShellContext.") + std::to_wstring(GetProcessId(m_child_process)) + L"." + std::to_wstring(lp);
			m_view = OpenMappingViewW(name.c_str());
				m_lines.reserve(m_view.info()->height);
				std::vector<TextUpdateInfoLine> info;
				for (unsigned short j = static_cast<unsigned short>(m_lines.size()); j <= m_view.info()->height; ++j) {
					m_lines.emplace_back(this, j);
					info.emplace_back(std::make_unique<TextUpdateInfoLineImpl>(TextUpdateStatus::NEW, m_lines, j));
				}
				if (!info.empty()) {
					for (auto&& l : m_text_change_listeners) {
						l.second(this, info);
					}
				}
			
			break;
		}
		default:
			m_message.postMessage({msg,lp});
		}

		return TRUE;
	}
}