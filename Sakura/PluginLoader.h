#pragma once
#include <unordered_map>
#include <string>
#include <ShellContextFactory.h>
#include <filesystem>
namespace tignear::sakura {
	std::unordered_map<std::string, std::unique_ptr<ShellContextFactory>> loadPlugin(std::filesystem::path pluginDir);
}