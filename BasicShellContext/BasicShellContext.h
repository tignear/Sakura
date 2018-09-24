#pragma once
#include <memory>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <IOCPMgr.h>
#include <ShellContext.h>
#include <ansi/AttributeText.h>
#include <ansi/AnsiParser.h>
#include <tstring.h>
#include "BasicShellContextDocument.h"

namespace tignear::sakura {
	class BasicShellContext:public ShellContext {
	public:

	protected:

		//ansi parser call backs

		friend BasicShellContext& ansi::parseA<BasicShellContext>(std::string_view,BasicShellContext&);
		void FindCSI(std::string_view);
		void FindString(std::string_view);
		void FindOSC(std::string_view);
		void FindBS();
		void FindFF();
		void FindCR();
		void FindLF();
		void ParseColor(std::string_view sv);
		//other class members
		static bool OutputWorker(std::shared_ptr<BasicShellContext>);
		static bool OutputWorkerHelper(DWORD cnt,std::shared_ptr<BasicShellContext>);
		void AddString(std::string_view);
		constexpr static unsigned int BUFFER_SIZE = 4096;
		const unsigned int m_codepage;
		const uint32_t m_bg_color;
		const uint32_t m_fr_color;
		static std::atomic_uintmax_t m_process_count;
		std::shared_ptr<iocp::IOCPMgr> m_iocpmgr;
		HANDLE m_out_pipe;
		std::atomic<HWND> m_hwnd;
		std::wstring m_title;
		mutable std::recursive_mutex m_lock;
		mutable std::unordered_map<std::uintptr_t, std::function<void(ShellContext*, std::vector<TextUpdateInfoLine>)>> m_text_change_listeners;
		mutable std::unordered_map<std::uintptr_t, std::function<void(ShellContext*, bool, bool)>> m_layout_change_listeners;
		mutable std::unordered_map<std::uintptr_t, std::function<void(ShellContext*)>> m_exit_listeners;
		BasicShellContextDocument m_document;
		bool m_use_terminal_echoback;
		std::vector<std::wstring> m_fontmap;
		double m_fontsize;


		void NotifyLayoutChange(bool,bool);
		void NotifyTextChange(std::vector<TextUpdateInfoLine>);
		void NotifyExit();
		//out pipe temp
		std::string m_outbuf;

	public:

		BasicShellContext(
			std::shared_ptr<iocp::IOCPMgr> iocpmgr,
			unsigned int codepage,
			const ansi::ColorTable& c_sys,
			const ansi::ColorTable& c_256,
			bool use_terminal_echoback,
			std::vector<std::wstring> fontmap,
			double fontsize,
			Attribute& def) :
			m_iocpmgr(iocpmgr),
			m_outbuf(BUFFER_SIZE, '\0'),
			m_codepage(codepage),
			m_fr_color(SolveColor(def.frColor,c_sys,c_256)),
			m_bg_color(SolveColor(def.bgColor, c_sys, c_256)),
			m_document(BasicShellContextDocument(c_sys, c_256, fontmap, def, std::bind(&BasicShellContext::NotifyLayoutChange, std::ref(*this), std::placeholders::_1, std::placeholders::_2), std::bind(&BasicShellContext::NotifyTextChange, std::ref(*this), std::placeholders::_1))),
			m_use_terminal_echoback(use_terminal_echoback),
			m_fontmap(fontmap),
			m_fontsize(fontsize),
			m_hwnd(0),
			m_out_pipe(0)
		{
		}
		virtual ~BasicShellContext() {
			if (m_out_pipe) {
				CloseHandle(m_out_pipe);
				m_out_pipe = NULL;
			}
		}

		void InputChar(WPARAM c,LPARAM lp) override;
		void InputKey(WPARAM keycode,LPARAM lp) override;
		void InputKey(WPARAM keycode, unsigned int count) override;
		void InputString(std::wstring_view) override;
		void ConfirmString(std::wstring_view) override;
		std::wstring_view GetTitle()const override;
		size_t GetViewCount()const override;
		size_t GetLineCount()const override;
		void SetPageSize(size_t count)override;
		size_t GetViewStart()const override;
		void SetViewStart(size_t)override;
		uintptr_t AddTextChangeListener(std::function<void(ShellContext*,std::vector<TextUpdateInfoLine>)>)const override;
		void RemoveTextChangeListener(uintptr_t)const override;
		uintptr_t AddLayoutChangeListener(std::function<void(ShellContext*,bool,bool)>)const override;
		void RemoveLayoutChangeListener(uintptr_t)const override;
		uintptr_t AddExitListener(std::function<void(ShellContext*)>)const override;
		void RemoveExitListener(uintptr_t)const override;
		BasicShellContextLineText& GetCursorY()override;
		size_t GetCursorXWStringPos()const override;
		void Resize(UINT w, UINT h)override;
		attrtext_document& GetAll() override;
		attrtext_document& GetView() override;
		double FontSize()const override;
		bool UseTerminalEchoBack()const override;
		uint32_t BackgroundColor()const override;
		uint32_t FrontColor()const override;
		const std::wstring& DefaultFont()const override;
		void Lock()override;
		void Unlock()override;
		LRESULT OnMessage(UINT,LPARAM)override;
};

}