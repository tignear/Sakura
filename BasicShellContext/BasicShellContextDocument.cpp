#include "stdafx.h"
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <unicode/uchar.h>
#include <unicode/schriter.h>
#include "EastAsianWidth.h"
#include "BasicShellContextDocument.h"
#include "Looper.h"
#include "split.h"
namespace tignear::sakura {
	using stdex::For;
	using ansi::ColorTable;

	BasicShellContextDocument::TextUpdateInfoLine BasicShellContextDocument::BuildTextUpdateInfoLine(ShellContext::TextUpdateStatus s, ShellContext::attrtext_line& l) {
		return TextUpdateInfoLine(std::make_unique<TextUpdateInfoImpl>(s,l));
	}

	void BasicShellContextDocument::NotifyLayoutChange(bool x,bool y) {
		if (x || y) {
			m_cursorX_wstringpos_cache_update = true;
		}
		m_layout_change_callback(x,y);

	}
	void BasicShellContextDocument::NotifyTextChange(std::vector<ShellContext::TextUpdateInfoLine> v) {
		m_cursorX_wstringpos_cache_update = true;
		m_text_change_callback(v);
	}
	void BasicShellContextDocument::FixEmptyLine() {
		auto ri = m_text.rbegin();
		auto re=decltype(ri)(m_cursorY_itr);
		//http://izmiz.hateblo.jp/entry/2014/11/01/140233
		while(ri != re) {
			if (ri->IsEmpty()) {
				NotifyTextChange({ BuildTextUpdateInfoLine(ShellContext::TextUpdateStatus::ERASE, *ri) });
				m_text.erase(--(ri.base())); 
			}
			else {
				break;
			}
		}
	}
	bool BasicShellContextDocument::FixCursorY(){
		if (m_text.begin() == m_text.end()) {
			return false;
		}
		if (m_cursorY_itr == m_text.end()) {
			--m_cursorY_itr;
			--m_cursorY;
		}
		return true;
	}
	void BasicShellContextDocument::SetCursorXY(size_t x, size_t y) {
		SetCursorX(x);
		SetCursorY(y);
	}
	void BasicShellContextDocument::SetCursorX(size_t x) {
		m_cursorX = x;
		NotifyLayoutChange(true, false);
	}
	void BasicShellContextDocument::SetCursorY(size_t y) {
		m_cursorY =0;
		m_cursorY_itr = m_text.begin();
		MoveCursorYDown(y);
	}
	void BasicShellContextDocument::MoveCursorX(int32_t x) {
		m_cursorX += x;
		NotifyLayoutChange(true,false);
	}
	void BasicShellContextDocument::MoveCursorYUp(size_t y) {
		for (auto i = 0_z; i < y&&m_cursorY_itr!=m_text.begin(); ++i) {
			--m_cursorY_itr;
			--m_cursorY;
		}
		NotifyLayoutChange(false, true);
	}
	bool BasicShellContextDocument::CreateIfEnd() {
		if (m_cursorY_itr != m_text.end()) {
			return false;
		}
		m_text.emplace_back(m_color_sys, m_color_256, m_fontmap);
		NotifyTextChange(std::vector<TextUpdateInfoLine>{BuildTextUpdateInfoLine(ShellContext::TextUpdateStatus::NEW, m_text.back())});
		--m_cursorY_itr;
		if (m_text.size() > m_max_line) {
			NotifyTextChange(std::vector<TextUpdateInfoLine>{BuildTextUpdateInfoLine(ShellContext::TextUpdateStatus::ERASE, m_text.front())});
			m_text.pop_front();
		}
		return true;
	}
	void BasicShellContextDocument::MoveCursorYDown(size_t y) {

		for (auto i = 0_z; i < y; ++i) {
			++m_cursorY_itr;
			++m_cursorY;
			CreateIfEnd();
		}
		if (m_cursorY  > m_viewcount) {
			m_viewend_itr = m_text.end();
			m_viewendpos = m_text.size();
		}
		NotifyLayoutChange(false,true);
	}
	size_t BasicShellContextDocument::GetLineCount()const {
		return m_text.size();
	}
	void BasicShellContextDocument::SetLineCountMax(size_t max) {
		m_max_line = max;
	}
	size_t BasicShellContextDocument::GetViewCount()const {
		return m_viewcount;
	}
	void BasicShellContextDocument::SetPageSize(size_t count) {
		if (count > m_max_line) {
			throw std::out_of_range("count must be less than or equal to m_max_line.");
		}
		m_viewcount = count;
		NotifyLayoutChange(false,true);

	}
	void BasicShellContextDocument::ViewPositionUp(size_t count) {
		for (auto i = 0_z; i < count&&m_viewend_itr != m_text.begin();++i) {
			--m_viewend_itr;
			--m_viewendpos;
		}
		NotifyLayoutChange(false,true);
	}
	void BasicShellContextDocument::ViewPositionDown(size_t count) {
		for (auto i = 0_z; i < count&&m_viewend_itr != m_text.end(); ++i) {

			++m_viewend_itr;
			++m_viewendpos;
		}
		NotifyLayoutChange(false,true);
	}
	size_t BasicShellContextDocument::GetViewPosition()const {
		if (m_viewendpos > m_viewcount) {
			return m_viewendpos - m_viewcount;
		}
		else {
			return 0;
		}
	}
	void BasicShellContextDocument::SetViewPosition(size_t y) {
		if (GetViewPosition() > y) {
			auto c = GetViewPosition() - y;
			ViewPositionUp(c);
		}
		else {
			auto c =y- GetViewPosition();
			ViewPositionDown(c);
		}
	}
	const Attribute& BasicShellContextDocument::GetCurrentAttribute()const {
		return m_current_attr;
	}
	const Attribute& BasicShellContextDocument::GetDefaultAttribute()const {
		return m_default_attr;
	}
	Attribute& BasicShellContextDocument::EditCurrentAttribute() {
		return m_current_attr;
	}
	Attribute& BasicShellContextDocument::EditDefaultAttribute() {
		return m_default_attr;
	}
	void BasicShellContextDocument::SetAttriuteDefault() {
		m_current_attr = m_default_attr;
	}
	void BasicShellContextDocument::RemoveLineBefore() {
		if (!FixCursorY()) {
			return;
		}
		m_cursorY_itr->RemoveBefore(m_cursorX);
		NotifyTextChange(std::vector<TextUpdateInfoLine>{BuildTextUpdateInfoLine(ShellContext::TextUpdateStatus::MODIFY, *m_cursorY_itr)});
	}
	void BasicShellContextDocument::RemoveLineAfter() {
		if (!FixCursorY()) {
			return;
		}
		m_cursorY_itr->RemoveAfter(m_cursorX);
		NotifyTextChange(std::vector<TextUpdateInfoLine>{BuildTextUpdateInfoLine(ShellContext::TextUpdateStatus::MODIFY, *m_cursorY_itr)});
	}
	void BasicShellContextDocument::RemoveLine() {
		if (!FixCursorY()) {
			return;
		}
		m_cursorY_itr->Remove();
		NotifyTextChange(std::vector<TextUpdateInfoLine>{BuildTextUpdateInfoLine(ShellContext::TextUpdateStatus::MODIFY, *m_cursorY_itr)});

	}
	void BasicShellContextDocument::RemoveAll() {
		std::vector<TextUpdateInfoLine> vec;
		vec.reserve(m_text.size());
		for (auto&& e : m_text) {
			vec.push_back(BuildTextUpdateInfoLine(ShellContext::TextUpdateStatus::ERASE, e));
		}
		NotifyTextChange(vec);

		m_text.clear();
		//m_text.resize(0);
		m_cursorY_itr  = m_text.begin();
		m_viewend_itr=m_text.end();
		m_cursorY   =m_viewendpos= 0;
		m_cursorX = 0;
		CreateIfEnd();
		NotifyLayoutChange(true,true);
	}
	void BasicShellContextDocument::Remove() {
		std::vector<TextUpdateInfoLine> vec;
		vec.reserve(m_text.size());
		for (auto itr = m_text.begin(); itr != m_text.end();++itr) {
			itr->Remove();
			vec.push_back(BuildTextUpdateInfoLine(ShellContext::TextUpdateStatus::MODIFY, *itr));
		}
		NotifyTextChange(vec);
	}
	void BasicShellContextDocument::RemoveBefore() {
		if (m_text.begin() == m_cursorY_itr) {
			RemoveLineBefore();
			return;
		}
		auto end=std::prev(m_cursorY_itr);
		for (auto itr = m_text.begin(); itr !=end ; ++itr) {
			itr->Remove();
		}
		RemoveLineBefore();
	}
	void BasicShellContextDocument::RemoveAfter() {
		if (m_text.end() == m_cursorY_itr) {
			return;
		}
		for (auto itr = std::next(m_cursorY_itr); itr != m_text.end(); ++itr) {
			itr->Remove();
		}
		RemoveLineAfter();
	}
	void BasicShellContextDocument::SaveCursor() {
		m_curorX_save = m_cursorX;
		m_cursorY_itr_save = m_cursorY_itr;
		m_cursorY_save = m_cursorY;
	}
	void BasicShellContextDocument::RestoreCursor() {
		 m_cursorX= m_curorX_save;
		 m_cursorY_itr=m_cursorY_itr_save;
		 m_cursorY =m_cursorY_save;
	}
	void BasicShellContextDocument::Insert(const std::wstring& wstr) {
		using tignear::stdex::split;
		auto r=split<wchar_t,std::vector<std::wstring>>(wstr,L"\n");

		auto back = r.back();
		r.pop_back();
		CreateIfEnd();
		for (auto&& e : r) {
			//e += L"\n";
			if (!e.empty()&&L'\r'==e.back()){
				e.pop_back();
			}
			m_cursorY_itr->Insert(m_cursorX,icu::UnicodeString(e.c_str()),m_current_attr);
			NotifyTextChange(std::vector<TextUpdateInfoLine>{BuildTextUpdateInfoLine(ShellContext::TextUpdateStatus::MODIFY, *m_cursorY_itr)});
			MoveCursorYDown(1);
			SetCursorX(0);
		}
		SetCursorX(m_cursorY_itr->Insert(m_cursorX, icu::UnicodeString(back.c_str()), m_current_attr));
		NotifyTextChange(std::vector<TextUpdateInfoLine>{BuildTextUpdateInfoLine(ShellContext::TextUpdateStatus::MODIFY, *m_cursorY_itr)});
		FixEmptyLine();

	}
	void BasicShellContextDocument::Erase(size_t count) {
		m_cursorY_itr->Erase(m_cursorX, count);//ignor
	}
	attrtext_document_all_impl& BasicShellContextDocument::GetAll() {
		return m_all;
	}
	attrtext_document_view_impl& BasicShellContextDocument::GetView() {
		return m_view;
	}
	size_t  BasicShellContextDocument::GetCursorX()const {
		return m_cursorX;
	}
	size_t BasicShellContextDocument::CalcWStringPosCache()const {
		const auto& l =decltype(m_text)::const_iterator(m_cursorY_itr)->Value();

		size_t cnt=0;
		size_t nowEAW = 0;
		auto end = l.cend();
		for (auto itr = l.cbegin(); itr != end&&nowEAW<m_cursorX;++itr) {
			auto charitr = icu::StringCharacterIterator(itr->ustr());
			for (UChar uc = charitr.first(); uc != charitr.DONE&&nowEAW < m_cursorX; uc = charitr.next()) {
				if ((u_getCombiningClass(uc) != 0)) {
					charitr.next();
					++cnt;
					continue;
				}
				++cnt;
				nowEAW += icuex::EastAsianWidth(uc,2);//MAGIC_NUMBER
			}
		}
		return cnt;
	}
	size_t  BasicShellContextDocument::GetCursorXWStringPos()const {
		if (m_cursorX_wstringpos_cache_update) {
			m_cursorX_wstringpos_cache=CalcWStringPosCache();
			m_cursorX_wstringpos_cache_update = false;
		}
		return static_cast<size_t>(m_cursorX_wstringpos_cache);
	}
	BasicShellContextLineText&  BasicShellContextDocument::GetCursorY(){
		if (m_cursorY_itr == m_text.end()) {
			return *std::prev(m_cursorY_itr);
		}
		return *m_cursorY_itr;
	}
	ShellContext::attrtext_line_iterator  BasicShellContextDocument::GetCursorYItr() {
		return ShellContext::attrtext_line_iterator(std::make_unique<attrtext_line_iterator_impl>(m_cursorY_itr == m_text.end() ?
			std::prev(m_cursorY_itr) : m_cursorY_itr));
	}
	void attrtext_line_iterator_impl::operator++() {
		++m_elem;
	}
	void attrtext_line_iterator_impl::operator--() {
		--m_elem;
	}

