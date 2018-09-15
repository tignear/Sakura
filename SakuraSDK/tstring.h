#pragma once
#include <tchar.h>
#include <string>
//http://marupeke296.com/TIPS_No14_tstring.html
namespace tignear::stdex {
#ifdef UNICODE
	using tstring=std::wstring;
#else
	using tstring = std::string;

#endif
}