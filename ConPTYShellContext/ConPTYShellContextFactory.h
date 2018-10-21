#pragma once
#include <ModuleFilePath.h>
#include <DefinedResource.h>
#include <ShellContextFactory.h>
#include <strconv.h>
#include <memory>
#include "ConPTYShellContext.h"
namespace tignear::sakura {
	class ConPTYShellContextFactory :public ShellContextFactory {
		HINSTANCE m_hinst;
	public:

		ConPTYShellContextFactory(HINSTANCE hinst):m_hinst(hinst){

		}
		std::shared_ptr<ShellContext> Create(const Information& info)const override {
			auto cmd = info.shellConf.get<std::string>("cmd");
			auto iocpMgr = std::static_pointer_cast<tignear::sakura::iocp::IOCPMgr>(info.getResource(resource::IOCPMgr));
			auto terminal_echo = info.shellConf.get<bool>("use_terminal_echoback");
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
			}
#ifdef UNICODE
			auto cmdc = cp_to_wide(cmd, 65001);
#else
			auto cmdc = std::move(cmd);
#endif // UNICODE
			if (admin) {
				return ConPTYShellContext::Create(stdex::tstring(tignear::win::GetModuleFilePath(m_hinst) / "ConPTYShellContextExeAdmin.exe"), cmdc, iocpMgr,2, ansi::BasicSystemColorTable(), ansi::Basic256ColorTable(), terminal_echo, fontmap, fontsize, ConPTYShellContext::Options{});
			}
			else {
				return ConPTYShellContext::Create(stdex::tstring(tignear::win::GetModuleFilePath(m_hinst) / "ConPTYShellContextExe.exe"), cmdc, iocpMgr,2, ansi::BasicSystemColorTable(), ansi::Basic256ColorTable(), terminal_echo, fontmap, fontsize, ConPTYShellContext::Options{});

			}
		}
	};
}