	attrtext_line_iterator_impl* attrtext_line_iterator_impl::operator++(int) {
		auto temp = clone();
		operator++();
		return temp;
	}
	attrtext_line_iterator_impl* attrtext_line_iterator_impl::operator--(int) {
		auto temp = clone();
		operator--();
		return temp;
	}

	attrtext_line_iterator_impl::reference attrtext_line_iterator_impl::operator*()const {
		return (*m_elem);
	}
	attrtext_line_iterator_impl::pointer attrtext_line_iterator_impl::operator->()const {
		auto r=m_elem.operator->();
		return r;
	}
	bool attrtext_line_iterator_impl::operator==(const attrtext_line_iterator_inner& iterator)const {
		auto r=dynamic_cast<const attrtext_line_iterator_impl*>(&iterator);
		if (r == nullptr) {
			return false;
		}
		return r->m_elem == m_elem;
	}
	bool attrtext_line_iterator_impl::operator!=(const attrtext_line_iterator_inner& iterator)const {
		return !operator==(iterator);
	}
	attrtext_line_iterator_impl* attrtext_line_iterator_impl::clone()const {
		return new attrtext_line_iterator_impl(*this);
	}
	attrtext_line_iterator_impl::attrtext_line_iterator_impl(const attrtext_line_iterator_impl& from) {
		m_elem = from.m_elem;
	}
	ShellContext::attrtext_line_iterator attrtext_document_all_impl::end() {
		return ShellContext::attrtext_line_iterator(std::make_unique<attrtext_line_iterator_impl>(m_text.end()));
	}
	ShellContext::attrtext_line_iterator attrtext_document_all_impl::begin() {
		return  ShellContext::attrtext_line_iterator(std::make_unique<attrtext_line_iterator_impl>(m_text.begin()));
	}
	ShellContext::attrtext_line_iterator attrtext_document_view_impl::end() {
		return ShellContext::attrtext_line_iterator(std::make_unique<attrtext_line_iterator_impl>(m_viewend_itr));
	}
	ShellContext::attrtext_line_iterator attrtext_document_view_impl::begin() {
		auto cp = m_viewend_itr;
		for (auto i = 0_z; i < m_viewcount&&cp!=m_text.begin();++i) {
			--cp;
		}
		return ShellContext::attrtext_line_iterator(std::make_unique<attrtext_line_iterator_impl>(cp));
	}
	ShellContext::TextUpdateStatus TextUpdateInfoImpl::status()const {
		return m_status;
	}
	ShellContext::attrtext_line& TextUpdateInfoImpl::line() {
		return m_line;
	}
	TextUpdateInfoImpl* TextUpdateInfoImpl::clone()const {
		return new TextUpdateInfoImpl(*this);
	}
}