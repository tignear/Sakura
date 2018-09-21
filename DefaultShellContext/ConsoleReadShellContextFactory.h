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
			//auto iocpMgr = std::static_pointer_cast<iocp::IOCPMgr>(info.getResource(resource::IOCPMgr));
			//auto cp = info.shellConf.get<int>("codepage");
			/*auto terminal_echo = info.shellConf.get<bool>("use_terminal_echoback");
			auto fontsize = info.shellConf.get<double>("fontsize");
			auto fontmap = std::vector<std::wstring>();
			LuaIntf::LuaRef fonts = info.shellConf["fonts"];
			for (auto e : fonts) {
				fontmap.push_back(cp_to_wide(e.value().toValue<std::string>(), 65001));
			}
			auto admin = false;
			auto hasAdminParam = info.shellConf.has("admin");
			if (hasAdminParam) {
				admin = info.shellConf.get<bool>("admin");
			}*/
#ifdef UNICODE
			auto cmdc = cp_to_wide(cmd, 65001);
#else
			auto cmdc = std::move(cmd);
#endif // UNICODE
			/*if (admin) {
				return std::shared_ptr<ShellContext>();
				//return ShellExecuteExShellContext::Create(stdex::tstring(tignear::win::GetModuleFilePath(m_hinst) / "AdminRedirectShellContext.exe"), cmdc, iocpMgr, cp, ansi::BasicSystemColorTable(), ansi::Basic256ColorTable(), terminal_echo, fontmap, fontsize, ShellExecuteExShellContext::Options{});
			}*/
			//else {

				return ConsoleReadShellContext::Create(stdex::tstring(tignear::win::GetModuleFilePath(m_hinst) / "ConsoleReadShellContext.exe"), cmdc,nullptr,L"");

			//}
		}
	};
}