#pragma once
#include <memory>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include "IOCPMgr.h"
#include "BasicShellContextDocument.h"
#include "ShellContext.h"
#include "ansi/AttributeText.h"
#include "ansi/AnsiParser.h"
#include "tstring.h"
namespace tignear::sakura {
	class BasicShellContext:public ShellContext {
	public:

	protected:

		//ansi parser call backs

		friend BasicShellContext& ansi::parseW<BasicShellContext>(std::wstring_view,BasicShellContext&);
		void FindCSI(std::wstring_view);
		void FindString(std::wstring_view);
		void FindOSC(std::wstring_view);
		void FindBS();
		void FindFF();		
		void ParseColor(std::wstring_view sv);
		//other class members
		static bool IOWorkerStart(std::shared_ptr<BasicShellContext>);
		static bool OutputWorker(std::shared_ptr<BasicShellContext>);
		static bool OutputWorkerHelper(DWORD cnt,std::shared_ptr<BasicShellContext>);
		void AddString(std::wstring_view);
		constexpr static unsigned int BUFFER_SIZE = 4096;
		const unsigned int m_codepage;
		static std::atomic_uintmax_t m_process_count;
		std::shared_ptr<iocp::IOCPMgr> m_iocpmgr;
		HANDLE m_childProcess;
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
			const ColorTable& c_sys,
			const ColorTable& c_256,
			bool use_terminal_echoback,
			std::vector<std::wstring> fontmap,
			double fontsize,
			Attribute& def) :
			m_iocpmgr(iocpmgr),
			m_outbuf(BUFFER_SIZE, '\0'),
			m_codepage(codepage),
			m_document(BasicShellContextDocument(c_sys, c_256, fontmap, def, std::bind(&BasicShellContext::NotifyLayoutChange, std::ref(*this), std::placeholders::_1, std::placeholders::_2), std::bind(&BasicShellContext::NotifyTextChange, std::ref(*this), std::placeholders::_1))),
			m_use_terminal_echoback(use_terminal_echoback),
			m_fontmap(fontmap),
			m_fontsize(fontsize),
			m_hwnd(0)
		{
		}
		~BasicShellContext() {
			CloseHandle(m_out_pipe);
			CloseHandle(m_childProcess);
		}

		void InputChar(WPARAM c) override;
		void InputKey(WPARAM keycode) override;
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
		void Set256Color(const std::unordered_map<unsigned int, uint32_t>&)override;
		void Set256Color(const std::unordered_map<unsigned int, uint32_t>&&)override;
		void SetSystemColor(const std::unordered_map<unsigned int, uint32_t>&) override;
		void SetSystemColor(const std::unordered_map<unsigned int, uint32_t>&&)override;
		BasicShellContextLineText& GetCursorY()const override;
		size_t GetCursorX()const override;
		void Resize(UINT w, UINT h)override;
		attrtext_document& GetAll() override;
		attrtext_document& GetView() override;
		double FontSize()const override;
		bool UseTerminalEchoBack()const override;
		const std::wstring& DefaultFont()const override;
		void Lock()override;
		void Unlock()override;
};

}