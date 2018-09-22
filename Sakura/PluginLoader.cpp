#include "stdafx.h"
#define TIGNEAR_SAKURA_IMPORT
#include <PluginEntry.h>
#include "PluginLoader.h"
namespace tignear::sakura {
	Plugins loadPlugin(std::filesystem::path pluginDir) {
		namespace fs = std::filesystem;
		Plugins r2;
		for (auto&& entry : fs::directory_iterator(pluginDir)) {
			auto mod=Plugin(LoadLibraryExW((entry.path()/"plugin.dll").c_str(),NULL, LOAD_WITH_ALTERED_SEARCH_PATH));
			if (mod == NULL) {
				continue;
			}
			r2.push_back(std::move(mod));
		}
		return r2;
	}
	std::unordered_map<std::string, std::unique_ptr<ShellContextFactory>> loadShellContext(const Plugins& src) {
		auto r = std::unordered_map<std::string, std::unique_ptr<ShellContextFactory>>();
		for (auto&& mod : src) {
			auto func = reinterpret_cast<PluginLoadFP>(GetProcAddress(mod.get(), "PluginLoad"));
			if (!func) {
				continue;
			}
			auto plugins = (*func)();
			for (auto&& e : (*plugins)) {
				r[e.first] = std::move(e.second);
			}
			delete plugins;
		}
		return r;
	}
}