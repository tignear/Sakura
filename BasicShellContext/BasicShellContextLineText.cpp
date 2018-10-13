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

bool BasicShellContextLineText::IsEmpty()const {
	return m_value.empty();
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
			if (r >= static_cast<unsigned>(p)) {//‘}“üˆÊ’u‚ð’T‚µo‚µ‚½
				if (itr->attribute() == attr) {//‘®«‚ª‚¨‚ñ‚È‚¶ê‡
					auto len = itr->ustr().length();
					if (ustr.length() > len+cnt) {//V‚µ‚¢•¶Žš—ñ‚Ì‚Ù‚¤‚ª’·‚¢‚©‚çŒÃ‚¢•¶Žš—ñ‚ðV‚µ‚¢•¶Žš‚Ì‚Ô‚ñÁ‚·
						itr->ustr().replace(cnt, len, ustr);
						auto removing = ustr.length() - (len +cnt);
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
				if (charitr.getIndex()==0||charitr.getIndex()==charitr.getLength()-1) {//‹«ŠE‚É‚ ‚é‚©‚çŽŸ‚Ì‚Æ‚±‚àŒ©‚Ä‚Ý‚é
					{
						auto n = std::next(itr);
						if (n != m_value.end()&& itr->attribute() == attr) {
							auto len = n->ustr().length();
							if (ustr.length() > len) {
								n->ustr().replace(0, len, ustr);
								auto removing = ustr.length() - len;
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
				else {//Š„‚èž‚ÞB
					/*auto n = std::next(itr);
					m_value.emplace(n, ustr, m_ct_sys, m_ct_256, m_fontmap,attr);
					if (n != m_value.end()) {
						n=std::next(itr);
					}
					if (cnt > ustr.length()) {
						m_value.emplace(n, icu::UnicodeString(itr->ustr(), cnt-ustr.length()), m_ct_sys, m_ct_256, m_fontmap, attr);
					}
					itr->ustr().removeBetween(cnt);
					return r2;*/
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
size_t BasicShellContextLineText::Erase(size_t ps,size_t lenEAWs) {
	auto p = static_cast<int32_t>(ps);
	auto lenEAW= static_cast<int32_t>(lenEAWs);
	unsigned char ambiguous_size = UINT8_C(2);//MAGIC_NUMBER
	auto itr = m_value.begin();
	if (itr == m_value.end()) {
		return p;
	}
	auto r = EAWtoIndex(itr->ustr(), p, ambiguous_size);
	auto r2 = EAWtoIndex(itr->ustr(), p +lenEAW, ambiguous_size);
	itr->ustr().removeBetween(r.first,r2.first);
	auto removingEAW = r2.second - p + lenEAW;
	++itr;
	while (removingEAW > 0&&itr != m_value.end()) {
		auto r3 = EAWtoIndex(itr->ustr(), removingEAW, ambiguous_size);
		if (itr->ustr().length() == r3.first) {
			itr = m_value.erase(itr);
		}
		else {
			itr->ustr().replace(0,r3.first,icu::UnicodeString(r3.first,32,r3.first));
			++itr;
		}
		removingEAW -= r3.second;
	}
	return p;
}
using tignear::sakura::ShellContext;
std::pair<int32_t,uint32_t> BasicShellContextLineText::EAWtoIndex(const icu::UnicodeString& ustr, uint32_t eaw,unsigned char ambiguous_size) {
	auto cnt = 0U;
	auto eawCnt = 0U;
	auto charitr = icu::StringCharacterIterator(ustr);
	for (UChar uc = charitr.first(); uc != charitr.DONE&&eawCnt<eaw; uc = charitr.next()) {

		if ((u_getCombiningClass(uc) != 0)) {
			charitr.next();
			++cnt;
			continue;
		}
		eawCnt += EastAsianWidth(uc, ambiguous_size);
		++cnt;
	}
	return { cnt,eawCnt };
}
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

const attrtext_iterator_impl::reference attrtext_iterator_impl::operator*()const {
	return *m_elem;
}
const attrtext_iterator_impl::pointer attrtext_iterator_impl::operator->()const {
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