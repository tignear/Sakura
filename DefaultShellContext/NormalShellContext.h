#pragma once
#include <memory>
#include "tstring.h"
#include "BasicShellContext.h"
namespace tignear::sakura {
	class NormalShellContext:public BasicShellContext {

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
		bool Init(stdex::tstring, const Options& opt);
	public:
		static std::shared_ptr<NormalShellContext> Create(
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
		NormalShellContext(
			std::shared_ptr<iocp::IOCPMgr> iocpmgr,
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
			codepage,
				c_sys,
				c_256,
				use_terminal_echoback,
				fontmap,
				fontsize,
				def
			) {}
	};
}