#include "stdafx.h"
#include <unicode/uiter.h>
#include "BasicShellContextLineText.h"
#include "EastAsianWidth.h"
#include "Looper.h"
using tignear::sakura::BasicShellContextLineText;
using tignear::sakura::AttributeTextImpl;
using tignear::icuex::EastAsianWidth;
using tignear::sakura::Attribute;
const std::list<AttributeTextImpl>& BasicShellContextLineText::Value()const{
	if (m_value.empty()) {
		return empty;
	}
	return m_value;
}
int32_t  BasicShellContextLineText::Remove() {
	m_value.clear();
	return 0;
}
int32_t BasicShellContextLineText::RemoveAfter(int32_t p) {
	unsigned char ambiguous_size = UINT8_C(2);//MAGIC_NUMBER
	uint32_t r=0;
	for (auto itr = m_value.begin(); itr != m_value.end();++itr) {
		auto charitr = icu::StringCharacterIterator(itr->ustr());
		int32_t cnt=0;
		for (UChar uc = charitr.first(); uc != charitr.DONE; uc = charitr.next()) {
			++cnt;
			if ((u_getIntPropertyValue(uc, UCHAR_CANONICAL_COMBINING_CLASS) != 0)) {
				charitr.next();
				continue;
			}
			r += EastAsianWidth(uc, ambiguous_size);
			if (r >= static_cast<unsigned>(p)) {
				itr->ustr().removeBetween(cnt);
				++itr;
				m_value.erase(itr, m_value.end());
				return p;
			}
		}
	}
	return p;
}

int32_t BasicShellContextLineText::RemoveBefore(int32_t p) {
	unsigned char ambiguous_size = UINT8_C(2);//MAGIC_NUMBER
	uint32_t r = 0;
	for (auto itr = m_value.begin(); itr != m_value.end();) {
		auto charitr = icu::StringCharacterIterator(itr->ustr());
		auto cnt = 0;
		for (UChar uc = charitr.first(); uc != charitr.DONE; uc = charitr.next()) {
			++cnt;
			if ((u_getIntPropertyValue(uc, UCHAR_CANONICAL_COMBINING_CLASS) != 0)) {
				charitr.next();
				continue;
			}
			r += EastAsianWidth(uc, ambiguous_size);
			if (r >= static_cast<unsigned>(p)) {
				itr->ustr().removeBetween(0,cnt);
				return p;
			}
		}
		itr=m_value.erase(itr);
	}
	return p;
}

int32_t BasicShellContextLineText::Insert(int32_t p,const icu::UnicodeString& ustr, const Attribute& attr) {
	/*if (ustr.isEmpty()) {
		return;
	}*/
	unsigned char ambiguous_size = UINT8_C(2);//MAGIC_NUMBER
	auto eaw = EastAsianWidth(ustr, ambiguous_size);
	auto r2 = p + eaw;
	auto itr = m_value.begin();
	if (itr == m_value.end()) {
		m_value.emplace_back(ustr, m_ct_sys, m_ct_256,m_fontmap, attr);
		return r2;
	}
	uint32_t r = 0;
	while (itr != m_value.end()) {
		if (itr->ustr().isEmpty()) {
			itr=m_value.erase(itr);
			continue;
		}
		auto charitr = icu::StringCharacterIterator(itr->ustr());
		auto cnt = 0;
		for (UChar uc = charitr.first();uc!=charitr.DONE; uc = charitr.next()) {

			if ((u_getCombiningClass(uc) != 0)) {
				charitr.next();
				++cnt;
				continue;
			}
			if (r >= static_cast<unsigned>(p)) {
				if (itr->attribute() == attr) {
					itr->ustr().insert(cnt, ustr);
					return r2;
				}
				if (r == static_cast<unsigned>(p)) {
					auto n=std::next(itr);
					if (n != m_value.end()) {
						if (n->attribute() == attr) {
							n->ustr().insert(0, ustr);
							return r2;
						}
					}
					m_value.emplace(n, ustr, m_ct_sys, m_ct_256, m_fontmap, attr);
					return r2;
				}
				else {
					auto n = std::next(itr);
					m_value.emplace(n, ustr, m_ct_sys, m_ct_256, m_fontmap,attr);
					if (n != m_value.end()) {
						n=std::next(itr);
					}
					m_value.emplace(n,icu::UnicodeString(itr->ustr(),cnt),m_ct_sys,m_ct_256,m_fontmap,attr);
					itr->ustr().removeBetween(cnt);
				}
				return r2;
			}
			r += EastAsianWidth(uc, ambiguous_size);
			++cnt;
		}
		++itr;
	}
	m_value.emplace_back(ustr, m_ct_sys, m_ct_256, m_fontmap,attr);
	return r2;
}

using tignear::sakura::ShellContext;
ShellContext::attrtext_iterator BasicShellContextLineText::begin() const{
	return ShellContext::attrtext_iterator(std::make_unique<attrtext_iterator_impl>(Value().begin()));
}
ShellContext::attrtext_iterator BasicShellContextLineText::end() const{
	return ShellContext::attrtext_iterator(std::make_unique<attrtext_iterator_impl>(Value().end()));
}
//
using tignear::sakura::ColorTable;
const std::list<AttributeTextImpl> BasicShellContextLineText::empty{ AttributeTextImpl(L"", ColorTable(), ColorTable(),std::vector<std::wstring>()) };
using tignear::sakura::attrtext_iterator_impl;
void attrtext_iterator_impl::operator++() {
	++m_elem;
	return;
}

attrtext_iterator_impl* attrtext_iterator_impl::operator++(int) {
	auto self = clone();
	operator++();
	return self;
}

attrtext_iterator_impl::reference attrtext_iterator_impl::operator*()const {
	return *m_elem;
}
attrtext_iterator_impl::pointer attrtext_iterator_impl::operator->()const {
	return m_elem.operator->();
}
bool attrtext_iterator_impl::operator==(const attrtext_iterator_innner& iterator)const {
	auto obj = dynamic_cast<const attrtext_iterator_impl*>(&iterator);
	if (obj == nullptr) {
		return false;
	}
	return obj->m_elem == m_elem;
}
bool attrtext_iterator_impl::operator!=(const attrtext_iterator_innner& iterator)const {
	return !operator==(iterator);
}
attrtext_iterator_impl* attrtext_iterator_impl::clone()const {
	return new attrtext_iterator_impl(*this);
}
attrtext_iterator_impl::attrtext_iterator_impl(const attrtext_iterator_impl& from)
{
	m_elem = from.m_elem;
}