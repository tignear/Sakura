#pragma once
#include <tchar.h>
#include <string>
namespace tignear::ansi {
	constexpr std::string_view csi_charsA = "0123456789;:? ";

	template <class R>
	R& parseA(std::string_view wstr, R& r) {
		std::string::size_type nowpos = 0;
		while (nowpos < wstr.length()) {
			auto elmpos = wstr.find_first_of("\x1b\f\b\r\n", nowpos);
			if (elmpos == std::string::npos) {
				auto sv = std::string_view{ wstr };
				sv.remove_prefix(nowpos);
				if (!sv.empty()) {
					r.FindString(sv);
				}
				break;
			}
			auto normalstr = std::string_view{ wstr }.substr(nowpos, elmpos - nowpos);
			if (!normalstr.empty()) {
				r.FindString(normalstr);
			}
			if (wstr[elmpos] == '\r') {
				r.FindCR();
				nowpos=elmpos;
				++nowpos;
				continue;
			}
			if (wstr[elmpos] == '\n') {
				r.FindLF();
				nowpos = elmpos;
				++nowpos;
				continue;
			}
			if (wstr[elmpos] == '\b') {
				r.FindBS();
				nowpos = elmpos;
				++nowpos;
				continue;
			}
			if (wstr[elmpos] == '\f') {
				r.FindFF();
				nowpos = elmpos;
				++nowpos;
				continue;
			}
			if (wstr[elmpos + 1] == '[') {
				auto csiendpos = wstr.find_first_not_of(csi_charsA, elmpos + 2);
				auto sv = std::string_view{ wstr }.substr(elmpos + 2, csiendpos + 1 - (elmpos + 2));
				r.FindCSI(sv);
				nowpos = csiendpos + 1;
				continue;
			}
			else if (wstr[elmpos + 1] == ']') {
				auto oscendpos = wstr.find('\x07', elmpos + 2);
				auto sv = std::string_view{ wstr }.substr(elmpos + 2, oscendpos - (elmpos + 2));
				r.FindOSC(sv);
				nowpos = oscendpos + 1;
				continue;
			}
			else {
				OutputDebugString(_T("Unsupported Sequence\n"));
				++nowpos;
			}
		}
		return r;
	}
}