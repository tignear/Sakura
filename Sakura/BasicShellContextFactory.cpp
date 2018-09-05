#pragma once
#include "stdafx.h"
#include "BasicShellContextFactory.h"
#include "BasicShellContext.h"
#include "strconv.h"
#include "ansi/BasicColorTable.h"
#include "DefinedResource.h"
namespace tignear::sakura {
	std::shared_ptr<ShellContext> BasicShellContextFactory::Create(const Information& info)const {
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
		return BasicShellContext::Create(cp_to_wide(cmd, 65001), iocpMgr, cp,ansi::BasicSystemColorTable(), ansi::Basic256ColorTable(), terminal_echo,fontmap,fontsize,BasicShellContext::Options{});
	}
}