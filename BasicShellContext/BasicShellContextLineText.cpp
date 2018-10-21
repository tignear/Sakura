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
			r += EastAsianWidth(uc, m_ambiguous_size);
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
			r += EastAsianWidth(uc, m_ambiguous_size);
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
	if (m_value.empty()) {
		m_value.emplace_back(ustr, m_ct_sys, m_ct_256, m_fontmap, attr);
		return EastAsianWidth(ustr, m_ambiguous_size);
	}
	auto insertpos=EAWtoIndexMulti(m_value,p,m_ambiguous_size);
	auto ustrEAW =EastAsianWidth(ustr, m_ambiguous_size);
	auto ret = std::get<1>(insertpos)+ustrEAW;
	auto itr = std::get<2>(insertpos);
	auto insertStart = std::get<3>(insertpos);
	const auto enditr = m_value.end();
	
	if (itr == enditr) {
		--itr;
	}
	if (itr->attribute()==attr) {
		auto& target = itr->ustr();
		auto len = ustr.length();
		target.insert(insertStart, ustr);
		Erase(p+ustrEAW,ustrEAW);
		return ret;
	}
	++itr;
	if (itr == enditr) {
		m_value.emplace_back(ustr, m_ct_sys, m_ct_256, m_fontmap, attr);
		return ret;
	}
	if (insertStart == itr->ustr().length()&& itr->attribute() == attr) {
		auto& target = itr->ustr();
		target.insert(0,ustr);
		Erase(p + ustrEAW, ustrEAW);
		return ret;
	}
	itr=m_value.emplace(itr, ustr, m_ct_sys, m_ct_256, m_fontmap, attr);
	Erase(p + ustrEAW, ustrEAW);

	return ret;
}
using tignear::sakura::ShellContext;
std::tuple<size_t, size_t, std::list<AttributeTextImpl>::iterator, int32_t,uint32_t> BasicShellContextLineText::EAWtoIndexMulti(std::list<AttributeTextImpl>& attrtexts, size_t eaw, unsigned char ambiguous_size) {
	size_t cnt = 0U;
	size_t eawCnt = 0U;
	auto enditr = std::end(attrtexts);
	for (auto itr = std::begin(attrtexts); itr != enditr;++itr) {
		auto r = EAWtoIndex(itr->ustr() ,static_cast<uint32_t>(std::min<size_t>( eaw-eawCnt,std::numeric_limits<uint32_t>::max())), ambiguous_size);
		auto n_eawCnt = r.second + eawCnt;
		auto n_cnt = r.first + cnt;
		if (n_eawCnt >=eaw) {
			return { n_cnt ,n_eawCnt,itr,r.first,r.second };
		}
		eawCnt = n_eawCnt;
		cnt = n_cnt;
	}
	return { cnt,eawCnt,enditr,0,0 };
}
size_t BasicShellContextLineText::Erase(size_t ps,size_t lenEAW) {
	auto rms=EAWtoIndexMulti(m_value, ps, m_ambiguous_size);
	auto rme= EAWtoIndexMulti(m_value, lenEAW+ps, m_ambiguous_size);
	auto rmsitr = std::get<2>(rms);
	auto enditr = m_value.end();
	if (rmsitr == enditr) {
		return ps;
	}
	auto rmeitr = std::get<2>(rme);
	if (rmsitr==rmeitr) {
		rmsitr->ustr().removeBetween(std::get<3>(rms), std::get<3>(rme));
		return ps;
	}
	if (rmsitr->ustr().length() != std::get<3>(rms)){
		rmsitr->ustr().removeBetween(std::get<3>(rms),rmsitr->ustr().length());
	}
	++rmsitr;

	if (rmeitr!=enditr&&rmeitr->ustr().length() != std::get<3>(rme)) {
		rmeitr->ustr().removeBetween(0, std::get<3>(rme));
		--rmeitr;
	}

	m_value.erase(rmsitr, rmeitr);
	return ps;

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