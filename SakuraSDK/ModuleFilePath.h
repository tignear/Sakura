#pragma once
#include <windows.h>
#include <filesystem>
#include <tstring.h>
namespace tignear::win {

	//http://www7.plala.or.jp/kfb/program/exedir.html
	//‚ð‰ü•Ï‚µ‚ÄŽg—p
	static inline std::filesystem::path GetModuleFilePath(HMODULE m)
	{
		auto reserve = MAX_PATH;
		stdex::tstring buf;
		while (true) {
			buf.clear();
			buf.reserve(reserve);
			auto copyed = GetModuleFileName(m, buf.data(), static_cast<DWORD>(buf.capacity()-1));
			if (copyed == 0) {
				throw std::runtime_error("GetModuleFileName is Failed");
			}
			if (copyed != buf.capacity()) {
				auto p = std::filesystem::path(buf.c_str());
				//auto p = std::filesystem::path(std::move(buf));//??? do not working this 
				p.remove_filename();
				return p;
			}
			reserve *= 2;
		}
	}


}