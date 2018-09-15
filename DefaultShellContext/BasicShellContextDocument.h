#pragma once
#include <functional>
#include <list>
#include "ShellContext.h"
#include "BasicShellContextLineText.h"
namespace tignear::sakura {


	class attrtext_line_iterator_impl :public ShellContext::attrtext_line_iterator_inner {
		attrtext_line_iterator_impl(const attrtext_line_iterator_impl&);
		std::list<BasicShellContextLineText>::iterator m_elem;
	public:
		attrtext_line_iterator_impl(std::list<BasicShellContextLineText>::iterator e) :m_elem(e) {

		}
		void operator++() override;
		void operator--() override;

		attrtext_line_iterator_impl* operator++(int) override;
		attrtext_line_iterator_impl* operator--(int) override;

		reference operator*()const override;
		pointer operator->()const override;
		bool operator==(const attrtext_line_iterator_inner& iterator)const override;
		bool operator!=(const attrtext_line_iterator_inner& iterator)const override;
		attrtext_line_iterator_impl* clone()const override;
		~attrtext_line_iterator_impl() {};

	};

	class attrtext_document_all_impl:public ShellContext::attrtext_document {
		std::list<BasicShellContextLineText>& m_text;
	public:
		attrtext_document_all_impl(std::list<BasicShellContextLineText>& text):m_text(text){}
		ShellContext::attrtext_line_iterator begin() override;
		ShellContext::attrtext_line_iterator end() override;

	};
	class attrtext_document_view_impl :public ShellContext::attrtext_document {
		const std::list<BasicShellContextLineText>::iterator& m_viewend_itr;
		const size_t& m_viewcount;
		const std::list<BasicShellContextLineText>& m_text;

	public:
		attrtext_document_view_impl(const std::list<BasicShellContextLineText>& text,const std::list<BasicShellContextLineText>::iterator& viewend_itr, const size_t& viewcount) :m_text(text),m_viewend_itr(viewend_itr),m_viewcount(viewcount) {}
		ShellContext::attrtext_line_iterator begin() override;
		ShellContext::attrtext_line_iterator end() override;
	};
	class TextUpdateInfoImpl:public ShellContext::TextUpdateInfoLineInner {
	private:
		ShellContext::TextUpdateStatus m_status;
		ShellContext::attrtext_line& m_line;
		TextUpdateInfoImpl(const TextUpdateInfoImpl& from):m_line(from.m_line){
			m_status = from.m_status;
			
		}
	public:
		TextUpdateInfoImpl(ShellContext::TextUpdateStatus sta, ShellContext::attrtext_line& line) :m_status(sta), m_line(line){}
		ShellContext::TextUpdateStatus status()const override;
		ShellContext::attrtext_line& line() override;
		TextUpdateInfoImpl* clone()const override;
	};
	class BasicShellContextDocument {
		using TextUpdateInfoLine = ShellContext::TextUpdateInfoLine;
		static TextUpdateInfoLine BuildTextUpdateInfoLine(ShellContext::TextUpdateStatus, ShellContext::attrtext_line&);
		std::list<BasicShellContextLineText> m_text;
		std::list<BasicShellContextLineText>::iterator m_cursorY_itr;
		std::list<BasicShellContextLineText>::iterator m_cursorY_itr_save;
		std::list<BasicShellContextLineText>::iterator m_viewend_itr;
		attrtext_document_all_impl m_all;
		attrtext_document_view_impl m_view;
		//std::list<BasicShellContextLineText>::iterator m_origin_itr;
		size_t m_max_line;
		size_t m_cursorY;
		size_t m_cursorY_save;
		//size_t m_origin;
		size_t m_viewendpos;
		size_t m_viewcount;
		int32_t m_cursorX;
		int32_t m_curorX_save;
		ansi::ColorTable m_color_256;
		ansi::ColorTable m_color_sys;
		std::vector<std::wstring> m_fontmap;
		Attribute m_current_attr;
		Attribute m_default_attr;
		std::function<void(bool,bool)> m_layout_change_callback;
		std::function<void(std::vector<ShellContext::TextUpdateInfoLine>)> m_text_change_callback;
		bool FixCursorY();
		bool CreateIfEnd();

		void NotifyLayoutChange(bool x,bool y);
		void NotifyTextChange(std::vector<ShellContext::TextUpdateInfoLine>);
	public:
		BasicShellContextDocument(
			const ansi::ColorTable& color_sys,
			const ansi::ColorTable& color_256,
			const std::vector<std::wstring>& fontmap,
			const Attribute& def,
			std::function<void(bool,bool)> layout_change,
			std::function<void(std::vector<ShellContext::TextUpdateInfoLine>)> text_change) :
			m_color_256(color_256),
			m_color_sys(color_sys),
			m_fontmap(fontmap),
			m_default_attr(def),
			m_current_attr(def),
			m_max_line(100),
			m_cursorY(0),
			m_cursorY_itr(m_text.begin()),
			m_viewend_itr(m_text.end()),
			m_viewendpos(0),
			m_viewcount(0),
			m_cursorX(0),
			m_layout_change_callback(layout_change),
			m_text_change_callback(text_change),
			m_all(m_text),
			m_view(m_text,m_viewend_itr,m_viewcount)
		{
			CreateIfEnd();
		}
		void SetSystemColorTable(const ansi::ColorTable&);
		void SetSystemColorTable(const ansi::ColorTable&&);
		void Set256ColorTable(const ansi::ColorTable&);
		void Set256ColorTable(const ansi::ColorTable&&);
		void SetCursorXY(int32_t x,size_t y);
		void SetCursorX(int32_t x);
		void SetCursorY(size_t x);
		size_t GetCursorX()const;
		BasicShellContextLineText& GetCursorY()const;
		void MoveCursorX(int32_t x);
		void MoveCursorYUp(size_t y);
		void MoveCursorYDown(size_t y);
		size_t GetLineCount()const;
		void SetLineCountMax(size_t max);
		size_t GetViewCount()const;
		void SetPageSize(size_t count);
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
		attrtext_document_all_impl& GetAll();
		attrtext_document_view_impl& GetView();
		void Insert(const std::wstring&);
	};
}