#pragma once
#include <memory>
#include <unordered_map>
#include <atomic>
#include "IOCPMgr.h"
#include "ShellContext.h"
#include "BasicAttributeText.h"
#include "tstring.h"
namespace tignear::sakura {
	class BasicShellContext:public ShellContext {
	private:
		constexpr static unsigned int BUFFER_SIZE = 4096;
		static std::atomic_uintmax_t m_process_count;
		std::shared_ptr<iocp::IOCPMgr> m_iocpmgr;
		HANDLE m_childProcess;
		HANDLE m_out_pipe;
		HANDLE m_in_pipe;
		HWND m_hwnd;
		bool m_close;
		unsigned int m_cursorX, m_cursorY;
		mutable std::unordered_map<std::uintptr_t,std::function<void(ShellContext*)>> m_text_change_listeners;
		static bool IOWorkerStart(std::shared_ptr<BasicShellContext>);
		static bool OutputWorker(std::shared_ptr<BasicShellContext>);
		static bool OutputWorkerHelper(DWORD cnt,std::shared_ptr<BasicShellContext>);
		void AddString(std::wstring);
		std::wstring m_buffer;
		std::list<std::list<AttributeText*>> m_text;
		bool Init(stdex::tstring);
		//out pipe temp
		std::string m_outbuf;
	public:
		BasicShellContext(std::shared_ptr<iocp::IOCPMgr> iocpmgr):
			m_close(false), 
			m_iocpmgr(iocpmgr),
			m_outbuf(BUFFER_SIZE, '\0'),
			m_text({ { new BasicAttributeText(m_buffer,0,0) } }),
			m_cursorX(0),
			m_cursorY(0){}
		~BasicShellContext() {
			CloseHandle(m_out_pipe);
			CloseHandle(m_in_pipe);
			CloseHandle(m_childProcess);
		}
		static std::shared_ptr<BasicShellContext> Create(stdex::tstring,std::shared_ptr<iocp::IOCPMgr>);
		void InputChar(WPARAM c) override;
		void InputKey(WPARAM keycode) override;
		void InputString(std::wstring_view) override;
		const std::list<std::list<AttributeText*>>& GetText()const override;
		std::wstring_view GetString()const override;
		unsigned int GetCursorX()const override;
		unsigned int GetCursorY()const override;
		uintptr_t AddTextChangeListener(std::function<void(ShellContext*)>)const override;
		void RemoveTextChangeListener(uintptr_t)const override;
		uintptr_t AddCursorChangeListener(std::function<void(ShellContext*)>)const override;
		void RemoveCursorChangeListener(uintptr_t)const override;
	};

}