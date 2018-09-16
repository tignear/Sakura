// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"
#include "BasicShellContextFactory.h"
#include <PluginEntry.h>
using tignear::sakura::ShellContextFactory;
using tignear::sakura::RedirectShellContextFactory;
HINSTANCE g_hinst;
BOOL APIENTRY DllMain( HINSTANCE hinst,
                       DWORD reason,
                       LPVOID
                     )
{
	switch(reason)
	{
	case DLL_PROCESS_ATTACH:
		g_hinst = hinst;
		break;
	default:
		break;
	}
	return TRUE;
}
TIGNEAR_SAKURA_EXPORT std::unordered_map<std::string, std::unique_ptr<tignear::sakura::ShellContextFactory>>* WINAPI PluginLoad() {
	auto r = new std::unordered_map<std::string, std::unique_ptr<tignear::sakura::ShellContextFactory>>();
	(*r)["RedirectShellContextFactory"] = std::unique_ptr<ShellContextFactory>(new RedirectShellContextFactory(g_hinst));
	return r;
}
