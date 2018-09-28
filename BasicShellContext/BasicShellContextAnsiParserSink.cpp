#include "stdafx.h"
#include <unicode/ubrk.h>
#include <unicode/brkiter.h>
#include <unicode/locid.h>
#include <strconv.h>
#include <numeric>
#include <cassert>
#include <algorithm>
#include <split.h>
#include "BasicShellContext.h"
#include "EastAsianWidth.h"
using tignear::sakura::BasicShellContext;
using tignear::ansi::AttributeText;
using tignear::stdex::split;
using icu::UnicodeString;
using tignear::icuex::EastAsianWidth;

void BasicShellContext::ParseColor(std::string_view sv) {
	if (sv.empty()) {
		m_document.SetAttriuteDefault();
		return;
	}
	auto elems = split<char, std::vector<std::string>>(std::string(sv), ";");
	try {
		for (auto itr = elems.begin(); itr != elems.end(); ++itr) {
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
				m_document.EditCurrentAttribute().crossed_out = true;
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
				auto t = std::stoul(std::string(*itr));
				switch (t)
				{
				case 5:
				{
					itr++;
					m_document.EditCurrentAttribute().frColor.color_256 = static_cast<unsigned char>(std::stoul(std::string(*itr)));
					m_document.EditCurrentAttribute().frColor.type = ColorType::Color256;
					break;
				}
				case 2:
				{
					m_document.EditCurrentAttribute().frColor.type = ColorType::ColorTrue;
					++itr;
					m_document.EditCurrentAttribute().frColor.color_true.r = static_cast<unsigned char>(std::stoul(std::string(*itr)));
					++itr;
					m_document.EditCurrentAttribute().frColor.color_true.g = static_cast<unsigned char>(std::stoul(std::string(*itr)));
					++itr;
					m_document.EditCurrentAttribute().frColor.color_true.b = static_cast<unsigned char>(std::stoul(std::string(*itr)));
					break;
				}
				default:
					break;
				}
				continue;
			}
			case 39:
				m_document.EditCurrentAttribute().frColor = m_document.GetDefaultAttribute().frColor;
				continue;
			case 48:
			{
				++itr;
				auto t = std::stoul(std::string(*itr));
				switch (t)
				{
				case 5:
				{
					itr++;
					m_document.EditCurrentAttribute().bgColor.color_256 = static_cast<unsigned char>(std::stoul(std::string(*itr)));
					m_document.EditCurrentAttribute().bgColor.type = ColorType::Color256;
					break;
				}
				case 2:
				{
					m_document.EditCurrentAttribute().bgColor.type = ColorType::ColorTrue;
					++itr;
					m_document.EditCurrentAttribute().bgColor.color_true.r = static_cast<unsigned char>(std::stoul(std::string(*itr)));
					++itr;
					m_document.EditCurrentAttribute().bgColor.color_true.g = static_cast<unsigned char>(std::stoul(std::string(*itr)));
					++itr;
					m_document.EditCurrentAttribute().bgColor.color_true.b = static_cast<unsigned char>(std::stoul(std::string(*itr)));
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
			if (num >= 10 && num <= 19) {
				m_document.EditCurrentAttribute().font = num - 10;
				continue;
			}
			if ((num >= 30 && num <= 37) || (num >= 90 && num <= 97)) {
				m_document.EditCurrentAttribute().frColor.type = ColorType::ColorSystem;
				m_document.EditCurrentAttribute().frColor.color_system = static_cast<unsigned char>(num);
				continue;
			}
			if ((num >= 40 && num <= 47) || (num >= 100 && num <= 107)) {
				m_document.EditCurrentAttribute().bgColor.type = ColorType::ColorSystem;
				m_document.EditCurrentAttribute().bgColor.color_system = static_cast<unsigned char>(num);
				continue;
			}
		}
	}catch(...){

	}
}
void BasicShellContext::FindString(std::string_view str) {

	m_document.Insert(cp_to_wide(str,m_codepage));
}
void BasicShellContext::FindCSI(std::string_view sv) {
	switch (sv.back())
	{
	case 'A'://cursor up
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_document.MoveCursorYDown(1);
			break;
		}
		m_document.MoveCursorYDown(std::stoul(std::string{ sv }));
		break;
	case 'B'://cursor down
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_document.MoveCursorYUp(1);
			break;
		}
		m_document.MoveCursorYUp(std::stoul(std::string{ sv }));
		break;
	case 'C'://cursor right
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_document.MoveCursorX(1);
			break;
		}
		m_document.MoveCursorX(std::stoi(std::string{ sv }));
		break;
	case 'D'://curosor left
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_document.MoveCursorX(-1);
			break;
		}
		m_document.MoveCursorX(-std::stoi(std::string{ sv }));
		break;
	case 'E':
		m_document.SetCursorX(0);
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_document.MoveCursorYDown(1);
			break;
		}
		m_document.MoveCursorYDown(std::stoul(std::string{ sv }));
		break;
	case 'F':
		m_document.SetCursorX(0);
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_document.MoveCursorYUp(1);
			break;
		}
		m_document.MoveCursorYDown(std::stoul(std::string{ sv }));
		break;
	case 'G':
		sv.remove_suffix(1);
		m_document.SetCursorX(std::stoul(std::string( sv ))-1);
		break;
	case 'H':
	case 'f':
	{
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_document.SetCursorXY(0, 0);
			break;
		}
		auto vec = split<char, std::vector<std::string>>(std::string( sv ), ";");
		if (vec.size()<2) {
			OutputDebugString(_T("Unsupported Opearation\n"));
			break;
		}
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
	case 'J':
	{
		sv.remove_suffix(1);
		unsigned long prop;
		if (sv.empty()) {
			prop = 0UL;
		}
		else {
			prop = std::stoul(std::string{ sv });
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
	case 'K':
	{
		sv.remove_suffix(1);
		unsigned long prop;
		if (sv.empty()) {
			prop = 0UL;
		}
		else {
			prop = std::stoul(std::string{ sv });
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
		break;
	}
	case 'T':
	case 'U':
		//not implemented
		break;
	case 'm':
		sv.remove_suffix(1);
		ParseColor(sv);
		break;
	case 's':
		m_document.SaveCursor();
		break;
	case 'u':
		m_document.RestoreCursor();
		break;
	case 'X':
	{
		sv.remove_suffix(1);
		size_t prop;
		if (sv.empty()) {
			prop = 0UL;
		}
		else {
			prop = std::stoul(std::string{ sv });
		}
		m_document.Erase(prop);
		break;
	}
	default:
		OutputDebugStringA((std::string("unsupported CSI") + std::string(sv) + "\n").c_str());
		break;
	}
}
void BasicShellContext::FindOSC(std::string_view sv) {

	auto r = split<char, std::vector<std::string>>(std::string(sv), ";");
	switch (std::stoul(r[0]))
	{
	case 0:
		m_title = cp_to_wide(r[1],m_codepage);
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
void BasicShellContext::FindCR() {
	m_document.SetCursorX(0);
}
void BasicShellContext::FindLF() {
	m_document.SetCursorX(0);
	m_document.MoveCursorYDown(1);
}