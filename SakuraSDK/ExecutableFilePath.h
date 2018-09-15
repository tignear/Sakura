#pragma once
#include <windows.h>
#include <filesystem>
#include "tstring.h"
namespace tignear::win {

	//http://www7.plala.or.jp/kfb/program/exedir.html
	//‚ð‰ü•Ï‚µ‚ÄŽg—p
	class ExecutableFilePath
	{
	public:
		ExecutableFilePath()
		{
			auto reserve = MAX_PATH;
			stdex::tstring buf;
			while (true) {
				buf.clear();
				buf.reserve(reserve);
				auto copyed = GetModuleFileName(NULL, buf.data(), static_cast<DWORD>(buf.capacity()));
				if (copyed == 0) {
					throw std::runtime_error("GetModuleFileName is Failed");
				}
				if (copyed != buf.capacity()) {
					auto p = std::filesystem::path(buf.c_str());
					//auto p = std::filesystem::path(std::move(buf));//??? do not working this 
					p.remove_filename();
					m_path = p;
					return;
				}
				reserve *= 2;
			}
		}
		std::filesystem::path GetPath() {
			return m_path;
		}
		~ExecutableFilePath() {
			OutputDebugString(_T("???"));
		}
	private:
		std::filesystem::path m_path;

	};
	static inline std::filesystem::path GetExecutableFilePath()
	{
		static auto p = ExecutableFilePath().GetPath();
		return p;
	}


}