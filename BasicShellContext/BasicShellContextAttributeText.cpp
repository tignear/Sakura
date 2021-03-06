#include "stdafx.h"
#include "BasicShellContextAttributeText.h"
using tignear::sakura::AttributeTextImpl;
using tignear::ansi::Blink;
using tignear::sakura::Color;
using tignear::sakura::Attribute;
uint32_t AttributeTextImpl::ColorHelper(const Color& color)const {
	return SolveColor(color, m_system_color_table,m_256_color_table);
}
std::wstring_view AttributeTextImpl::textW()const{
	return std::wstring_view(reinterpret_cast<const wchar_t*>(m_ustr.getBuffer()), m_ustr.length());
}
size_t AttributeTextImpl::length()const{
	return m_ustr.length();
}
std::uint32_t AttributeTextImpl::textColor()const{
	if (m_attr.reverse) {
		return ColorHelper(m_attr.bgColor);
	}
	else {
		return ColorHelper(m_attr.frColor);
	}
}
std::uint32_t AttributeTextImpl::backgroundColor()const{
	if (m_attr.reverse) {
		return ColorHelper(m_attr.frColor);
	}
	else {
		return ColorHelper(m_attr.bgColor);
	}
}
bool AttributeTextImpl::bold()const{
	return m_attr.bold;
}
bool AttributeTextImpl::faint()const{
	return m_attr.faint;
}
bool AttributeTextImpl::italic()const{
	return m_attr.italic;
}
bool AttributeTextImpl::underline()const{
	return m_attr.underline;
}
bool AttributeTextImpl::fluktur()const{
	return m_attr.fluktur;
}
Blink AttributeTextImpl::blink()const{
	return m_attr.blink;
}
bool AttributeTextImpl::conceal() const{
	return m_attr.conceal;
}
bool AttributeTextImpl::crossed_out()const{
	return m_attr.crossed_out;
}
const std::wstring& AttributeTextImpl::font()const{
	return m_font_map.at(m_attr.font);
}
const Attribute& AttributeTextImpl::attribute()const {
	return m_attr;
}
const icu::UnicodeString& AttributeTextImpl::ustr()const {
	return m_ustr;
}
icu::UnicodeString& AttributeTextImpl::ustr() {
	return m_ustr;
}