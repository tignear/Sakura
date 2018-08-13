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
		bool m_close;
		bool IOWorkerStart(std::shared_ptr<BasicShellContext>);
		void OutputWorker(std::shared_ptr<BasicShellContext>);
		void AddString(tignear::stdex::tstring);
		tignear::stdex::tstring buffer;
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
		bool Init(stdex::tstring);
	};

}