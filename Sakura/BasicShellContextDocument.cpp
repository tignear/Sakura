#include "stdafx.h"
#include <memory>
#include "BasicShellContextDocument.h"
#include "Looper.h"
#include "split.h"
namespace tignear::sakura {
	using stdex::For;
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
	}
	void BasicShellContextDocument::SetCursorY(size_t y) {
		m_cursorY = m_origin;
		m_cursorY_itr = m_origin_itr;
		MoveCursorYDown(y);
	}
	void BasicShellContextDocument::MoveCursorX(int32_t x) {
		m_cursorX += x;
	}
	void BasicShellContextDocument::MoveCursorYUp(size_t y) {
		for (auto i = 0_z; i < y&&m_cursorY_itr!=m_text.begin(); ++i) {
			--m_cursorY_itr;
			--m_cursorY;
		}
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
	void BasicShellContextDocument::SetViewCount(size_t count) {
		if (m_viewcount>count) {
			auto c = m_viewcount - count;
			For(c, [this]() {++m_viewpos_itr; ++m_viewpos; ++m_origin_itr; ++m_origin; });
		}
		else {
			auto c =count- m_viewcount;
			For(c, [this]() {--m_viewend_itr; });
			for (auto i = 0_z; i < c&&m_viewend_itr != m_text.end();++i) {
				++m_viewend_itr;
			}
		}
		m_viewcount = count;
	}
	void BasicShellContextDocument::ViewPositionUp(size_t count) {
		for (auto i = 0_z; i < count&&m_viewpos_itr != m_text.begin();++i) {
			--m_viewpos;
			--m_viewpos_itr;
			--m_viewend_itr;
		}
	}
	void BasicShellContextDocument::ViewPositionDown(size_t count) {
		for (auto i = 0_z; i < count&&m_viewend_itr != m_text.end(); ++i) {
			++m_viewpos;
			++m_viewpos_itr;
			++m_viewend_itr;
		}
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
	}
	void BasicShellContextDocument::RemoveLineAfter() {
		if (!FixCursorY()) {
			return;
		}
		m_cursorY_itr->RemoveAfter(m_cursorX);
	}
	void BasicShellContextDocument::RemoveLine() {
		if (!FixCursorY()) {
			return;
		}
		m_cursorY_itr->Remove();
	}
	void BasicShellContextDocument::RemoveAll() {
		m_text.clear();
	}
	void BasicShellContextDocument::Remove() {
		for (auto itr = m_origin_itr; itr != m_text.end();++itr) {
			itr->Remove();
		}
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
		if (m_viewpos_itr == m_text.end()) {
			m_viewpos_itr = m_text.begin();
		}
	}
	ShellContext::attrtext_iterator BasicShellContextDocument::begin()const {

		return ShellContext::attrtext_iterator(std::make_unique<attrtext_iterator_impl>(m_viewpos_itr,m_viewend_itr));
	}
	ShellContext::attrtext_iterator BasicShellContextDocument::end()const {
		return ShellContext::attrtext_iterator(std::make_unique<attrtext_iterator_impl>(m_viewend_itr, m_viewend_itr));

	}
}