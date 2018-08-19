#pragma once
#include <string>
namespace tignear::ansi {
	constexpr std::wstring_view csi_chars=L"0123456789;:? ";
	template <class R>
	R& parse(std::wstring_view wstr,R& r) {
		std::string::size_type nowpos = 0;
		while (nowpos<wstr.length()) {
			auto elmpos = wstr.find(L'\x1b', nowpos);
			if (elmpos == std::wstring::npos) {
				auto sv = std::wstring_view{ wstr };
				sv.remove_prefix(nowpos);
				if (!sv.empty()) {
					r.FindString(sv);
				}
				break;
			}
			auto normalstr=std::wstring_view{ wstr }.substr(nowpos, elmpos-nowpos);
			if (!normalstr.empty()) {
				r.FindString(normalstr);
			}
			if (wstr[elmpos + 1] == L'[') {
				auto csiendpos = wstr.find_first_not_of(csi_chars, elmpos+2);
				auto sv = std::wstring_view{ wstr }.substr(elmpos+2, csiendpos +1-(elmpos+2));
				r.FindCSI(sv);
				nowpos = csiendpos + 1;
				continue;
			}
			else if(wstr[elmpos+1]==L']'){
				auto oscendpos = wstr.find(L'\x07', elmpos + 2);
				auto sv = std::wstring_view{ wstr }.substr(elmpos + 2, oscendpos- (elmpos + 2));
				r.FindOSC(sv);
				nowpos = oscendpos + 1;
				continue;
			}
		}
		return r;
	}
}