#include "stdafx.h"
#include <numeric>
#include <algorithm>
#include "BasicShellContext.h"
#include "split.h"
using tignear::sakura::BasicShellContext;
using tignear::ansi::AttributeText;
using tignear::stdex::split;
/*
m_viewstartY_itrより後ろの要素とm_viewstartY_itrがさす要素のstartIndexの書き換えが必要になることはあってはならない。
またm_viewstartY_itrより後ろの要素のtextを書き換えてはならない。(m_viewstartY_itrのtextは書き換えても良い)
*/
void BasicShellContext::MoveCurosorYUp(std::wstring::size_type count) {
	for (auto i = 0U; i < count&&m_cursorY_itr!=m_text.begin(); i++) {
		m_cursorY_itr--;
	}
}
void BasicShellContext::MoveCurosorYDown(std::wstring::size_type count) {
	for (auto i = 0U; i < count&&m_cursorY_itr != m_text.end(); i++) {
		m_cursorY_itr++;
	}
}
void BasicShellContext::RemoveRows(size_t count) {
	auto copy_itr = m_cursorY_itr;
	for (auto i = 0U; i < count; i++) {
		copy_itr->clear();
	}
	copy_itr++;
	m_buffer_rebuild = true;
}
void BasicShellContext::RemoveRowsR(size_t count) {
	auto copy = m_cursorY_itr;
	auto ritr=std::reverse_iterator(copy);
	for (auto i = 0U; i < count; i++) {
		(--(ritr.base()))->clear();
	}
	m_buffer_rebuild = true;
}
void BasicShellContext::RemoveColumns(std::wstring::size_type count) {
	auto& elem = (*m_cursorY_itr);
	auto itr= elem.begin();//copy iterator
	std::wstring::size_type removed = 0;
	while (itr != elem.end() && removed < count) {
		if (itr->length() >= count - removed) {
			itr->text().erase(0,count-removed);
			break;
		}
		else {
			removed += itr->length();
			itr=elem.erase(itr);
			continue;
		}
	}
	m_buffer_rebuild = true;
}
void BasicShellContext::RemoveColumnsR(std::wstring::size_type count) {
	auto& elem = (*m_cursorY_itr);
	std::wstring::size_type removed = 0;
	while (removed < count) {
		auto& e = elem.back();
		if (e.length()>=count-removed) {
			e.text().erase(e.length()-(count-removed), count - removed);
			break;
		}
		else {
			removed += e.length();
			elem.pop_back();
			continue;
		}
	}
	m_buffer_rebuild = true;
}
void BasicShellContext::RemoveCursorBefore() {
	auto prev=std::prev(m_cursorY_itr);
	std::for_each(m_viewstartY_itr, prev, [] (std::list<ansi::AttributeText>& e){
		e.clear();
	});
	RemoveColumns(m_cursorX);
	m_buffer_rebuild = true;
}
void BasicShellContext::RemoveCursorAfter() {
	auto next = std::next(m_cursorY_itr);
	std::for_each(next, m_text.end(), [](std::list<ansi::AttributeText>& e) {
		e.clear();
	});
	RemoveColumnsR(CurosorLineLength() - m_cursorX);
	m_buffer_rebuild = true;
}
std::wstring::size_type BasicShellContext::CurosorLineLength() {
	std::wstring::size_type cnt=0;
	for (auto itr = m_cursorY_itr->cbegin(); itr != m_cursorY_itr->cend();itr++) {
		cnt += itr->length();
	}
	return cnt;
}
void BasicShellContext::ParseColor(std::wstring_view sv) {
	auto elems=split<wchar_t, std::vector<std::wstring>>(std::wstring(sv), L";");
	for (auto itr = elems.begin(); itr != elems.end();itr++) {
		auto num = std::stoul(*itr);
		switch (num)
		{
		case 0://reset
			m_current_attr = m_def_attr;
			continue;
		case 1:
			m_current_attr.bold = true;
			continue;
		case 2:
			m_current_attr.faint = true;
			continue;
		case 3:
			m_current_attr.italic = true;
			continue;
		case 4:
			m_current_attr.underline = true;
			continue;
		case 5:
			m_current_attr.blink = ansi::Blink::Slow;
			continue;
		case 6:
			m_current_attr.blink = ansi::Blink::Fast;
			continue;
		case 7:
			m_current_attr.reverse = true;
			continue;
		case 8:
			m_current_attr.conceal = true;
			continue;
		case 9:
			//deprecated
			continue;
		case 20:
			m_current_attr.fluktur = true;
			continue;
		case 21:
			m_current_attr.bold = false;
			continue;
		case 22:
			m_current_attr.bold = false;
			m_current_attr.faint = false;
			continue;
		case 23:
			m_current_attr.italic = false;
			m_current_attr.fluktur = false;
			continue;
		case 24:
			m_current_attr.underline = false;
			continue;
		case 25:
			m_current_attr.blink = ansi::Blink::None;
			continue;
		case 27://???
			m_current_attr.reverse = false;
			continue;
		case 28:
			m_current_attr.conceal = false;
			continue;
		case 29:
			//deprecated
			continue;
		case 38:
		{
			itr++;
			auto t = std::stoul(std::wstring(*itr));
			switch (t)
			{
			case 5:
			{
				itr++;
				m_current_attr.textColor=m_256_color_table.at(std::stoul(std::wstring(*itr)));
				break;
			}
			case 2:
			{
				itr++;
				auto r = std::stoul(std::wstring(*itr));
				itr++;
				auto g=std::stoul(std::wstring(*itr));
				itr++;
				auto b = std::stoul(std::wstring(*itr));
				m_current_attr.textColor = r << 16 | g << 8 | b;
				break;
			}
			default:
				break;
			}
			continue;
		}
		case 39:
			m_current_attr.textColor = m_def_attr.textColor;
			continue;
		case 48:
		{
			itr++;
			auto t = std::stoul(std::wstring(*itr));
			switch (t)
			{
			case 5:
			{
				itr++;
				m_current_attr.backgroundColor = m_256_color_table.at(std::stoul(std::wstring(*itr)));
				break;
			}
			case 2:
			{
				itr++;
				auto r = std::stoul(std::wstring(*itr));
				itr++;
				auto g = std::stoul(std::wstring(*itr));
				itr++;
				auto b = std::stoul(std::wstring(*itr));
				m_current_attr.backgroundColor = r << 16 | g << 8 | b;
				break;
			}
			default:
				break;
			}
			continue;
		}
		case 49:
			m_current_attr.backgroundColor = m_def_attr.backgroundColor;
			continue;
		default:
			break;
		}
		if (num>=10&&num<=19) {
			m_current_attr.font = num - 10;
			continue;
		}
		if (num >= 30 && num <= 37) {
			m_current_attr.textColor =m_system_color_table.at(num);
			continue;
		}
		if (num >= 40 && num <= 47) {
			m_current_attr.backgroundColor = m_system_color_table.at( num );
			continue;
		}
		if (num >= 90 && num <= 97) {
			m_current_attr.textColor = m_system_color_table.at(num);
			continue;
		}
		if (num >= 100 && num <= 107) {
			m_current_attr.backgroundColor = m_system_color_table.at(num);
			continue;
		}
	}
}
void BasicShellContext::FindString(std::wstring_view) {

}
void BasicShellContext::FindCSI(std::wstring_view sv) {
	switch (sv.back())
	{
	case L'A'://cursor up
		sv.remove_suffix(1);
		if (sv.empty()) {
			MoveCurosorYDown(1);
			break;
		}
		MoveCurosorYDown (std::stoul(std::wstring{ sv }));
		break;
	case L'B'://cursor down
		sv.remove_suffix(1);
		if (sv.empty()) {
			MoveCurosorYUp(1);
			break;
		}
		MoveCurosorYUp(std::stoul(std::wstring{ sv }));
		break;
	case L'C'://cursor right
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_cursorX += 1;
			break;
		}
		m_cursorX += std::stoul(std::wstring{ sv });
		break;
	case L'D'://curosor left
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_cursorX -= 1;
			break;
		}
		m_cursorX -= std::stoul(std::wstring{ sv });
		break;
	case L'E':
		m_cursorX = 0;
		sv.remove_suffix(1);
		if (sv.empty()) {
			MoveCurosorYDown(1);
			break;
		}
		MoveCurosorYDown(std::stoul(std::wstring{ sv }));
		break;
	case L'F':
		m_cursorX = 0;
		sv.remove_suffix(1);
		if (sv.empty()) {
			MoveCurosorYUp(1);
			break;
		}
		MoveCurosorYUp(std::stoul(std::wstring{ sv }));
		break;
	case L'G':
		sv.remove_suffix(1);
		m_cursorX=std::stoul(std::wstring( sv ))-1;
		break;
	case L'H':
	case L'f':
	{
		sv.remove_suffix(1);
		auto vec = split<wchar_t, std::vector<std::wstring>>(std::wstring( sv ), L";");
		if (vec[0].empty()) {
			m_cursorY_itr = m_viewstartY_itr;
		}
		else {
			m_cursorY_itr = m_viewstartY_itr;
			MoveCurosorYDown(std::stoul(vec[0]) - 1);
		}
		if (vec[1].empty()) {
			m_cursorX = 0;
		}
		else {
			m_cursorX = std::stoul(vec[1]) - 1;
		}
		break;
	}
	case L'J':
	{
		sv.remove_suffix(1);
		unsigned long prop;
		if (sv.empty()) {
			prop = 0UL;
		}
		else {
			prop = std::stoul(std::wstring{ sv });
		}
		switch (prop)
		{
		case 0:
			RemoveCursorBefore();
		case 1:
			RemoveCursorAfter();
		case 2:
		{
			m_text.erase(m_viewstartY_itr, m_text.end());
			m_viewstartY_itr = m_text.end();
			m_cursorY_itr = m_text.end();
		}

		case 3:
			m_text.clear();
			break;
		default:
			break;
		}
	}
	case L'K':
		sv.remove_suffix(1);
		unsigned long prop;
		if (sv.empty()) {
			prop = 0UL;
		}
		else {
			prop = std::stoul(std::wstring{ sv });
		}
		switch (prop)
		{
		case 0:
			RemoveColumns(m_cursorX);
		case 1:
			RemoveColumnsR(CurosorLineLength() - m_cursorX);
		case 2:
			m_cursorY_itr->clear();
		default:
			break;
		}
	case L'T':
	case L'U':
		//not implemented
		break;
	case L'm':
		sv.remove_suffix(1);
		ParseColor(sv);
		break;
	default:
		break;
	}
}