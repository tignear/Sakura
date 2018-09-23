#include "stdafx.h"
#include <unicode/uiter.h>
#include <algorithm>
#include <ansi/BasicColorTable.h>
#include "BasicShellContextLineText.h"
#include "EastAsianWidth.h"
#include "Looper.h"
#undef min
using tignear::sakura::BasicShellContextLineText;
using tignear::sakura::AttributeTextImpl;
using tignear::icuex::EastAsianWidth;
using tignear::sakura::Attribute;
std::list<AttributeTextImpl>& BasicShellContextLineText::Value(){
	if (m_value.empty()) {
		return empty;
	}
	return m_value;
}
const std::list<AttributeTextImpl>& BasicShellContextLineText::Value()const{
	if (m_value.empty()) {
		return empty;
	}
	return m_value;
}
size_t  BasicShellContextLineText::Remove() {
	m_value.clear();
	return 0;
}
size_t BasicShellContextLineText::RemoveAfter(size_t p) {
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

size_t BasicShellContextLineText::RemoveBefore(size_t p) {
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


size_t BasicShellContextLineText::Insert(size_t p,const icu::UnicodeString& ustr, const Attribute& attr) {
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
			if (r >= static_cast<unsigned>(p)) {//挿入位置を探し出した
				if (itr->attribute() == attr) {//属性がおんなじ場合
					if (ustr.length() > itr->ustr().length()+cnt) {//新しい文字列のほうが長いから古い文字列を新しい文字のぶん消す
						itr->ustr().replace(cnt, itr->ustr().length(), ustr);
						auto removing = ustr.length() - itr->ustr().length()+cnt;
						while (removing>0) {
							++itr;
							if (itr == m_value.end()) {
								break;
							}
							auto nr = std::min(removing, itr->ustr().length());
							itr->ustr().remove(0, nr);
							removing -= nr;
						}
					}
					else {
						itr->ustr().replace(cnt, ustr.length(), ustr);
					}
					return r2;
				}
				if (charitr.getIndex()==0||charitr.getIndex()==charitr.getLength()-1) {//境界にあるから次のとこも見てみる
					{
						auto n = std::next(itr);
						if (n != m_value.end()) {
							if (itr->attribute() == attr) {
								if (ustr.length() > n->ustr().length()) {
									n->ustr().replace(0, n->ustr().length(), ustr);
									auto removing = ustr.length() - n->ustr().length();
									while (removing > 0) {
										++n;
										if (n == m_value.end()) {
											break;
										}
										auto nr = std::min(removing, n->ustr().length());
										n->ustr().remove(0, nr);
										removing -= nr;
									}
								}
								else {
									n->ustr().replace(0, ustr.length(), ustr);
								}
								return r2;
							}
						}
					}
					m_value.emplace(itr, ustr, m_ct_sys, m_ct_256, m_fontmap, attr);
					auto removing = ustr.length();
					while (removing > 0) {
						auto nr = std::min(removing, itr->ustr().length());
						itr->ustr().remove(0, nr);
						removing -= nr;
						++itr;
						if (itr== m_value.end()) {
							break;
						}
					}
					return r2;
				}
				else {//割り込む。
					auto n = std::next(itr);
					m_value.emplace(n, ustr, m_ct_sys, m_ct_256, m_fontmap,attr);
					if (n != m_value.end()) {
						n=std::next(itr);
					}
					if (cnt > ustr.length()) {
						m_value.emplace(n, icu::UnicodeString(itr->ustr(), cnt-ustr.length()), m_ct_sys, m_ct_256, m_fontmap, attr);
					}
					itr->ustr().removeBetween(cnt);
					return r2;
				}
			}
			r += EastAsianWidth(uc, ambiguous_size);
			++cnt;
		}
		++itr;
	}
	m_value.emplace_back(ustr, m_ct_sys, m_ct_256, m_fontmap,attr);
	return r2;
}
size_t BasicShellContextLineText::Erase(size_t p,size_t len) {
	unsigned char ambiguous_size = UINT8_C(2);//MAGIC_NUMBER
	auto itr = m_value.begin();
	if (itr == m_value.end()) {
		return p;
	}
	uint32_t r = 0;
	while (itr != m_value.end()) {
		if (itr->ustr().isEmpty()) {
			itr = m_value.erase(itr);
			continue;
		}
		auto charitr = icu::StringCharacterIterator(itr->ustr());
		auto cnt = 0;
		for (UChar uc = charitr.first(); uc != charitr.DONE; uc = charitr.next()) {
			if ((u_getCombiningClass(uc) != 0)) {
				charitr.next();
				++cnt;
				continue;
			}
			if (r >= static_cast<unsigned>(p)) {
				uint32_t removedEAW = 0;
				auto rs = cnt;
				auto rlen = 0;
				for (; uc != charitr.DONE&&removedEAW < len; uc = charitr.next()) {
					++rlen;
					removedEAW += EastAsianWidth(uc, ambiguous_size);
				}
				itr->ustr().remove(rs, rlen);
				++itr;
				if (itr == m_value.end()) {
					return p;
				}
				while (removedEAW <len) {
					rlen = 0;
					auto charitr = icu::StringCharacterIterator(itr->ustr());
					for (UChar uc = charitr.first(); uc != charitr.DONE; uc = charitr.next()) {
						removedEAW += EastAsianWidth(uc, ambiguous_size);
						++rlen;
						if (removedEAW >= len) {
							itr->ustr().remove(0, rlen);
							return p;
						}
					}
					itr=m_value.erase(itr);
					if (itr == m_value.end()) {
						return p;
					}
				}
			}
			r += EastAsianWidth(uc, ambiguous_size);
			++cnt;
		}
		++itr;
	}
	return p;
}
using tignear::sakura::ShellContext;
ShellContext::attrtext_iterator BasicShellContextLineText::begin(){
	return ShellContext::attrtext_iterator(std::make_unique<attrtext_iterator_impl>(Value().begin()));
}
ShellContext::attrtext_iterator BasicShellContextLineText::end(){
	return ShellContext::attrtext_iterator(std::make_unique<attrtext_iterator_impl>(Value().end()));
}
std::shared_ptr<void>& BasicShellContextLineText::resource() {
	return m_resource;
}
bool BasicShellContextLineText::operator==(const attrtext_line& obj)const {
	auto* other = dynamic_cast<const BasicShellContextLineText*>(&obj);
	if (!other) {
		return false;
	}
	return this == other;
}
bool BasicShellContextLineText::operator!=(const attrtext_line& obj)const {
	return !operator==(obj);
}

//
using tignear::ansi::ColorTable;
std::list<AttributeTextImpl> BasicShellContextLineText::empty{ AttributeTextImpl(L"", ColorTable(), ColorTable(),std::vector<std::wstring>()) };
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