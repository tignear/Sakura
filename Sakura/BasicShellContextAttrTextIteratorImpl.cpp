#include "stdafx.h"
#include "BasicShellContext.h"
using tignear::sakura::BasicShellContext;
using tignear::ansi::AttributeText;
void BasicShellContext::attrtext_iterator_impl::operator++() {
	++m_line_itr;
	if (m_line_itr == m_base_itr->end()) {
		++m_base_itr;
		if (m_base_itr == m_base_itr_end) {
			return;
		}
		m_line_itr = m_base_itr->begin();

	}
}
BasicShellContext::attrtext_iterator_impl* BasicShellContext::attrtext_iterator_impl::operator++(int) {
	auto temp = new attrtext_iterator_impl (*this);
	this->operator++();
	return temp;
}

BasicShellContext::attrtext_iterator_impl::reference BasicShellContext::attrtext_iterator_impl::operator*()const {
	return (*m_line_itr);
}
BasicShellContext::attrtext_iterator_impl::pointer BasicShellContext::attrtext_iterator_impl::operator->()const {
	return m_line_itr.operator->();
}
bool BasicShellContext::attrtext_iterator_impl::operator==(const attrtext_iterator_innner& iterator)const {
	auto* pitr=dynamic_cast<decltype(this)>(&iterator);
	if (pitr == nullptr) {
		return false;
	}
	if (m_base_itr != pitr->m_base_itr) {
		return false;
	}
	if (m_base_itr_end != pitr->m_base_itr_end) {
		return false;
	}
	if (m_base_itr == m_base_itr_end) {
		return true;
	}
	return m_line_itr == pitr->m_line_itr;
}
bool BasicShellContext::attrtext_iterator_impl::operator!=(const attrtext_iterator_innner& iterator)const {
	return !operator==(iterator);
}
BasicShellContext::attrtext_iterator_impl* BasicShellContext::attrtext_iterator_impl::clone()const {
	return new attrtext_iterator_impl(*this);
}
BasicShellContext::attrtext_iterator_impl ::attrtext_iterator_impl(const BasicShellContext::attrtext_iterator_impl& from)
{
	m_base_itr = from.m_base_itr;
	m_line_itr = from.m_line_itr;
}
BasicShellContext::attrtext_iterator_impl::attrtext_iterator_impl(std::list<std::list<AttributeText>>::const_iterator base_itr, std::list<std::list<AttributeText>>::const_iterator end_itr, std::list<AttributeText>::const_iterator line_itr) {
	m_base_itr = base_itr;
	m_base_itr_end = end_itr;
	m_line_itr = line_itr;
}
BasicShellContext::attrtext_iterator_impl::attrtext_iterator_impl(std::list<std::list<AttributeText>>::const_iterator base_itr, std::list<std::list<AttributeText>>::const_iterator end_itr) {
	m_base_itr = base_itr;
	m_base_itr_end = end_itr;
	if (m_base_itr == m_base_itr_end) {
		return;
	}
	m_line_itr = m_base_itr->begin();
}