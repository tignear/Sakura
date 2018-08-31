#pragma once
#include <memory>
#include "lua.hpp"
#include <LuaIntf/LuaIntf.h>
namespace tignear::sakura {
	struct LuaStateCloser {
		void operator()(lua_State *l) const {
			lua_close(l);
		}
	};
	using LuaSta = std::unique_ptr<lua_State,LuaStateCloser>;

	LuaSta openwithStdLib() {
		auto L=LuaSta(luaL_newstate());
		luaL_openlibs(L.get());
		return L;
	}
	class ConfigLoader {
		LuaSta m_state;
	public:
		ConfigLoader():m_state(openwithStdLib()){
		}
	};
}