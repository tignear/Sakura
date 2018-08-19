#pragma once
#include <memory>
#include <unordered_map>
#include <atomic>
#include "IOCPMgr.h"
#include "ShellContext.h"
#include "ansi/AttributeText.h"
#include "ansi/AnsiParser.h"
#include "tstring.h"
namespace tignear::sakura {
	class BasicShellContext:public ShellContext {
	private:
		struct Attribute {
			std::uint32_t textColor;
			std::uint32_t backgroundColor;
			bool bold;
			bool faint;
			bool italic;
			bool underline;
			bool fluktur;
			ansi::Blink blink;
			bool conceal;//âBÇ∑
			unsigned int font;//0-9
			bool reverse;
		};
		static ansi::AttributeText CreateAttrText(std::wstring& str,const Attribute& attr);
		static ansi::AttributeText CreateAttrText(std::wstring&& str, const Attribute& attr);

		//ansi parser call backs
		friend BasicShellContext& ansi::parse<BasicShellContext>(std::wstring_view,BasicShellContext&);
		void FindCSI(std::wstring_view);
		void FindString(std::wstring_view);
		void FindOSC(std::wstring_view);
		//ansi parser work helpers
		void RemoveRows(std::list<std::list<ansi::AttributeText>>::size_type count);
		void RemoveRowsR(std::list<std::list<ansi::AttributeText>>::size_type count);
		void RemoveColumns(std::wstring::size_type count);
		void RemoveColumnsR(std::wstring::size_type count);
		void RemoveCursorBefore();
		void RemoveCursorAfter();
		void ParseColor(std::wstring_view);
		void InsertCursorPos(std::wstring&&);
		std::wstring::size_type CurosorLineLength();
		//class members
		constexpr static unsigned int BUFFER_SIZE = 4096;
		static std::atomic_uintmax_t m_process_count;
		std::shared_ptr<iocp::IOCPMgr> m_iocpmgr;
		HANDLE m_childProcess;
		HANDLE m_out_pipe;
		HANDLE m_in_pipe;
		HWND m_hwnd;
		std::list<std::list<ansi::AttributeText>> m_text;
		std::list<std::list<ansi::AttributeText>>::iterator m_viewstartY_itr;
		std::list<std::list<ansi::AttributeText>>::iterator m_cursorY_itr;
		std::wstring::size_type m_cursorX;
		std::list<std::list<ansi::AttributeText>>::size_type m_viewline_count;//âÊñ Ç…âfÇÈçsÇÃêî
		mutable std::unordered_map<std::uintptr_t,std::function<void(ShellContext*)>> m_text_change_listeners;
		std::wstring m_title;
		static bool IOWorkerStart(std::shared_ptr<BasicShellContext>);
		static bool OutputWorker(std::shared_ptr<BasicShellContext>);
		static bool OutputWorkerHelper(DWORD cnt,std::shared_ptr<BasicShellContext>);
		void AddString(std::wstring_view);
		void MoveCurosorYUp(std::list<std::list<ansi::AttributeText>>::size_type count);
		void MoveCurosorYDown(std::list<std::list<ansi::AttributeText>>::size_type count);
		Attribute m_current_attr;
		Attribute m_def_attr;
		const std::unordered_map<unsigned int, std::uint32_t> m_system_color_table;
		const std::unordered_map<unsigned int, std::uint32_t> m_256_color_table;
		bool Init(stdex::tstring);
		//out pipe temp
		std::string m_outbuf;
	public:
		BasicShellContext(std::shared_ptr<iocp::IOCPMgr> iocpmgr):
			m_iocpmgr(iocpmgr),
			m_outbuf(BUFFER_SIZE, '\0'),
			m_text{ {ansi::AttributeText(L"")} },
			m_current_attr(),
			m_def_attr()
		{
			m_viewstartY_itr = m_text.begin();
			m_cursorY_itr = m_text.begin();
			m_viewline_count = 20;
			m_cursorX = 0;
		}
		~BasicShellContext() {
			CloseHandle(m_out_pipe);
			CloseHandle(m_in_pipe);
			CloseHandle(m_childProcess);
		}
		static std::shared_ptr<BasicShellContext> Create(stdex::tstring,std::shared_ptr<iocp::IOCPMgr>);
		void InputChar(WPARAM c) override;
		void InputKey(WPARAM keycode) override;
		void InputKey(WPARAM keycode, unsigned int count) override;
		void InputString(std::wstring_view) override;
		void ConfirmString(std::wstring_view) override;
		std::list<std::list<ansi::AttributeText>>::const_iterator GetViewTextBegin()const override;
		std::list<std::list<ansi::AttributeText>>::const_iterator GetViewTextEnd()const override;
		std::wstring_view GetTitle()const override;
		std::wstring::size_type GetViewLineCount()const override;
		void SetViewLineCount(std::wstring::size_type count)override;
		uintptr_t AddTextChangeListener(std::function<void(ShellContext*)>)const override;
		void RemoveTextChangeListener(uintptr_t)const override;
		uintptr_t AddCursorChangeListener(std::function<void(ShellContext*)>)const override;
		void RemoveCursorChangeListener(uintptr_t)const override;

	};

}