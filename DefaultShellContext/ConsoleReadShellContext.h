#pragma once
#include <GetHwndFromPid.h>
#include <unordered_map>
#include <condition_variable>
#include <shellapi.h>
#include <ConsoleReadShellContextCommon.h>
#include <tstring.h>
#include <atomic>
#include <thread>
#include <ShellContext.h>
#include <unordered_map>
namespace tignear::sakura {
	class ConsoleReadShellContext:public ShellContext {
		static const constexpr std::hash<std::wstring_view> hash = {};
		std::condition_variable m_update_watch_variable;
		std::mutex m_update_watch_mutex;
		bool m_update_watch_closing=false;
		bool m_update=false;
		HANDLE m_child_process;

		std::thread m_update_watch_thread;
		std::thread m_close_watch_thread;
		DWORD m_child_pid;
		mutable std::unordered_map<std::uintptr_t, std::function<void(ShellContext*, std::vector<TextUpdateInfoLine>)>> m_text_change_listeners;
		mutable std::unordered_map<std::uintptr_t, std::function<void(ShellContext*, bool, bool)>> m_layout_change_listeners;
		mutable std::unordered_map<std::uintptr_t, std::function<void(ShellContext*)>> m_exit_listeners;
		MappingView m_view;
		//fields
		HANDLE m_win_mutex = NULL;
		std::atomic<std::thread::id> m_lock_holder;
		unsigned int m_lock_count = 0;
		std::wstring m_nodata_text;
		const std::wstring m_default_font;
		const double m_fontsize;
		HWND m_child_hwnd=0;
		//metods
		std::wstring_view GetStringAtLineCount(int lc);
		
		class AttributeTextImpl:public ansi::AttributeText {
			std::wstring_view text;
			const std::wstring& m_font;
		public:
			AttributeTextImpl(std::wstring_view text, const std::wstring& font):text(text), m_font(font) {
				 
			}
			std::wstring_view textW()const {
				return text;
			}
			size_t length()const {
				return text.length();
			}
			std::uint32_t textColor()const {
				return 0;
			}
			std::uint32_t backgroundColor()const {
				return 0xffffff;
			}
			bool bold()const {
				return false;
			}
			bool faint()const {
				return false;
			}
			bool italic()const {
				return false;
			}
			bool underline()const {
				return false;
			}
			bool fluktur()const {
				return false;
			}
			ansi::Blink blink()const {
				return ansi::Blink::None;
			}
			bool conceal() const {
				return false;
			}
			bool crossed_out()const {
				return false;
			}
			const std::wstring& font()const {
				return m_font;//TODO
			}
			~AttributeTextImpl() {};
		};

		class attrtext_iterator_impl :public attrtext_iterator_innner {
			std::shared_ptr<AttributeTextImpl> impl;
			bool end=false;
		public:
			void operator++() {
				end = true;
			}
			attrtext_iterator_innner* operator++(int) {
				auto temp = clone();
				end = true;
				return temp;
			}
			reference operator*() const{
				return *impl;
			}
			pointer operator->() const{
				return impl.get();
			}
			bool operator==(const attrtext_iterator_innner& iterator)const {
				auto c=dynamic_cast<const attrtext_iterator_impl*>(&iterator);
				if (c == nullptr) {
					return false;
				}
				if (end&&c->end) {
					return true;
				}
				return impl.get() == c->impl.get();
			};
			bool operator!=(const attrtext_iterator_innner& iterator)const {
				return !operator==(iterator);
			}
			attrtext_iterator_innner* clone()const {
				return new attrtext_iterator_impl(*this);
			}
			attrtext_iterator_impl(const attrtext_iterator_impl& from):impl(from.impl) {

			}
			attrtext_iterator_impl(
				std::wstring_view text,
				const std::wstring& font
			):impl(std::make_unique<AttributeTextImpl>(text,font)) {

			}
			explicit attrtext_iterator_impl() {
				end = true;
			}
			~attrtext_iterator_impl() {};
			
		};
		class attrtext_line_impl :public attrtext_line {
			size_t m_hash;
			const unsigned short line_count;
			ConsoleReadShellContext* self;
			std::shared_ptr<void> m_resource;
		public:
			attrtext_line_impl(const attrtext_line_impl& from) :m_hash(from.m_hash),line_count(from.line_count), self(from.self),m_resource(from.m_resource){}
			attrtext_line_impl(ConsoleReadShellContext* self,unsigned short lc):self(self),line_count(lc) {

			}
			attrtext_iterator begin() {
				return attrtext_iterator(std::make_unique<attrtext_iterator_impl>(self->GetStringAtLineCount(line_count), self->m_default_font));
			}
			attrtext_iterator end() {
				return attrtext_iterator(std::make_unique<attrtext_iterator_impl>());
			}
			std::shared_ptr<void>& resource()override {
				return m_resource;
			}
			bool operator==(const attrtext_line& l)const {
				auto c=dynamic_cast<const attrtext_line_impl*>(&l);
				if (!c) {
					return false;
				}
				return c->self == self && line_count == c->line_count;
			}
			bool operator!=(const attrtext_line& l)const {
				return !operator==(l);
			}
			bool isUpdated(){
				auto begin = self->m_view.info()->viewBeginY;
				if ((!self->m_view) || line_count < begin || line_count>begin+self->m_view.info()->viewSize) {
					return false;
				}
				auto && str = self->GetStringAtLineCount(line_count);
				auto temp = hash(str);
				auto r=m_hash != temp;
				m_hash = temp;	
				return r;
			}
		};
		
