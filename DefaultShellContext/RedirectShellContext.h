#pragma once
#include <memory>
#include <tstring.h>
#include <BasicShellContext.h>
namespace tignear::sakura {
	class RedirectShellContext :public BasicShellContext {

	public:	
		struct Options {
			LPVOID environment;
			LPCTSTR current_directory;
			bool use_size;
			DWORD width;
			DWORD height;
			bool use_count_chars;
			DWORD x_count_chars;
			DWORD y_count_chars;
		};
	private:
		static bool CreateShell(std::shared_ptr<RedirectShellContext> s,stdex::tstring, const Options& opt);
		HANDLE m_childProcess;
		std::thread m_thread;
	public:
		static std::shared_ptr<RedirectShellContext> Create(
			stdex::tstring,
			std::shared_ptr<iocp::IOCPMgr>,
			unsigned char ambiguous_size,
			unsigned int codepage,
			std::unordered_map<unsigned int, uint32_t>,
			std::unordered_map<unsigned int, uint32_t>,
			bool use_terminal_echoback,
			std::vector<std::wstring> fontmap,
			double fontsize,
			const Options& opt
		);
		RedirectShellContext(
			std::shared_ptr<iocp::IOCPMgr> iocpmgr,
			unsigned char ambiguous_size,
			unsigned int codepage,
			const ansi::ColorTable& c_sys,
			const ansi::ColorTable& c_256,
			bool use_terminal_echoback,
			std::vector<std::wstring> fontmap,
			double fontsize,
			Attribute& def
		) :
			BasicShellContext(
				iocpmgr,
				ambiguous_size,
				codepage,
				c_sys,
				c_256,
				use_terminal_echoback,
				fontmap,
				fontsize,
				def
			) {}
		void Terminate()override;
		~RedirectShellContext() {
			if (m_childProcess) {
				CloseHandle(m_childProcess);
			}
			if (m_thread.joinable()) {
				m_thread.detach();
			}
		}
	};
}