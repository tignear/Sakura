#pragma once
#include "stdafx.h"
#include <ModuleFilePath.h>
#include <strconv.h>
#include <ansi/BasicColorTable.h>
#include <DefinedResource.h>

#include "RedirectShellContextFactory.h"
#include "ShellExecuteExShellContext.h"
#include "RedirectShellContext.h"

namespace tignear::sakura {
	std::shared_ptr<ShellContext> RedirectShellContextFactory::Create(const Information& info)const {
		auto cmd=info.shellConf.get<std::string>("cmd");
		auto iocpMgr=std::static_pointer_cast<tignear::sakura::iocp::IOCPMgr>(info.getResource(resource::IOCPMgr));
		auto cp = info.shellConf.get<int>("codepage");
		auto terminal_echo = info.shellConf.get<bool>("use_terminal_echoback");
		auto fontsize= info.shellConf.get<double>("fontsize");
		auto fontmap = std::vector<std::wstring>();
		LuaIntf::LuaRef fonts = info.shellConf["fonts"];
		for (auto e : fonts) {
			fontmap.push_back(cp_to_wide(e.value().toValue<std::string>(),65001));
		}
		auto admin = false;
		auto hasAdminParam = info.shellConf.has("admin");
		if (hasAdminParam) {
			admin=info.shellConf.get<bool>("admin");
		}
#ifdef UNICODE
		auto cmdc = cp_to_wide(cmd, 65001);
#else
		auto cmdc = std::move(cmd);
#endif // UNICODE
		if (admin) {
			return ShellExecuteExShellContext::Create(stdex::tstring(tignear::win::GetModuleFilePath(m_hinst) / "AdminRedirectShellContext.exe"), cmdc, iocpMgr, 1,cp, ansi::BasicSystemColorTable(), ansi::Basic256ColorTable(), terminal_echo, fontmap, fontsize, ShellExecuteExShellContext::Options{});
		}
		else {

			return RedirectShellContext::Create(cmdc, iocpMgr,1, cp, ansi::BasicSystemColorTable(), ansi::Basic256ColorTable(), terminal_echo, fontmap, fontsize, RedirectShellContext::Options{});

		}
	}
}