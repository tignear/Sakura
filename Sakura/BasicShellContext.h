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
			bool crossed_out;
			unsigned int font;//0-9
			bool reverse;
		};
		static ansi::AttributeText CreateAttrText(icu::UnicodeString& str,const Attribute& attr);
		static ansi::AttributeText CreateAttrText(icu::UnicodeString&& str,const Attribute& attr);
		//static bool EqAttr(const ansi::AttributeText&,const Attribute&);
		//ansi parser call backs
		friend BasicShellContext& ansi::parseW<BasicShellContext>(std::wstring_view,BasicShellContext&);
		void FindCSI(std::wstring_view);
		void FindString(std::wstring_view);
		void FindOSC(std::wstring_view);
		void FindBS();
		void FindFF();
		//ansi parser work helpers
		void RemoveRows(std::list<std::list<ansi::AttributeText>>::size_type count);
		void RemoveRowsR(std::list<std::list<ansi::AttributeText>>::size_type count);
		void RemoveColumns();
		void RemoveColumnsR();
		void RemoveCursorBefore();
		void RemoveCursorAfter();
		void ParseColor(std::wstring_view);
		void InsertCursorPos(const std::wstring&);
		int32_t CurosorLineLength();
		//other class members
		static bool IOWorkerStart(std::shared_ptr<BasicShellContext>);
		static bool OutputWorker(std::shared_ptr<BasicShellContext>);
		static bool OutputWorkerHelper(DWORD cnt,std::shared_ptr<BasicShellContext>);
		void AddString(std::wstring_view);
		void MoveCurosorYUp(std::list<std::list<ansi::AttributeText>>::size_type count);
		void MoveCurosorYDown(std::list<std::list<ansi::AttributeText>>::size_type count);
		void NotifyUpdateText();
		void NotifyUpdateString();
		constexpr static unsigned int BUFFER_SIZE = 4096;
		const unsigned int m_codepage;
		static std::atomic_uintmax_t m_process_count;
		std::shared_ptr<iocp::IOCPMgr> m_iocpmgr;
		HANDLE m_childProcess;
		HANDLE m_out_pipe;
		HWND m_hwnd;
		std::list<std::list<ansi::AttributeText>> m_text;
		std::list<std::list<ansi::AttributeText>>::iterator m_viewstartY_itr;
		std::list<std::list<ansi::AttributeText>>::iterator m_cursorY_itr;
		int32_t m_cursorX;
		std::list<std::list<ansi::AttributeText>>::size_type m_viewline_count;//âÊñ Ç…âfÇÈçsÇÃêî
		mutable std::unordered_map<std::uintptr_t,std::function<void(ShellContext*)>> m_text_change_listeners;
		std::wstring m_title;
		Attribute m_current_attr;
		Attribute m_def_attr;
		bool m_attr_updated;
		std::unordered_map<unsigned int, std::uint32_t> m_system_color_table;
		std::unordered_map<unsigned int, std::uint32_t> m_256_color_table;
		bool Init(stdex::tstring);
		//out pipe temp
		std::string m_outbuf;
	public:
		BasicShellContext(std::shared_ptr<iocp::IOCPMgr> iocpmgr,unsigned int codepage):
			m_iocpmgr(iocpmgr),
			m_outbuf(BUFFER_SIZE, '\0'),
			m_text{ {ansi::AttributeText(icu::UnicodeString())} },
			m_current_attr{ 0,0xFFB6C1,false,false,false,false,false,ansi::Blink::None,false,false,0,false },
			m_def_attr{ 0,0xFFB6C1,false,false,false,false,false,ansi::Blink::None,false,false,0,false },
			m_codepage(codepage)
		{
			m_viewstartY_itr = m_text.begin();
			m_cursorY_itr = m_text.begin();
			m_viewline_count = 20;
			m_cursorX = 0;
		}
		~BasicShellContext() {
			CloseHandle(m_out_pipe);
			CloseHandle(m_childProcess);
		}
		static std::shared_ptr<BasicShellContext> Create(stdex::tstring,std::shared_ptr<iocp::IOCPMgr>,unsigned int codepage, std::unordered_map<unsigned int, uint32_t>, std::unordered_map<unsigned int, uint32_t>);
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
		void Set256Color(const std::unordered_map<unsigned int, uint32_t>&)override;
		void Set256Color(const std::unordered_map<unsigned int, uint32_t>&&)override;
		void SetSystemColor(const std::unordered_map<unsigned int, uint32_t>&) override;
		void SetSystemColor(const std::unordered_map<unsigned int, uint32_t>&&)override;
	};

}