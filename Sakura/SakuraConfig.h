#pragma once
#include <memory>
#include "lua.hpp"
#include <LuaIntf/LuaIntf.h>
namespace tignear::sakura {
	struct Config {
		LuaIntf::LuaContext L= LuaIntf::LuaContext();
		size_t initshell;
		std::vector<std::tuple<std::string,std::string,LuaIntf::LuaRef>> shells;
	};
	struct config_exception:public std::exception {
	};
	/*
	 *@throw config_exception
	 */
	static inline Config LoadConfig(std::string path) {
		using namespace LuaIntf;
		try {
			Config rconfig{};
			auto& L = rconfig.L;
			auto loadfile = L.getGlobal("dofile");
			auto conf = loadfile.call<LuaRef>(path);
			{
				auto& rshells = rconfig.shells;
				LuaRef shells = conf["shells"];
				for (auto&& e : shells) {

					auto value = e.value();
					rshells.emplace_back(value.get<std::string>("display"), value.get<std::string>("factory"), value);
				}
			}
			{
				rconfig.initshell = 0;
				if (conf.has("initshell")) {
					rconfig.initshell = static_cast<size_t>(conf.get<int>("initshell"));
				}
			}

			return rconfig;
		}
		catch (LuaIntf::LuaException&) {
			throw config_exception();
		}

	}
}