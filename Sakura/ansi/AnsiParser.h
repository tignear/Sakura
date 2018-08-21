#pragma once
#include <string>
namespace tignear::ansi {
	constexpr std::wstring_view csi_charsW=L"0123456789;:? ";
	constexpr std::string_view csi_charsA = "0123456789;:? ";

	template <class R>
	R& parseW(std::wstring_view wstr,R& r) {
		std::wstring::size_type nowpos = 0;
		while (nowpos<wstr.length()) {
			auto elmpos = wstr.find_first_of(L"\x1b\f\b", nowpos);
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
			if (wstr[elmpos] == L'\b') {
				r.FindBS();
				nowpos++;
				continue;
			}
			if (wstr[elmpos] == L'\f') {
				r.FindFF();
				nowpos++;
				continue;
			}
			if (wstr[elmpos + 1] == L'[') {
				auto csiendpos = wstr.find_first_not_of(csi_charsW, elmpos+2);
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
			else {
				OutputDebugString(_T("Unsupported Sequence"));
				nowpos += 1;
			}
		}
		return r;
	}
	template <class R>
	R& parseA(std::string_view wstr, R& r) {
		std::string::size_type nowpos = 0;
		while (nowpos < wstr.length()) {
			auto elmpos = wstr.find_first_of("\x1b\f\b", nowpos);
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
			if (wstr[elmpos] == '\b') {
				r.FindBS();
				nowpos++;
				continue;
			}
			if (wstr[elmpos] == '\f') {
				r.FindFF();
				nowpos++;
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
				OutputDebugString(_T("Unsupported Sequence"));
				nowpos += 1;
			}
		}
		return r;
	}
	template <class R>
	R& parse32(std::u32string_view wstr, R& r) {
		std::string::size_type nowpos = 0;
		while (nowpos < wstr.length()) {
			auto elmpos = wstr.find_first_of(U"\x1b\f\b", nowpos);
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
			if (wstr[elmpos] == U'\b') {
				r.FindBS();
				nowpos++;
				continue;
			}
			if (wstr[elmpos] == U'\f') {
				r.FindFF();
				nowpos++;
				continue;
			}
			if (wstr[elmpos + 1] == U'[') {
				auto csiendpos = wstr.find_first_not_of(csi_charsA, elmpos + 2);
				auto sv = std::string_view{ wstr }.substr(elmpos + 2, csiendpos + 1 - (elmpos + 2));
				r.FindCSI(sv);
				nowpos = csiendpos + 1;
				continue;
			}
			else if (wstr[elmpos + 1] == U']') {
				auto oscendpos = wstr.find(U'\x07', elmpos + 2);
				auto sv = std::string_view{ wstr }.substr(elmpos + 2, oscendpos - (elmpos + 2));
				r.FindOSC(sv);
				nowpos = oscendpos + 1;
				continue;
			}
			else {
				OutputDebugString(_T("Unsupported Sequence"));
				nowpos += 1;
			}
		}
		return r;
	}
}