#pragma once
#include <functional>
#include <list>
#include "ShellContext.h"
#include "BasicShellContextLineText.h"
namespace tignear::sakura {
	class attrtext_iterator_impl :public ShellContext::attrtext_iterator_innner {
	public:
		attrtext_iterator_impl(std::list<BasicShellContextLineText>::const_iterator main,
			std::list<BasicShellContextLineText>::const_iterator main_end) :
			m_main(main),
			m_main_end(main_end) 
		{
			if (main != main_end) {
				auto l =main->Value().begin();
				m_line = l;
			}
		}
		void operator++()override;
		attrtext_iterator_impl* operator++(int) override;
		reference operator*()const override;
		pointer operator->()const override;
		bool operator==(const attrtext_iterator_innner& iterator)const override;
		bool operator!=(const attrtext_iterator_innner& iterator)const override;
		attrtext_iterator_impl* clone()const override;
	private:
		attrtext_iterator_impl(const attrtext_iterator_impl&);
		std::list<BasicShellContextLineText>::const_iterator m_main;
		std::list<BasicShellContextLineText>::const_iterator m_main_end;
		std::list<AttributeTextImpl>::const_iterator m_line;
	};
	class BasicShellContextDocument {
		std::list<BasicShellContextLineText> m_text;
		std::list<BasicShellContextLineText>::iterator m_cursorY_itr;
		std::list<BasicShellContextLineText>::iterator m_cursorY_itr_save;
		std::list<BasicShellContextLineText>::iterator m_viewpos_itr;
		std::list<BasicShellContextLineText>::iterator m_viewend_itr;
		std::list<BasicShellContextLineText>::iterator m_origin_itr;
		size_t m_max_line;
		size_t m_cursorY;
		size_t m_cursorY_save;
		size_t m_viewpos;
		size_t m_origin;
		size_t m_viewcount;
		int32_t m_cursorX;
		int32_t m_curorX_save;
		ColorTable m_color_256;
		ColorTable m_color_sys;
		Attribute m_current_attr;
		Attribute m_default_attr;
		bool FixCursorY();
	public:
		BasicShellContextDocument(
			const ColorTable& color_sys,
			const ColorTable& color_256,
			const Attribute& def) :
			m_color_256(color_256),
			m_color_sys(color_sys),
			m_default_attr(def),
			m_current_attr(def),
			m_max_line(30),
			m_cursorY(0),
			m_cursorY_itr(m_text.begin()),
			m_origin(0),
			m_origin_itr(m_text.begin()),
			m_viewpos(0),
			m_viewpos_itr(m_text.begin()),
			m_viewend_itr(m_text.end()),
			m_cursorX(0)
		{

		}
		void SetSystemColorTable(const ColorTable&);
		void SetSystemColorTable(const ColorTable&&);
		void Set256ColorTable(const ColorTable&);
		void Set256ColorTable(const ColorTable&&);
		void SetCursorXY(int32_t x,size_t y);
		void SetCursorX(int32_t x);
		void SetCursorY(size_t x);
		void MoveCursorX(int32_t x);
		void MoveCursorYUp(size_t y);
		void MoveCursorYDown(size_t y);
		size_t GetLineCount()const;
		void SetLineCountMax(size_t max);
		size_t GetViewCount()const;
		void SetViewCount(size_t count);
		void ViewPositionUp(size_t count);
		void ViewPositionDown(size_t count);
		size_t GetViewPosition()const;
		void SetViewPosition(size_t y);
		const Attribute& GetCurrentAttribute()const;
		const Attribute& GetDefaultAttribute()const;
		Attribute& EditCurrentAttribute();
		Attribute& EditDefaultAttribute();
		void SetAttriuteDefault();
		void RemoveLineBefore();
		void RemoveLineAfter();
		void RemoveLine();
		void RemoveAll();
		void Remove();
		void RemoveBefore();
		void RemoveAfter();
		void SaveCursor();
		void RestoreCursor();
		ShellContext::attrtext_iterator begin()const;
		ShellContext::attrtext_iterator end()const ;
		void Insert(const std::wstring&);
	};
}