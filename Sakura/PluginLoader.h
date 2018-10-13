#pragma once
#define TIGNEAR_SAKURA_IMPORT
#include <Plugin.h>
#include <Windows.h>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include "Environment.h"
#include <ShellContextFactory.h>
#include <filesystem>
#include <optional>
#include <strconv.h>
#include <LuaIntf/LuaIntf.h>
namespace tignear::sakura {
	struct PluginInfo{
		unsigned int manifestVersion;
		virtual ~PluginInfo()noexcept{}
	};
	struct PluginInfov1:public PluginInfo {
		std::string name;
		std::unordered_set<std::string> dependencies;
		LuaIntf::LuaRef main;
		std::filesystem::path manifestFilePath;
	};
	struct FreeLib {
		void operator()(HMODULE p) {
			FreeLibrary(p);
		}
	};
	class PluginManager {
		std::unordered_set<std::unique_ptr<std::remove_pointer<HMODULE>::type,FreeLib>> m_hmodules;
		std::unordered_set<std::unique_ptr<Plugin>> m_plugins;
		std::shared_ptr<LuaIntf::LuaContext> L;
	private:

		static std::unordered_map<std::string,std::unique_ptr<PluginInfo>> loadPluginInfo(LuaIntf::LuaContext& L,std::filesystem::path pluginDir) {
			namespace fs = std::filesystem;
			using LuaIntf::LuaRef;
			std::unordered_map<std::string, std::unique_ptr<PluginInfo>> ret;

			auto&& dofile = L.getGlobal("dofile");
			for (auto&& entry : fs::directory_iterator(pluginDir)) {
				try {
					auto manifestFilePath = entry.path() / "manifest.lua";
					auto manifest = dofile.call<LuaRef>(manifestFilePath.u8string().c_str());
					auto infop=std::make_unique<PluginInfov1>();
					auto& info = *infop;
					auto manifestVersion = manifest.get<unsigned int>("manifestVersion");
					if (manifestVersion != 1) {
						continue;
					}
					info.manifestVersion = manifestVersion;
					info.manifestFilePath = manifestFilePath;
					info.name = manifest.get<std::string>("name");
					LuaRef dependencies = manifest["dependencies"];
					for (auto&& dentry : dependencies) {
						info.dependencies.insert(dentry.value().toValue<std::string>());
					}
					info.main = manifest["main"];
					ret.insert({ info.name,std::move(infop) });
				}
				catch (LuaIntf::LuaException e) {
					auto what=e.what();
					OutputDebugStringA(what);
				}
			}
			return ret;
		}
		void executeMain(const PluginInfo& info,const std::unordered_map<std::string, std::unique_ptr<PluginInfo>>& infos, std::unordered_set<std::string>& executed) {
			using LuaIntf::LuaRef;
			switch (info.manifestVersion) {
			case 1:
			{
				const auto& infov = dynamic_cast<const PluginInfov1&>(info);
				for (const auto& e : infov.dependencies) {
					if (executed.find(e)!=executed.end()) {
						continue;
					}
					executeMain(*infos.at(e),infos,executed);
				}
				auto table=LuaRef::createTable(infov.main.state());
				table["manifestPath"] = infov.manifestFilePath.u8string();


				switch (environment::arch()) {
				case environment::CPU_ARCH::X64:
					table["arch"] = "x64";
					break;
				case environment::CPU_ARCH::X86:
					table["arch"] = "x86";
					break;
				}
				infov.main.call(table);
				executed.insert(infov.name);
			}
			}
		}
		const void executeMain(const std::unordered_map<std::string, std::unique_ptr<PluginInfo>>& infos) {
			std::unordered_set<std::string> executed;
			for (const auto& e : infos) {
				executeMain(*e.second,infos,executed);
			}
		}
		
	public:
		const std::unordered_set<std::unique_ptr<Plugin>>& plugins() {
			return m_plugins;
		}
		struct Config {
			HWND hwnd = NULL;
		};
		static PluginManager loadPlugin(std::filesystem::path pluginDir, std::shared_ptr<LuaIntf::LuaContext> L,const Config& conf=Config()) {
			PluginManager mgr;
			mgr.L = L;
			try {
				L->doString(
				u8R"(
					tignear=tignear or {}
					tignear.sakura = tignear.sakura or {}
				)"
				);
				LuaIntf::LuaBinding(L->getGlobal(u8"tignear.sakura"))
					.addFunction(u8"loadPluginDLL", [&mgr](std::string str) {
						auto path = std::filesystem::u8path(str);
						auto mod=(mgr.m_hmodules.emplace(LoadLibraryExW(path.wstring().c_str(), NULL, LOAD_LIBRARY_SEARCH_USER_DIRS| LOAD_LIBRARY_SEARCH_DEFAULT_DIRS)).first)->get();
						if (!mod) {
							return false;
						}
					
						auto proc = reinterpret_cast<CreatePluginFP>(GetProcAddress(mod, CreatePluginFunctionNameA));
						if (!proc) {
							return false;
						}
						mgr.m_plugins.emplace(proc());
						return true;
					})
					.addFunction(u8"AddDllDirectory", [](std::string pstr) {
						auto path = std::filesystem::u8path(pstr);
						auto wstr = path.wstring();
						if (!AddDllDirectory(wstr.c_str())) {
							return false;
						}
						return true;
					})
					.addFunction(u8"MessageBox", [hwnd=conf.hwnd](const std::string& mes, const std::string& title, const int& type) {
						auto bufm=utf8_to_ansi(mes);
						auto buft= utf8_to_ansi(title.empty()?"Sakura":title);

						return MessageBoxA(hwnd , bufm.c_str(), buft.c_str(), type);
					}, LUA_ARGS(std::string, LuaIntf::_opt<std::string>, LuaIntf::_opt<int>));

				auto&& pluginInfos = loadPluginInfo(*L, pluginDir);
				mgr.executeMain(pluginInfos);
			}catch (LuaIntf::LuaException e) {
				auto what = e.what();
				OutputDebugStringA(what);
			}
			return mgr;
		}
	};

}