		std::vector<attrtext_line_impl> m_lines;
		attrtext_line_impl& Lines(size_t i) {
			if (m_view&&i > m_view.info()->height) {
				throw std::runtime_error("");
			}
			if (m_lines.size() > i) {
				return m_lines.at(i);
			}
			m_lines.reserve(i);
			for (unsigned short j =static_cast<unsigned short>( m_lines.size()); j <= i;++j) {
				m_lines.emplace_back(this,j);
			}
			return m_lines.at(i);
		}
		class attrtext_line_iterator_impl:public attrtext_line_iterator_inner {
			unsigned short line_count;
			ConsoleReadShellContext*  const self;
		public:
			attrtext_line_iterator_impl(const attrtext_line_iterator_impl& from):self(from.self),line_count(from.line_count) {

			}
			explicit attrtext_line_iterator_impl(ConsoleReadShellContext* self, unsigned short lc) :self(self), line_count(lc) {

			}
			void operator++()override{
				++line_count;
			}
			void operator--()override {
				--line_count;
			}
			attrtext_line_iterator_inner* operator++(int)override {
				auto temp = this->clone();
				++line_count;
				return temp;
			}
			attrtext_line_iterator_inner* operator--(int)override {
				auto temp = this->clone();
				--line_count;
				return temp;
			}
			reference operator*()const override {
				return self->Lines(line_count);
			}
			pointer operator->()const override {
				return &self->m_lines.at(line_count);
			}
			bool operator==(const attrtext_line_iterator_inner& iterator)const override {
				auto c = dynamic_cast<const attrtext_line_iterator_impl*>(&iterator);
				if (!c) {
					return false;
				}
				return c->self == self && c->line_count == line_count;
			}
			bool operator!=(const attrtext_line_iterator_inner& iterator)const override {
				return !operator==(iterator);
			}
			attrtext_line_iterator_inner* clone()const override{
				return new attrtext_line_iterator_impl(*this);
			}
			~attrtext_line_iterator_impl() {};
		};
		class attrtext_document_impl_view:public attrtext_document {
			ConsoleReadShellContext*  const self;
		public:
			explicit attrtext_document_impl_view(ConsoleReadShellContext* self):self(self) {

			}
			attrtext_line_iterator begin()override {
				return attrtext_line_iterator(std::make_unique<attrtext_line_iterator_impl>(self, static_cast<unsigned short>(self->GetViewStart())));
			}
			attrtext_line_iterator end()override {
				if (self->m_view) {
					return attrtext_line_iterator(std::make_unique<attrtext_line_iterator_impl>(self, static_cast<unsigned short>(self->GetViewStart() + self->GetViewCount())));
				}
				return attrtext_line_iterator(std::make_unique<attrtext_line_iterator_impl>(self, static_cast<unsigned short>(1)));
			}

		};
		attrtext_document_impl_view m_doc_view{ this };
		class TextUpdateInfoLineImpl :public TextUpdateInfoLineInner {
			TextUpdateStatus sta;
			size_t index;
			std::vector<attrtext_line_impl>& m_lines;
		public:
			 TextUpdateInfoLineImpl(TextUpdateStatus sta,std::vector<attrtext_line_impl>& lines,size_t index) noexcept :sta(sta),m_lines(lines),index(index) {

			}
			TextUpdateStatus status()const noexcept override  {
				return sta;
			}
			attrtext_line& line()noexcept override {
				return m_lines[index];
			}
			TextUpdateInfoLineInner* clone()const noexcept override {
				return new TextUpdateInfoLineImpl(sta,m_lines,index);
			}
		};
	public:
		ConsoleReadShellContext(stdex::tstring exe,stdex::tstring cmd,std::wstring default_font,double fontsize) :
			m_default_font(default_font),
			m_fontsize(fontsize),
			m_child_process([this,&exe,&cmd]() {
				SHELLEXECUTEINFO sei{};
				sei.cbSize = sizeof(sei);
				sei.nShow = SW_HIDE;
				sei.fMask = SEE_MASK_NOCLOSEPROCESS| SEE_MASK_NOASYNC;
				sei.lpFile = exe.c_str();
				auto ccmd = (cmd + _T(" ") + stdex::to_tstring(GetCurrentProcessId()) + _T(" ") + stdex::to_tstring(reinterpret_cast<uintptr_t>(this)));
				sei.lpParameters = ccmd.c_str();
				ShellExecuteEx(&sei);
				return sei.hProcess;
			}()),
			m_child_pid(GetProcessId(m_child_process)),
			m_win_mutex([this]() {
				using namespace stdex;
				tstring mutex_name = _T("tignear.sakura.ConsoleReadShellContext.mutex.");
				auto str_process_id = to_tstring(GetCurrentProcessId());
				auto str_process_cnt = to_tstring(reinterpret_cast<uintptr_t>(this));
				mutex_name += str_process_id;
				mutex_name += _T(".");
				mutex_name += str_process_cnt;
				return CreateMutexEx(NULL, mutex_name.c_str(), NULL, MUTEX_ALL_ACCESS);
		}()),
			m_update_watch_thread([this]() {
			MappingInfo prev_info{};
			while (true) {
				{
					std::unique_lock lock(m_update_watch_mutex);
					m_update_watch_variable.wait(lock, [this]() {return m_update || m_update_watch_closing; });
				}
				if (m_update_watch_closing) {
					return;
				}
				if (!m_view) {
					m_update = false;
					continue;
				}
				if (m_update) {
					//text update
					auto s = m_view.info()->viewBeginY;
					auto e = s + m_view.info()->viewSize;
					std::vector<TextUpdateInfoLine> up;
					for (unsigned short i = s; i < e; ++i) {
						while(i >= m_lines.size()) {
							m_lines.emplace_back(this,static_cast<unsigned short>(m_lines.size()));
						}
						if (m_lines[i].isUpdated()) {
							up.emplace_back(std::make_unique<TextUpdateInfoLineImpl>(ShellContext::TextUpdateStatus::MODIFY, m_lines,i));
						}
					}
					if (!up.empty()) {
						for (auto&& elem : m_text_change_listeners) {
							elem.second(this, up);
						}
					}
					//layout update					
					if (memcmp(&prev_info, m_view.info(), sizeof(prev_info))) {
						if (prev_info.width != m_view.info()->width) {
							m_nodata_text.assign(m_view.info()->width, L' ');
						}
						for (auto&& l : m_layout_change_listeners) {
							l.second(this, true, true);
						}
						memcpy(&prev_info, m_view.info(), sizeof(prev_info));
					}

					{
						std::lock_guard lock(m_update_watch_mutex);
						m_update = false;

					}
				}
			}
			})
		{
			WaitForInputIdle(m_child_process,INFINITE);
			m_child_hwnd = win32::GetHwndFromProcess(m_child_pid);
		}
		static std::shared_ptr<ConsoleReadShellContext> Create(stdex::tstring exe, stdex::tstring cmd,LPVOID env,stdex::tstring cdir,std::wstring font,double fontsize);
		void InputKey(WPARAM keycode,LPARAM)override;//no lock call
		void InputKey(WPARAM keycode, unsigned int count)override;//no lock call
		void InputChar(WPARAM charcode,LPARAM)override;//no lock call
		void InputString(std::wstring_view wstr)override;//no lock call
		void ConfirmString(std::wstring_view)override;//no lock call
		attrtext_document& GetAll()override;//lock call
		attrtext_document& GetView()override;//lock call
		std::wstring_view GetTitle()const override;//no lock call
		size_t GetLineCount()const override;//no lock call
		size_t GetViewCount()const override;//no lock call
		void SetPageSize(size_t count)override;//no lock call
		size_t GetViewStart()const override;//no lock call
		attrtext_line_impl& GetCursorY()override;
		size_t GetCursorXWStringPos()const override;//no lock call.wstring_view position
		void SetViewStart(size_t)override;//no lock call
		uintptr_t AddTextChangeListener(std::function<void(ShellContext*, std::vector<TextUpdateInfoLine>)>)const override;//no lock call
		void RemoveTextChangeListener(uintptr_t)const override;//no lock call
		uintptr_t AddLayoutChangeListener(std::function<void(ShellContext*, bool, bool)>)const override;//no lock call
		void RemoveLayoutChangeListener(uintptr_t)const override;//no lock call
		uintptr_t AddExitListener(std::function<void(ShellContext*)>)const override;//no lock call
		void RemoveExitListener(uintptr_t)const override;//no lock call
		void Lock()override;//no lock call
		void Unlock()override;//lock call
		void Resize(UINT w, UINT h)override;//no lock call
		const std::wstring& DefaultFont()const override;//no lock call
		double FontSize()const override;//no lock call
		bool UseTerminalEchoBack()const override;//no lock call
		void Terminate()override;
		LRESULT OnMessage(UINT,LPARAM lparam)override;
		~ConsoleReadShellContext() {
			CloseHandle(m_win_mutex);
			{
				std::lock_guard lock(m_update_watch_mutex);
				m_update_watch_closing = true;
			}
			m_update_watch_variable.notify_all();
			m_update_watch_thread.join();
		};//no lock call.bat not require lock.
	};
}