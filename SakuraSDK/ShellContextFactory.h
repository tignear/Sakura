#pragma once
#include <variant>
#include <functional>
#include <string>
#include <lua.hpp>
#include <LuaIntf/LuaIntf.h>
#include "ShellContext.h"
#include "Dpi.h"
namespace tignear::sakura {
	class ShellContextFactory {
	public:
		struct Information {
			DIP width;
			DIP height;
			LuaIntf::LuaRef shellConf;
			std::function<std::shared_ptr<void>(std::string)> getResource;
		};
		virtual std::shared_ptr<ShellContext> Create(const Information&)const=0;

	};
}