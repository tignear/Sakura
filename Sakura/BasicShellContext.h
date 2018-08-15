#pragma once
#include <memory>
#include <atomic>
#include "IOCPMgr.h"
#include "ShellContext.h"
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
		static bool IOWorkerStart(std::shared_ptr<BasicShellContext>);
		static bool OutputWorker(std::shared_ptr<BasicShellContext>);
		static bool OutputWorkerHelper(DWORD cnt,std::shared_ptr<BasicShellContext>);
		void AddString(std::wstring);
		std::list<LineText> m_text;
		bool Init(stdex::tstring);
		//out pipe temp
		char m_outbuf[BUFFER_SIZE]{};
	public:
		BasicShellContext(std::shared_ptr<iocp::IOCPMgr> iocpmgr) :m_close(false),m_iocpmgr(iocpmgr) {}
		~BasicShellContext() {
			CloseHandle(m_out_pipe);
			CloseHandle(m_in_pipe);
			CloseHandle(m_childProcess);
		}
		static std::shared_ptr<BasicShellContext> Create(stdex::tstring,std::shared_ptr<iocp::IOCPMgr>);
		void InputChar(WPARAM c) override;
		void InputKey(WPARAM keycode) override;
		void InputString(std::wstring) override;
		std::list<LineText> GetText()override;
		
		unsigned int GetCursorX() override;
		unsigned int GetCursorY() override;
	};

}