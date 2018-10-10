#pragma once
#include <unordered_map>
#include <memory>
#include "ShellContextFactory.h"
#ifndef TIGNEAR_SAKURA_EXPORT
#ifdef TIGNEAR_SAKURA_IMPORT
#define TIGNEAR_SAKURA_EXPORT
#else
#define TIGNEAR_SAKURA_EXPORT __declspec(dllexport)
#endif
#endif // !TIGNEAR_SAKURA_EXPORT
namespace tignear::sakura {
	class Plugin
	{
	public:
		Plugin() {}
		virtual ~Plugin() {}
		virtual std::unordered_map<std::string_view, std::unique_ptr<ShellContextFactory>> CreateShellContextFactorys() {
			return {};
		};
	private:

	};

}
//Ownership to move to the caller!
extern "C" {TIGNEAR_SAKURA_EXPORT tignear::sakura::Plugin* WINAPI CreatePlugin(); }
using CreatePluginFP = decltype(&CreatePlugin);