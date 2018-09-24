#pragma once
#include <iterator>
#include <Windows.h>
#include "ansi/AttributeText.h"
namespace tignear::sakura {
	class ShellContext {

	public:
		class attrtext_iterator_innner {
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type =const ansi::AttributeText;
			using pointer = value_type * ;
			using reference = value_type & ;
			using difference_type = std::ptrdiff_t;

			virtual void operator++() = 0;
			virtual attrtext_iterator_innner* operator++(int) = 0;
			virtual reference operator*()const = 0;
			virtual pointer operator->()const = 0;
			virtual bool operator==(const attrtext_iterator_innner& iterator)const = 0;
			virtual bool operator!=(const attrtext_iterator_innner& iterator)const = 0;
			virtual attrtext_iterator_innner* clone()const = 0;
			virtual ~attrtext_iterator_innner() {};
		};
		class attrtext_iterator {
		private:
			std::unique_ptr<attrtext_iterator_innner> m_inner;
		public:
			attrtext_iterator(std::unique_ptr<attrtext_iterator_innner> inner):m_inner(std::move(inner)){
				
			}
			attrtext_iterator(const attrtext_iterator& self):m_inner(std::unique_ptr<attrtext_iterator_innner>(self.m_inner->clone())) {
				
			}
			using iterator_category = std::forward_iterator_tag;
			using value_type = const ansi::AttributeText;
			using pointer = value_type * ;
			using reference = value_type & ;
			using difference_type = std::ptrdiff_t;

			attrtext_iterator& operator++() {
				 m_inner->operator++();
				 return *this;
			};
			attrtext_iterator operator++(int i) {
				return attrtext_iterator(std::unique_ptr<attrtext_iterator_innner>(m_inner->operator++(i)));
			}
			reference operator*()const {
				return m_inner->operator*();
			};
			pointer operator->()const {
				return m_inner->operator->();
			};
			bool operator==(const attrtext_iterator& iterator)const {
				return m_inner->operator==((*iterator.m_inner));
			}
			bool operator!=(const attrtext_iterator& iterator)const {
				return m_inner->operator!=((*iterator.m_inner));
			}
			~attrtext_iterator() {};

		};
		struct attrtext_line {
			virtual attrtext_iterator begin() = 0;
			virtual attrtext_iterator end() = 0;
			virtual std::shared_ptr<void>& resource() = 0;
			virtual bool operator==(const attrtext_line&)const=0;
			virtual bool operator!=(const attrtext_line&)const = 0;
		};
		class attrtext_line_iterator_inner{
		public:
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = attrtext_line;
			using pointer = value_type * ;
			using reference = value_type & ;
			using difference_type = std::ptrdiff_t;

			virtual void operator++() = 0;
			virtual void operator--() = 0;
			virtual attrtext_line_iterator_inner* operator++(int) = 0;
			virtual attrtext_line_iterator_inner* operator--(int) = 0;
			virtual reference operator*()const = 0;
			virtual pointer operator->()const = 0;
			virtual bool operator==(const attrtext_line_iterator_inner& iterator)const = 0;
			virtual bool operator!=(const attrtext_line_iterator_inner& iterator)const = 0;
			virtual attrtext_line_iterator_inner* clone()const = 0;
			virtual ~attrtext_line_iterator_inner() {};
		};

