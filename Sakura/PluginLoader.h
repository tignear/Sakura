#pragma once
#include <Windows.h>
#include <unordered_map>
#include <string>
#include <ShellContextFactory.h>
#include <filesystem>
namespace tignear::sakura {
	struct handle_deleter
	{
		void operator()(HMODULE p) const
		{
			FreeLibrary(p);
		}
	};
	using Plugin = std::unique_ptr<std::remove_pointer<HMODULE>::type, handle_deleter>;
	using Plugins = std::vector<Plugin>;
	Plugins loadPlugin(std::filesystem::path pluginDir);
	std::unordered_map < std::string, std::unique_ptr<ShellContextFactory>> loadShellContext(const Plugins&);
}