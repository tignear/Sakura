// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"
#include <Plugin.h>
#include "ConsoleReadShellContextFactory.h"
#include "RedirectShellContextFactory.h"

using tignear::sakura::ShellContextFactory;
using tignear::sakura::RedirectShellContextFactory;
using tignear::sakura::conread::ConsoleReadShellContextFactory;

namespace {
	HINSTANCE g_hinst;



}
BOOL APIENTRY DllMain(HINSTANCE hinst,
	DWORD reason,
	LPVOID
)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		g_hinst = hinst;
		break;
	default:
		break;
	}
	return TRUE;
}
TIGNEAR_SAKURA_EXPORT tignear::sakura::Plugin* WINAPI CreatePlugin() {
	using namespace tignear::sakura;
	class Impl :public Plugin {
		virtual std::unordered_map<std::string_view, std::unique_ptr<ShellContextFactory>> CreateShellContextFactorys()override {
			std::unordered_map<std::string_view, std::unique_ptr<ShellContextFactory>> r;
			r.emplace("RedirectShellContextFactory", std::unique_ptr<ShellContextFactory>(new RedirectShellContextFactory(g_hinst)));
			r.emplace("ConsoleReadShellContextFactory", std::unique_ptr<ShellContextFactory>(new ConsoleReadShellContextFactory(g_hinst)));
			return r;
		};
	};
	return new Impl();
}