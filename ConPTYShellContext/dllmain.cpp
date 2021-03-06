// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"
#include <Plugin.h>
#include "ConPtyShellContextFactory.h"
static HINSTANCE g_hinst;
BOOL APIENTRY DllMain( HMODULE hinst,
                       DWORD  ul_reason_for_call,
                       LPVOID 
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		g_hinst = hinst;
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
using namespace tignear::sakura;
TIGNEAR_SAKURA_EXPORT Plugin* WINAPI CreatePlugin() {
	class Impl :public Plugin {
		virtual std::unordered_map<std::string_view, std::unique_ptr<ShellContextFactory>> CreateShellContextFactorys()override {
			std::unordered_map<std::string_view, std::unique_ptr<ShellContextFactory>> r;
			r.emplace("ConPTYShellContextFactory",new ConPTYShellContextFactory(g_hinst));
			return r;
		};
	};
	return new Impl();
}
