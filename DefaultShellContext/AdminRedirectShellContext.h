#pragma once
#pragma once
#include <memory>
#include "tstring.h"
#include "BasicShellContext.h"
namespace tignear::sakura {
	class AdminRedirectShellContext :public BasicShellContext {

	public:
		struct Options {
			LPVOID environment;
			LPCTSTR current_directory;
		};
	private:
		static bool CreateShell(std::shared_ptr<AdminRedirectShellContext> s,stdex::tstring,stdex::tstring, const Options& opt);
		static bool WorkerStart(std::shared_ptr<AdminRedirectShellContext>);
		HANDLE m_childProcess=0;
		std::thread m_thread;
	public:
		static std::shared_ptr<AdminRedirectShellContext> Create(
			HINSTANCE,
			stdex::tstring,
			std::shared_ptr<iocp::IOCPMgr>,
			unsigned int codepage,
			std::unordered_map<unsigned int, uint32_t>,
			std::unordered_map<unsigned int, uint32_t>,
			bool use_terminal_echoback,
			std::vector<std::wstring> fontmap,
			double fontsize,
			const Options& opt
		);
		AdminRedirectShellContext(
			std::shared_ptr<iocp::IOCPMgr> iocpmgr,
			unsigned int codepage,
			const ansi::ColorTable& c_sys,
			const ansi::ColorTable& c_256,
			bool use_terminal_echoback,
			std::vector<std::wstring> fontmap,
			double fontsize,
			Attribute& def
		):BasicShellContext(
				iocpmgr,
				codepage,
				c_sys,
				c_256,
				use_terminal_echoback,
				fontmap,
				fontsize,
				def
			) {}
		LRESULT OnMessage(LPARAM)override;
		void Terminate()override;
		~AdminRedirectShellContext();
	};
}