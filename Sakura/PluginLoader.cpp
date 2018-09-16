#include "stdafx.h"
#define TIGNEAR_SAKURA_IMPORT
#include <PluginEntry.h>
#include "PluginLoader.h"
namespace tignear::sakura {
	std::unordered_map<std::string, std::unique_ptr<ShellContextFactory>> loadPlugin(std::filesystem::path pluginDir) {
		namespace fs = std::filesystem;
		auto r = std::unordered_map<std::string, std::unique_ptr<ShellContextFactory>>();
		for (auto&& entry : fs::directory_iterator(pluginDir)) {
			HMODULE mod=LoadLibraryExW((entry.path()/"plugin.dll").c_str(),NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
			if (mod == NULL) {
				continue;
			}
			auto func=reinterpret_cast<PluginLoadFP>(GetProcAddress(mod,"PluginLoad"));
			if (func == nullptr) {
				FreeLibrary(mod);
				continue;
			}
			auto plugins=(*func)();
			for(auto&& e:(*plugins)){
				r[e.first] = std::move(e.second);
			}
			delete plugins;
		}
		return r;
	}
}