		class attrtext_line_iterator{
		private:
			std::unique_ptr<attrtext_line_iterator_inner> m_inner;
		public:
			attrtext_line_iterator(std::unique_ptr<attrtext_line_iterator_inner> inner) :m_inner(std::move(inner)) {

			}
			attrtext_line_iterator(const attrtext_line_iterator& self) :m_inner(std::unique_ptr<attrtext_line_iterator_inner>(self.m_inner->clone())) {

			}
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = attrtext_line;
			using pointer = value_type * ;
			using reference = value_type & ;
			using difference_type = std::ptrdiff_t;
			attrtext_line_iterator& operator++() {
				m_inner->operator++();
				return *this;
			};
			attrtext_line_iterator& operator--() {
				m_inner->operator--();
				return *this;
			};
			attrtext_line_iterator operator++(int i) {
				return attrtext_line_iterator(std::unique_ptr<attrtext_line_iterator_inner>(m_inner->operator++(i)));
			}
			attrtext_line_iterator operator--(int i) {
				return attrtext_line_iterator(std::unique_ptr<attrtext_line_iterator_inner>(m_inner->operator--(i)));
			}
			reference operator*() {
				return m_inner->operator*();
			};
			pointer operator->() {
				return m_inner->operator->();
			};
			bool operator==(const attrtext_line_iterator& iterator)const {
				return m_inner->operator==((*iterator.m_inner));
			}
			bool operator!=(const attrtext_line_iterator& iterator)const {
				return m_inner->operator!=((*iterator.m_inner));
			}
			~attrtext_line_iterator() {};
		};
		struct attrtext_document {
			virtual attrtext_line_iterator begin() = 0;
			virtual attrtext_line_iterator end() = 0;
		};
		enum class TextUpdateStatus {
			ERASE,MODIFY,NEW
		};
		struct TextUpdateInfoLineInner {
			virtual TextUpdateStatus status()const=0;
			virtual attrtext_line& line()=0;
			virtual TextUpdateInfoLineInner* clone()const = 0;
		};
		class TextUpdateInfoLine {
			std::unique_ptr<TextUpdateInfoLineInner> info;
		public:
			TextUpdateInfoLine(std::unique_ptr<TextUpdateInfoLineInner> info) :info(std::move(info)) {}
			TextUpdateInfoLine(const TextUpdateInfoLine& from) {
				info = std::unique_ptr<TextUpdateInfoLineInner>(from.info->clone());
			}
			TextUpdateStatus status()const{
				return info->status();
			}
			attrtext_line& line() {
				return info->line();
			}
		};
		virtual void InputKey(WPARAM keycode,LPARAM lp=0)=0;//no lock call
		virtual void InputKey(WPARAM keycode,unsigned int count) = 0;//no lock call
		virtual void InputChar(WPARAM charcode,LPARAM lp=0) = 0;//no lock call
		virtual void InputString(std::wstring_view wstr) = 0;//no lock call
		virtual void ConfirmString(std::wstring_view)=0;//no lock call
		virtual attrtext_document& GetAll()=0;//lock call
		virtual attrtext_document& GetView()=0;//lock call
		virtual std::wstring_view GetTitle()const = 0;//no lock call
		virtual size_t GetLineCount()const=0;//no lock call
		virtual size_t GetViewCount()const=0;//no lock call
		virtual void SetPageSize(size_t count)=0;//no lock call
		virtual size_t GetViewStart()const=0;//no lock call
		virtual attrtext_line& GetCursorY()=0;
		virtual size_t GetCursorXWStringPos()const = 0;//no lock call. wstring_view position.
		virtual void SetViewStart(size_t)=0;//no lock call
		virtual uintptr_t AddTextChangeListener(std::function<void(ShellContext*,std::vector<TextUpdateInfoLine>)>)const=0;//no lock call
		virtual void RemoveTextChangeListener(uintptr_t)const = 0;//no lock call
		virtual uintptr_t AddLayoutChangeListener(std::function<void(ShellContext*,bool,bool)>)const = 0;//no lock call
		virtual void RemoveLayoutChangeListener(uintptr_t)const = 0;//no lock call
		virtual uintptr_t AddExitListener(std::function<void(ShellContext*)>)const = 0;//no lock call
		virtual void RemoveExitListener(uintptr_t)const = 0;//no lock call
		virtual void Lock()=0;//no lock call
		virtual void Unlock()=0;//lock call
		virtual void Resize(UINT w,UINT h)=0;//no lock call
		virtual uint32_t BackgroundColor()const=0;//no lock call
		virtual uint32_t FrontColor()const=0;//no lock call
		virtual const std::wstring& DefaultFont()const=0;//no lock call
		virtual double FontSize()const =0;//no lock call
		virtual bool UseTerminalEchoBack()const=0;//no lock call
		virtual void Terminate()=0;
		virtual LRESULT OnMessage(UINT,LPARAM lparam)=0;
		virtual ~ShellContext() {};//no lock call.bat not require lock.
	};

}