#include "stdafx.h"
#include "BasicShellContextDocument.h"
using tignear::ansi::AttributeText;
using tignear::sakura::attrtext_iterator_impl;
void attrtext_iterator_impl::operator++() {
	++m_line;
	if (m_main->Value().end() == m_line) {
		++m_main;
		if (m_main == m_main_end) {
			return;
		}
		m_line = m_main->Value().begin();
	}
	return;
}

attrtext_iterator_impl* attrtext_iterator_impl::operator++(int) {
	auto self = clone();
	operator++();
	return self;
}

attrtext_iterator_impl::reference attrtext_iterator_impl::operator*()const {
	return *m_line;
}
attrtext_iterator_impl::pointer attrtext_iterator_impl::operator->()const {
	return m_line.operator->();
}
bool attrtext_iterator_impl::operator==(const attrtext_iterator_innner& iterator)const {
	auto obj=dynamic_cast<const attrtext_iterator_impl*>(&iterator);
	if (obj == nullptr) {
		return false;
	}
	if (obj->m_main != m_main) {
		return false;
	}
	if (m_main_end != obj->m_main_end) {
		return false;
	}
	if (m_main == m_main_end) {
		return true;
	}
	if (obj->m_line != m_line) {
		return false;
	}
	return true;
}
bool attrtext_iterator_impl::operator!=(const attrtext_iterator_innner& iterator)const {
	return !operator==(iterator);
}
attrtext_iterator_impl* attrtext_iterator_impl::clone()const {
	return new attrtext_iterator_impl(*this);
}
attrtext_iterator_impl::attrtext_iterator_impl(const attrtext_iterator_impl& from)
{
	m_line = from.m_line;
	m_main = from.m_main;
	m_main_end = from.m_main_end;
}