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

//Ownership to move to the caller!
extern "C" {TIGNEAR_SAKURA_EXPORT std::unordered_map<std::string, std::unique_ptr<tignear::sakura::ShellContextFactory>>* WINAPI PluginLoad(); }
using PluginLoadFP = decltype(&PluginLoad);
