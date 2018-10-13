#pragma once
#include <memory>
#include "lua.hpp"
#include <LuaIntf/LuaIntf.h>
namespace tignear::sakura {
	struct Config {
		std::shared_ptr<LuaIntf::LuaContext> L;
		INT initshell;
		std::vector<std::tuple<std::string,std::string,LuaIntf::LuaRef>> shells;
	};
	class config_exception:public std::exception {
		const std::string m_what;
	public:
		config_exception(const char* what):m_what(what) {
			
		}
		const char* what() {
			return m_what.c_str();
		}
	};
	/*
	 *@throw config_exception
	 */
	static inline Config LoadConfig(std::shared_ptr<LuaIntf::LuaContext> L,std::string path) {
		using namespace LuaIntf;
		try {
			Config rconfig{};
			auto&& Ls = *L;
			auto loadfile = Ls.getGlobal("dofile");
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
					rconfig.initshell = static_cast<INT>(conf.get<int>("initshell"));
				}
			}

			return rconfig;
		}
		catch (LuaIntf::LuaException& e) {
			throw config_exception(e.what());
		}

	}
}