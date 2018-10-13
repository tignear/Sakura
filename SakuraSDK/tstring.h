#pragma once
#include <string>
#include <tchar.h>
namespace tignear::stdex {
#ifdef UNICODE
	using tstring=std::wstring;
	struct {
		template<typename ...Args>                                
		auto operator()(Args&&... args) const->decltype(std::to_wstring(std::forward<Args>(args)...)){          
			return std::to_wstring(std::forward<Args>(args)...);
		}                                                         
	} const to_tstring;
#else
	using tstring = std::string;
	struct {
		template<typename ...Args>
		auto operator()(Args&&... args) const->decltype(std::to_string(std::forward<Args>(args)...)) {
			return std::to_string(std::forward<Args>(args)...);
		}
	} const to_tstring;
#endif
}
