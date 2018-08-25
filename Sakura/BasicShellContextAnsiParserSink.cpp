#include "stdafx.h"
#include <unicode/ubrk.h>
#include <unicode/brkiter.h>
#include <unicode/locid.h>
#include <numeric>
#include <cassert>
#include <algorithm>
#include "BasicShellContext.h"
#include "split.h"
#include "EastAsianWidth.h"
using tignear::sakura::BasicShellContext;
using tignear::ansi::AttributeText;
using tignear::stdex::split;
using icu::UnicodeString;
using tignear::icuex::EastAsianWidth;

AttributeText BasicShellContext::CreateAttrText(icu::UnicodeString& str,const Attribute& attr) {
	if (attr.reverse) {
		return AttributeText(str,  attr.backgroundColor, attr.textColor, attr.bold, attr.faint, attr.italic, attr.underline, attr.blink, attr.conceal, attr.crossed_out, attr.font);
	}
	else {
		return AttributeText(str, attr.textColor, attr.backgroundColor, attr.bold, attr.faint, attr.italic, attr.underline, attr.blink, attr.conceal, attr.crossed_out, attr.font);
	}
}
AttributeText BasicShellContext::CreateAttrText(icu::UnicodeString&& str, const Attribute& attr) {
	if (attr.reverse) {
		return AttributeText(str, attr.backgroundColor, attr.textColor, attr.bold, attr.faint, attr.italic, attr.underline, attr.blink, attr.conceal, attr.crossed_out, attr.font);
	}
	else {
		return AttributeText(str, attr.textColor, attr.backgroundColor, attr.bold, attr.faint, attr.italic, attr.underline, attr.blink, attr.conceal, attr.crossed_out, attr.font);
	}
}
/*bool BasicShellContext::EqAttr(const AttributeText& a,const Attribute& b) {
	auto cp = b;
	if (cp.reverse) {
		std::swap(cp.textColor,cp.backgroundColor);
	}
	return a.backgroundColor() == cp.backgroundColor&&
		a.blink() == cp.blink&&
		a.bold() == cp.bold&&
		a.conceal() == cp.conceal&&
		a.crossed_out() == cp.crossed_out&&
		a.faint() == cp.faint&&
		a.fluktur() == cp.fluktur&&
		a.font() == cp.font&&
		a.italic() == cp.italic&&
		a.textColor() == cp.textColor&&
		a.underline() == cp.underline;
}*/
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
void BasicShellContext::RemoveRows(std::list<std::list<AttributeText>>::size_type count) {
	auto copy_itr = m_cursorY_itr;
	for (auto i = 0U; i < count; i++) {
		copy_itr->clear();
		copy_itr++;
	}
}
void BasicShellContext::RemoveRowsR(std::list<std::list<AttributeText>>::size_type count) {
	auto copy = m_cursorY_itr;
	auto ritr=std::reverse_iterator(copy);
	for (auto i = 0U; i < count; i++) {
		(--(ritr.base()))->clear();
	}
}
void BasicShellContext::RemoveColumns() {
	if (m_cursorY_itr == m_text.end()) {
		return;
	}
	auto& elem = (*m_cursorY_itr);
	auto itr= elem.begin();//copy iterator
	int32_t removed = 0;
	while (itr != elem.end() && removed < m_cursorX) {
		if(static_cast<int32_t>(itr->lengthEAW()) > m_cursorX - removed) {
			itr->textE(
				[this, &removed](auto self, auto e) {
				UErrorCode status = U_ZERO_ERROR;
				int32_t previous;
				int32_t current;
				icu::BreakIterator *it = icu::BreakIterator::createCharacterInstance(
					icu::Locale::getDefault(), status
				);
				it->setText(e);
				for (
					previous = it->first(), current = it->next();
					current != icu::BreakIterator::DONE&&removed < m_cursorX;
					previous = current, current = it->next()
					) {
					auto size = current - previous;
					auto count32 = e.countChar32(previous, size);
					if (count32 == 1) {
						auto eaw = EastAsianWidth(e.char32At(previous));
						removed += eaw;
					}
					else {
						removed += 2;//書記素クラスタがUTF32で1文字で表されない場合は文字幅2(要検証)
					}
				}
				e.removeBetween(0,current);
				m_cursorX = 0;
				return true;
			});
			break;
		}
		else {
			removed += itr->lengthEAW();
			itr=elem.erase(itr);
			continue;
		}
	}
}
void BasicShellContext::RemoveColumnsR() {
	if (m_cursorY_itr == m_text.end()) {
		return;
	}
	auto& elem = (*m_cursorY_itr);
	int32_t skip = 0;
	auto itr = elem.begin();
	for (; itr != elem.end() && skip <= m_cursorX;itr++) {
		if (itr->lengthEAW() >static_cast<uint32_t>(m_cursorX-skip)) {
			//https://qiita.com/masakielastic/items/a3387817afb0a03def2b#unicodestring
			itr->textE([this,&skip](auto self, icu::UnicodeString& e2) {
				UErrorCode status = U_ZERO_ERROR;
				int32_t previous;
				int32_t current;
				icu::BreakIterator *it = icu::BreakIterator::createCharacterInstance(
					icu::Locale::getDefault(), status
				);
				it->setText(e2);
				for (
					previous = it->first(), current = it->next();
					current != icu::BreakIterator::DONE&&skip<m_cursorX;
					previous = current, current = it->next()
					) {
					auto size = current - previous;
					auto count32=e2.countChar32(previous, size);
					if (count32 == 1) {
						auto eaw=EastAsianWidth(e2.char32At(previous));
						skip += eaw;
					}
					else {
						skip += 2;//書記素クラスタがUTF32で1文字で表されない場合は文字幅2(要検証)
					}
				}
				e2.removeBetween(previous,e2.length());
				m_cursorX = skip;
				return true;
			});
			itr++;
			break;
		}
		else {
			skip += itr->lengthEAW();
			continue;
		}
	}
	elem.erase(itr,elem.end());
}
void BasicShellContext::RemoveCursorBefore() {
	auto prev=std::prev(m_cursorY_itr);
	std::for_each(m_viewstartY_itr, prev, [] (std::list<ansi::AttributeText>& e){
		e.clear();
	});
	RemoveColumns();
}
void BasicShellContext::RemoveCursorAfter() {
	auto next = std::next(m_cursorY_itr);
	std::for_each(next, m_text.end(), [](std::list<ansi::AttributeText>& e) {
		e.clear();
	});
	RemoveColumnsR();
}
int32_t BasicShellContext::CurosorLineLength() {
	int32_t cnt=0;
	for (auto itr = m_cursorY_itr->cbegin(); itr != m_cursorY_itr->cend();itr++) {
		cnt += itr->length();
	}
	return cnt;
}
void BasicShellContext::InsertCursorPos(const std::wstring& wstr) {
	assert(!wstr.empty());
	int32_t i=0;
	auto ustr = UnicodeString(wstr.c_str());
	if (m_cursorY_itr == m_text.end()) {
		m_cursorX = EastAsianWidth(ustr);
		m_text.push_back({ CreateAttrText(std::move(ustr), m_current_attr) });
		m_cursorY_itr = m_text.end();
		m_cursorY_itr--;
		return;
	}
	for (auto itr = m_cursorY_itr->begin(); itr != m_cursorY_itr->end();itr++) {
		int32_t l=static_cast<int32_t>(itr->lengthEAW());
		if (i+l > m_cursorX) {
			auto ustrlen = EastAsianWidth(ustr);
			auto titr = itr;
			if (m_attr_updated) {
				itr->textE([nitr = std::next(itr),itr,this,&ustr,&i](auto& self,auto& e){
					UErrorCode status = U_ZERO_ERROR;
					int32_t previous;
					int32_t current;
					icu::BreakIterator *it = icu::BreakIterator::createCharacterInstance(
						icu::Locale::getDefault(), status
					);
					it->setText(e);
					for (
						previous = it->first(), current = it->next();
						current != icu::BreakIterator::DONE&&i < m_cursorX;
						previous = current, current = it->next()
						) {
						auto size = current - previous;
						auto count32 = e.countChar32(previous, size);
						if (count32 == 1) {
							auto eaw = EastAsianWidth(e.char32At(previous));
							i += eaw;
						}
						else {
							i += 2;
						}
					}
					if (-1 == current) {
						m_cursorY_itr->insert(nitr,CreateAttrText(ustr,m_current_attr));
						return false;
					}
					else {
						auto afustr = UnicodeString(e,current);
						e.remove(0, current);
						m_cursorY_itr->insert(nitr, { CreateAttrText(ustr, m_current_attr),AttributeText(afustr,itr->textColor(),itr->backgroundColor(),itr->bold(),itr->faint(),itr->italic(),itr->underline(),itr->blink(),itr->conceal(),itr->crossed_out(),itr->font()) });
						return true;
					}
				});
			}
			else {
				titr->textE(
					[this, &ustr, &i](auto& self, auto& e) {
					UErrorCode status = U_ZERO_ERROR;
					int32_t previous;
					int32_t current;
					icu::BreakIterator *it = icu::BreakIterator::createCharacterInstance(
						icu::Locale::getDefault(), status
					);
					it->setText(e);
					for (
						previous = it->first(), current = it->next();
						current != icu::BreakIterator::DONE&&i < m_cursorX;
						previous = current, current = it->next()
						) {
						auto size = current - previous;
						auto count32 = e.countChar32(previous, size);
						if (count32 == 1) {
							auto eaw = EastAsianWidth(e.char32At(previous));
							i += eaw;
						}
						else {
							i += 2;
						}
					}
					if (-1 == current) {
						e.append(ustr);
					}
					else {
						e.insert(previous, ustr);
					}
					//e.insert(m_cursorX - i, ustr); 
					return true;
				});
				m_cursorX += ustrlen;
				return;
			}
		}
		i += l;
	}
	m_cursorX =i+ EastAsianWidth(ustr);
	m_cursorY_itr->back().textE([&ustr](auto& self, auto& e) {
		e+= ustr;
		return true;
	});

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
			m_current_attr.crossed_out=true;
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
			m_current_attr.crossed_out = false;
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
			m_current_attr.backgroundColor = m_system_color_table.at(num);
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
void BasicShellContext::FindString(std::wstring_view str) {
	if (m_attr_updated) {
		m_text.back().push_back(CreateAttrText(u"",m_current_attr));
		m_attr_updated = false;
	}
	auto r=split<wchar_t, std::vector<std::wstring>>(std::wstring(str), L"\n");
	auto back=r.back();
	r.pop_back();
	for (auto e : r) {
		e +=L"\n";
		InsertCursorPos(std::move(e));
		m_cursorX++;
		MoveCurosorYDown(1);
	}
	if (!back.empty()) {
		InsertCursorPos(std::move(back));
	}

}
void BasicShellContext::FindCSI(std::wstring_view sv) {
	m_attr_updated = true;
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
			RemoveCursorAfter();
			break;
		case 1:
			RemoveCursorBefore();
			break;
		case 2:
		{
			m_text.erase(m_viewstartY_itr, m_text.end());
			m_viewstartY_itr = m_text.end();
			m_cursorY_itr = m_text.end();
			break;
		}

		case 3:
			m_text.clear();
			break;
		default:
			break;
		}
		break;
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
			RemoveColumnsR();

			break;
		case 1:
			RemoveColumns();
			break;
		case 2:
			m_cursorY_itr->clear();
			break;
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
void BasicShellContext::FindOSC(std::wstring_view sv) {
	auto r = split<wchar_t, std::vector<std::wstring>>(std::wstring(sv), L";");
	switch (std::stoul(r[0]))
	{
	case 0:
		m_title = r[1];
		break;
	default:
		break;
	}
}
void BasicShellContext::FindBS() {
	OutputDebugString(_T("BackSpace\n"));
	int32_t i = 0;
	if (m_cursorY_itr == m_text.end()) {
		return;
	}
	for (auto itr = m_cursorY_itr->begin(); itr != m_cursorY_itr->end(); itr++) {
		auto l = itr->length();
		if (i + l >= m_cursorX) {
			itr->textE([this, i](auto& self, auto& e) {e.remove(m_cursorX - i, 1); return true; });
			m_cursorX -= 1;
			if (l == 1) {
				m_cursorY_itr->erase(itr);
			}
			return;
		}
		i += l;
	}
	m_cursorY_itr->back().textE([len = m_cursorY_itr->back().length()](auto& self, auto& e)
	{
		e.remove(len - 1, 1);	
		return true; 
	});
	if (m_cursorY_itr->back().length() == 0) {
		m_cursorY_itr->pop_back();
	}
}
void BasicShellContext::FindFF() {
	m_attr_updated = true;
}