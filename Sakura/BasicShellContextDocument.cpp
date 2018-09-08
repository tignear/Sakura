#include "stdafx.h"
#include <memory>
#include <stdexcept>
#include <algorithm>
#include "BasicShellContextDocument.h"
#include "Looper.h"
#include "split.h"
namespace tignear::sakura {
	using stdex::For;
	void BasicShellContextDocument::NotifyLayoutChange() {
		m_layout_change_callback();
	}
	void BasicShellContextDocument::NotifyTextChange() {
		m_text_change_callback();
	}
	bool BasicShellContextDocument::FixCursorY() {
		if (m_cursorY_itr == m_text.end()) {
			MoveCursorYUp(1);
			if (m_cursorY_itr == m_text.end()) {
				return false;
			}
		}
		return true;
	}
	void BasicShellContextDocument::SetSystemColorTable(const ColorTable& t) {
		m_color_sys = t;
	}
	void BasicShellContextDocument::SetSystemColorTable(const ColorTable&& t) {
		m_color_sys = t;
	}
	void BasicShellContextDocument::Set256ColorTable(const ColorTable& t) {
		m_color_256 = t;
	}
	void BasicShellContextDocument::Set256ColorTable(const ColorTable&& t) {
		m_color_256 = t;
	}
	void BasicShellContextDocument::SetCursorXY(int32_t x, size_t y) {
		SetCursorX(x);
		SetCursorY(y);
	}
	void BasicShellContextDocument::SetCursorX(int32_t x) {
		m_cursorX = x;
		NotifyLayoutChange();
	}
	void BasicShellContextDocument::SetCursorY(size_t y) {
		m_cursorY =0;
		m_cursorY_itr = m_text.begin();
		MoveCursorYDown(y);
	}
	void BasicShellContextDocument::MoveCursorX(int32_t x) {
		m_cursorX += x;
		NotifyLayoutChange();
	}
	void BasicShellContextDocument::MoveCursorYUp(size_t y) {
		for (auto i = 0_z; i < y&&m_cursorY_itr!=m_text.begin(); ++i) {
			--m_cursorY_itr;
			--m_cursorY;
		}
		NotifyLayoutChange();
	}
	void BasicShellContextDocument::MoveCursorYDown(size_t y) {
		for (auto i = 0_z; i < y; ++i) {
			++m_cursorY_itr;
			++m_cursorY;

			if (m_cursorY_itr == m_text.end()) {
				m_text.emplace_back(m_color_sys,m_color_256,m_fontmap);
				--m_cursorY_itr;
				if (m_text.size() > m_max_line) {
					m_text.pop_front();
				}
			}
		}
		if (m_cursorY  > m_viewcount) {
			m_viewend_itr = m_text.end();
			m_viewendpos = m_text.size();
		}
		NotifyLayoutChange();
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
		NotifyLayoutChange();

	}
	void BasicShellContextDocument::ViewPositionUp(size_t count) {
		for (auto i = 0_z; i < count&&m_viewend_itr != m_text.begin();++i) {
			--m_viewend_itr;
			--m_viewendpos;
		}
		NotifyLayoutChange();
	}
	void BasicShellContextDocument::ViewPositionDown(size_t count) {
		for (auto i = 0_z; i < count&&m_viewend_itr != m_text.end(); ++i) {

			++m_viewend_itr;
			++m_viewendpos;
		}
		NotifyLayoutChange();
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
		NotifyTextChange();
	}
	void BasicShellContextDocument::RemoveLineAfter() {
		if (!FixCursorY()) {
			return;
		}
		m_cursorY_itr->RemoveAfter(m_cursorX);
		NotifyTextChange();
	}
	void BasicShellContextDocument::RemoveLine() {
		if (!FixCursorY()) {
			return;
		}
		m_cursorY_itr->Remove();
		NotifyTextChange();
	}
	void BasicShellContextDocument::RemoveAll() {
		m_text.clear();
		//m_text.resize(0);
		m_cursorY_itr  = m_text.begin();
		m_viewend_itr=m_text.end();
		m_cursorY   =m_viewendpos= 0;
		m_cursorX = 0;
		NotifyLayoutChange();
		NotifyTextChange();
	}
	void BasicShellContextDocument::Remove() {
		for (auto itr = m_text.begin(); itr != m_text.end();++itr) {
			itr->Remove();
		}
		NotifyTextChange();
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
		if (m_cursorY_itr == m_text.end()) {
			m_text.emplace_back(m_color_sys,m_color_256,m_fontmap);
			--m_cursorY_itr;
			if (m_text.size() > m_max_line) {
				m_text.pop_front();
			}
		}
		for (auto&& e : r) {
			e += L"\n";
			m_cursorY_itr->Insert(m_cursorX,icu::UnicodeString(e.c_str()),m_current_attr);
			MoveCursorYDown(1);
			SetCursorX(0);
		}
		SetCursorX(m_cursorY_itr->Insert(m_cursorX, icu::UnicodeString(back.c_str()), m_current_attr));
		NotifyTextChange();
	}
	ShellContext::attrtext_line_iterator BasicShellContextDocument::begin()const {
		auto sitr = m_viewend_itr;
		for (auto i = 0_z; i<m_viewcount &&sitr != m_text.begin();++i,--sitr) {

		}
		return ShellContext::attrtext_line_iterator(std::make_unique<attrtext_line_iterator_impl>(sitr));
	}
	ShellContext::attrtext_line_iterator BasicShellContextDocument::end()const {
		return ShellContext::attrtext_line_iterator(std::make_unique<attrtext_line_iterator_impl>(m_viewend_itr));

	}


	void attrtext_line_iterator_impl::operator++() {
		m_elem++;
	}
	attrtext_line_iterator_impl* attrtext_line_iterator_impl::operator++(int) {
		auto temp = clone();
		operator++();
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
}