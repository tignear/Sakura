#include "stdafx.h"
#include <memory>
#include <stdexcept>
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
		m_cursorY = m_origin;
		m_cursorY_itr = m_origin_itr;
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
				m_text.emplace_back(m_color_sys,m_color_256);
				--m_cursorY_itr;
				if (m_text.size() > m_max_line) {
					m_text.pop_front();
				}
			}
		}
		if (m_cursorY - m_origin > m_viewcount) {
			if (m_origin_itr == m_text.end()) {
				m_origin_itr = m_text.begin();
			}
			for (auto i = 0_z; i < m_cursorY - m_origin-m_viewcount; i++) {
				++m_origin;
				++m_origin_itr;
			}
			m_viewpos = m_origin;
			m_viewpos_itr = m_origin_itr;
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
		auto c = m_viewendpos - m_viewpos;
		if (c==count) {
			m_viewcount = count;
			return;
		}
		if (c > count) {
			for (auto i = 0_z; m_viewendpos-m_viewpos != count&&m_viewend_itr != m_viewpos_itr; ++i) {
				++m_viewpos;
				++m_viewpos_itr;
				++m_origin;
				++m_origin_itr;
			}

		}
		else {
			for (auto i = 0_z; m_viewendpos - m_viewpos != count &&m_text.begin() != m_viewpos_itr; ++i) {
					--m_viewpos;
					--m_viewpos_itr;
					--m_origin;
					--m_origin_itr;
				}
			}
		m_viewcount = count;
		NotifyLayoutChange();

	}
	void BasicShellContextDocument::ViewPositionUp(size_t count) {
		for (auto i = 0_z; i < count&&m_viewpos_itr != m_text.begin();++i) {
			--m_viewpos;
			--m_viewpos_itr;
			--m_viewend_itr;
			--m_viewendpos;
		}
		NotifyLayoutChange();
	}
	void BasicShellContextDocument::ViewPositionDown(size_t count) {
		for (auto i = 0_z; i < count&&m_viewend_itr != m_text.end(); ++i) {
			if (m_viewendpos - m_viewpos>m_viewcount) {
				++m_viewpos;
				++m_viewpos_itr;
			}

			++m_viewend_itr;
			++m_viewendpos;
		}
		NotifyLayoutChange();
	}
	size_t BasicShellContextDocument::GetViewPosition()const {
		return m_viewpos;
	}
	void BasicShellContextDocument::SetViewPosition(size_t y) {
		if (m_viewpos > y) {
			auto c = m_viewpos - y;
			ViewPositionUp(c);
		}
		else {
			auto c =y- m_viewpos;
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
		m_cursorY_itr = m_viewpos_itr = m_origin_itr =m_viewend_itr=m_text.begin();
		m_cursorY  = m_origin = m_viewpos =m_viewendpos= 0;
		m_cursorX = 0;
		NotifyTextChange();
	}
	void BasicShellContextDocument::Remove() {
		for (auto itr = m_origin_itr; itr != m_text.end();++itr) {
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
		for (auto itr = m_origin_itr; itr !=end ; ++itr) {
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
		if (m_viewpos_itr == m_text.end()) {
			m_viewpos_itr = m_text.begin();
			m_viewpos = 0;
		}
		if (m_origin_itr == m_text.end()) {
			m_origin_itr = m_text.begin();
			m_origin = 0;
		}
		using tignear::stdex::split;
		auto r=split<wchar_t,std::vector<std::wstring>>(wstr,L"\n");
		auto back = r.back();
		r.pop_back();
		if (m_cursorY_itr == m_text.end()) {
			m_text.emplace_back(m_color_sys,m_color_256);
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
	ShellContext::attrtext_iterator BasicShellContextDocument::begin()const {
		return ShellContext::attrtext_iterator(std::make_unique<attrtext_iterator_impl>(m_viewpos_itr,m_viewend_itr));
	}
	ShellContext::attrtext_iterator BasicShellContextDocument::end()const {
		return ShellContext::attrtext_iterator(std::make_unique<attrtext_iterator_impl>(m_viewend_itr, m_viewend_itr));

	}
}