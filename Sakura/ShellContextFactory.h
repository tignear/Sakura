#pragma once
#include <variant>
#include <functional>
#include <string>
#include "ShellContext.h"
#include <lua.hpp>
#include <LuaIntf/LuaIntf.h>
namespace tignear::sakura {
	class ShellContextFactory {
	public:
		struct Information {
			uint32_t width;
			uint32_t height;
			LuaIntf::LuaRef shellConf;
			std::function<std::shared_ptr<void>(std::string)> getResource;
		};
		virtual std::shared_ptr<ShellContext> Create(const Information&)const=0;

	};
}