#pragma once
#include <ModuleFilePath.h>
#include <ShellContextFactory.h>
#include <DefinedResource.h>
#include <IOCPMgr.h>
#include <strconv.h>
#include <ansi/BasicColorTable.h>
#include "ConsoleReadShellContext.h"
namespace tignear::sakura {


	class ConsoleReadShellContextFactory :public ShellContextFactory {
		HINSTANCE m_hinst;
	public:
		ConsoleReadShellContextFactory(HINSTANCE hinst) :m_hinst(hinst) {}
		std::shared_ptr<ShellContext> Create(const Information& info)const override {
			auto cmd = info.shellConf.get<std::string>("cmd");
			auto fontsize = info.shellConf.get<double>("fontsize");
			auto fontmap = std::vector<std::wstring>();
			auto font = info.shellConf.get<std::string>("font");
			auto admin = false;
			auto hasAdminParam = info.shellConf.has("admin");
			if (hasAdminParam) {
				admin = info.shellConf.get<bool>("admin");
			}
#ifdef UNICODE
			auto cmdc = cp_to_wide(cmd, 65001);
#else
			auto cmdc = std::move(cmd);
#endif // UNICODE

			return ConsoleReadShellContext::Create(stdex::tstring(tignear::win::GetModuleFilePath(m_hinst) /(admin?"AdminConsoleReadShellContext.exe" :"ConsoleReadShellContext.exe")), cmdc,nullptr,L"",utf8_to_wide(font),fontsize);

			
		}
	};
}