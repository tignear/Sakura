// DefaultShellContext.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "BasicShellContextFactory.h"
#include <PluginEntry.h>
using tignear::sakura::ShellContextFactory;
using tignear::sakura::BasicShellContextFactory;
TIGNEAR_SAKURA_EXPORT std::unordered_map<std::string, std::unique_ptr<tignear::sakura::ShellContextFactory>>* WINAPI PluginLoad(){
	auto r = new std::unordered_map<std::string, std::unique_ptr<tignear::sakura::ShellContextFactory>>();
	(*r)["BasicShellContextFactory"]=std::unique_ptr<ShellContextFactory>(new BasicShellContextFactory());
	return r;
}
