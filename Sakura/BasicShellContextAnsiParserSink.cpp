#include "stdafx.h"
#include "BasicShellContext.h"
#include "split.h"
using tignear::sakura::BasicShellContext;
using tignear::ansi::AttributeText;
using tignear::ansi::BasicAttributeText;
void BasicShellContext::RemoveRows(size_t count) {
	for (auto i = 0U; i < count; i++) {
		m_text.pop_front();
	}
	m_buffer_rebuild = true;
}
void BasicShellContext::RemoveRowsR(size_t count) {
	for (auto i = 0U; i < count; i++) {
		m_text.pop_back();
	}
	m_buffer_rebuild = true;
}
void BasicShellContext::RemoveColumns(size_t count) {
	for (auto i = 0U; i < count;) {
		
	}
	m_buffer_rebuild = true;
}
void BasicShellContext::FindString(std::wstring_view) {

}
void BasicShellContext::FindCSI(std::wstring_view sv) {
	using tignear::stdex::split;
	switch (sv.back())
	{
	case L'A'://cursor up
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_cursorY -= 1;
			break;
		}
		m_cursorY-=std::stoul(std::wstring{ sv });
		break;
	case L'B'://cursor down
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_cursorY += 1;
			break;
		}
		m_cursorY += std::stoul(std::wstring{ sv });
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
			m_cursorY -= 1;
			break;
		}
		m_cursorY -= std::stoul(std::wstring{ sv });
		break;
	case L'F':
		m_cursorX = 0;
		sv.remove_suffix(1);
		if (sv.empty()) {
			m_cursorY += 1;
			break;
		}
		m_cursorY += std::stoul(std::wstring{ sv });
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
			m_cursorY = 0;
		}
		else {
			m_cursorY = std::stoul(vec[0]) - 1;
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
		{
		}
		case 1:
		case 2:
		default:
			break;
		}
	}

	default:
		break;
	}
}