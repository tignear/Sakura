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

void BasicShellContext::ParseColor(std::wstring_view sv) {
	auto elems=split<wchar_t, std::vector<std::wstring>>(std::wstring(sv), L";");
	for (auto itr = elems.begin(); itr != elems.end();++itr) {
		auto num = std::stoul(*itr);
		switch (num)
		{
		case 0://reset
			m_document.SetAttriuteDefault();
			continue;
		case 1:
			m_document.EditCurrentAttribute().bold = true;
			continue;
		case 2:
			m_document.EditCurrentAttribute().faint = true;
			continue;
		case 3:
			m_document.EditCurrentAttribute().italic = true;
			continue;
		case 4:
			m_document.EditCurrentAttribute().underline = true;
			continue;
		case 5:
			m_document.EditCurrentAttribute().blink = ansi::Blink::Slow;
			continue;
		case 6:
			m_document.EditCurrentAttribute().blink = ansi::Blink::Fast;
			continue;
		case 7:
			m_document.EditCurrentAttribute().reverse = true;
			continue;
		case 8:
			m_document.EditCurrentAttribute().conceal = true;
			continue;
		case 9:
			m_document.EditCurrentAttribute().crossed_out=true;
			continue;
		case 20:
			m_document.EditCurrentAttribute().fluktur = true;
			continue;
		case 21:
			m_document.EditCurrentAttribute().bold = false;
			continue;
		case 22:
			m_document.EditCurrentAttribute().bold = false;
			m_document.EditCurrentAttribute().faint = false;
			continue;
		case 23:
			m_document.EditCurrentAttribute().italic = false;
			m_document.EditCurrentAttribute().fluktur = false;
			continue;
		case 24:
			m_document.EditCurrentAttribute().underline = false;
			continue;
		case 25:
			m_document.EditCurrentAttribute().blink = ansi::Blink::None;
			continue;
		case 27://???
			m_document.EditCurrentAttribute().reverse = false;
			continue;
		case 28:
			m_document.EditCurrentAttribute().conceal = false;
			continue;
		case 29:
			m_document.EditCurrentAttribute().crossed_out = false;
			continue;
		case 38:
		{
			++itr;
			auto t = std::stoul(std::wstring(*itr));
			switch (t)
			{
			case 5:
			{
				itr++;
				m_document.EditCurrentAttribute().frColor.color_256=static_cast<unsigned char>(std::stoul(std::wstring(*itr)));
				m_document.EditCurrentAttribute().frColor.type = ColorType::Color256;
				break;
			}
			case 2:
			{
				m_document.EditCurrentAttribute().frColor.type = ColorType::ColorTrue;
				++itr;
				m_document.EditCurrentAttribute().frColor.color_true.r = static_cast<unsigned char>(std::stoul(std::wstring(*itr)));
				++itr;
				m_document.EditCurrentAttribute().frColor.color_true.g= static_cast<unsigned char>(std::stoul(std::wstring(*itr)));
				++itr;
				m_document.EditCurrentAttribute().frColor.color_true.b = static_cast<unsigned char>(std::stoul(std::wstring(*itr)));
				break;
			}
			default:
				break;
			}
			continue;
		}
		case 39:
			m_document.EditCurrentAttribute().frColor =m_document.GetDefaultAttribute().frColor;
			continue;
		case 48:
		{
			++itr;
			auto t = std::stoul(std::wstring(*itr));
			switch (t)
			{
			case 5:
			{
				itr++;
				m_document.EditCurrentAttribute().bgColor.color_256 = static_cast<unsigned char>(std::stoul(std::wstring(*itr)));
				m_document.EditCurrentAttribute().bgColor.type = ColorType::Color256;
				break;
			}
			case 2:
			{
				m_document.EditCurrentAttribute().bgColor.type = ColorType::ColorTrue;
				++itr;
				m_document.EditCurrentAttribute().bgColor.color_true.r = static_cast<unsigned char>(std::stoul(std::wstring(*itr)));
				++itr;
				m_document.EditCurrentAttribute().bgColor.color_true.g = static_cast<unsigned char>(std::stoul(std::wstring(*itr)));
				++itr;
				m_document.EditCurrentAttribute().bgColor.color_true.b = static_cast<unsigned char>(std::stoul(std::wstring(*itr)));
				break;
			}
			default:
				break;
			}
			continue;
		}
		case 49:
			m_document.EditCurrentAttribute().bgColor = m_document.GetDefaultAttribute().bgColor;
			continue;
		default:
			break;
		}
		if (num>=10&&num<=19) {
			m_document.EditCurrentAttribute().font = num - 10;
			continue;
		}
		if ((num >= 30 && num <= 37)||(num >= 90 && num <= 97)) {
			m_document.EditCurrentAttribute().frColor.type = ColorType::ColorSystem;
			m_document.EditCurrentAttribute().frColor.color_system= static_cast<unsigned char>(num);
			continue;
		}
		if ((num >= 40 && num <= 47)||( num >= 100 && num <= 107)) {
			m_document.EditCurrentAttribute().bgColor.type = ColorType::ColorSystem;
			m_document.EditCurrentAttribute().bgColor.color_system = static_cast<unsigned char>(num);
			continue;
		}
	}
}
void BasicShellContext::FindString(std::wstring_view str) {
	/*if (m_attr_updated) {
		m_text.back().push_back(CreateAttrText(u"",m_current_attr));
		m_attr_updated = false;
	}*/
	m_document.Insert(std::wstring(str));
}
void BasicShellContext::FindCSI(std::wstring_view sv) {
	switch (sv.back())
	{
	case L'A'://cursor up
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_document.MoveCursorYDown(1);
			break;
		}
		m_document.MoveCursorYDown(std::stoul(std::wstring{ sv }));
		break;
	case L'B'://cursor down
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_document.MoveCursorYUp(1);
			break;
		}
		m_document.MoveCursorYUp(std::stoul(std::wstring{ sv }));
		break;
	case L'C'://cursor right
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_document.MoveCursorX(1);
			break;
		}
		m_document.MoveCursorX(std::stoi(std::wstring{ sv }));
		break;
	case L'D'://curosor left
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_document.MoveCursorX(-1);
			break;
		}
		m_document.MoveCursorX(-std::stoi(std::wstring{ sv }));
		break;
	case L'E':
		m_document.SetCursorX(0);
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_document.MoveCursorYDown(1);
			break;
		}
		m_document.MoveCursorYDown(std::stoul(std::wstring{ sv }));
		break;
	case L'F':
		m_document.SetCursorX(0);
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_document.MoveCursorYUp(1);
			break;
		}
		m_document.MoveCursorYDown(std::stoul(std::wstring{ sv }));
		break;
	case L'G':
		sv.remove_suffix(1);
		m_document.SetCursorX(std::stoul(std::wstring( sv ))-1);
		break;
	case L'H':
	case L'f':
	{
		sv.remove_suffix(1);
		auto vec = split<wchar_t, std::vector<std::wstring>>(std::wstring( sv ), L";");
		if (vec[0].empty()) {
			m_document.SetCursorY(0);
		}
		else {
			m_document.SetCursorY(std::stoul(vec[0]) - 1);
		}
		if (vec[1].empty()) {
			m_document.SetCursorX(0);
		}
		else {
			m_document.SetCursorX(std::stoul(vec[1]) - 1);
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
			m_document.RemoveAfter();
			break;
		case 1:
			m_document.RemoveBefore();
			break;
		case 2:
		{
			m_document.Remove();
			break;
		}
		case 3:
			m_document.RemoveAll();
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
			m_document.RemoveLineAfter();
			break;
		case 1:
			m_document.RemoveLineBefore();
			break;
		case 2:
			m_document.RemoveLine();
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
	case L's':
		m_document.SaveCursor();
		break;
	case L'u':
		m_document.RestoreCursor();
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
	m_document.MoveCursorX(-1);
}
void BasicShellContext::FindFF() {
	m_document.RemoveAll();
}