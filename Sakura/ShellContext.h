#pragma once
#include <iterator>
#include <Windows.h>
#include "ansi/AttributeText.h"
namespace tignear::sakura {
	class ShellContext {

	public:
		class attrtext_iterator_innner {
		public:
			using iterator_category = std::input_iterator_tag;
			using value_type = const ansi::AttributeText;
			using pointer = value_type * ;
			using reference = value_type & ;
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
			attrtext_iterator(attrtext_iterator& self):m_inner(std::unique_ptr<attrtext_iterator_innner>(self.m_inner->clone())) {
				
			}
			using iterator_category = std::input_iterator_tag;
			using value_type = const ansi::AttributeText;
			using pointer = value_type * ;
			using reference = value_type & ;
			attrtext_iterator& operator++() {
				 m_inner->operator++();
				 return *this;
			};
			attrtext_iterator operator++(int i) {
				return attrtext_iterator(std::unique_ptr<attrtext_iterator_innner>(m_inner->operator++(i)));
			}
			reference operator*() {
				return m_inner->operator*();
			};
			pointer operator->() {
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
			virtual attrtext_iterator begin()const = 0;
			virtual attrtext_iterator end()const = 0;
		};
		class attrtext_line_iterator_inner{
		public:
			using iterator_category = std::input_iterator_tag;
			using value_type =const attrtext_line;
			using pointer = value_type * ;
			using reference = value_type & ;
			virtual void operator++() = 0;
			virtual attrtext_line_iterator_inner* operator++(int) = 0;
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
			attrtext_line_iterator(attrtext_line_iterator& self) :m_inner(std::unique_ptr<attrtext_line_iterator_inner>(self.m_inner->clone())) {

			}
			using iterator_category = std::input_iterator_tag;
			using value_type = const attrtext_line;
			using pointer = value_type * ;
			using reference = value_type & ;
			attrtext_line_iterator& operator++() {
				m_inner->operator++();
				return *this;
			};
			attrtext_line_iterator operator++(int i) {
				return attrtext_line_iterator(std::unique_ptr<attrtext_line_iterator_inner>(m_inner->operator++(i)));
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
		virtual void InputKey(WPARAM keycode)=0;//no lock call
		virtual void InputKey(WPARAM keycode,unsigned int count) = 0;//no lock call
		virtual void InputChar(WPARAM charcode) = 0;//no lock call
		virtual void InputString(std::wstring_view wstr) = 0;//no lock call
		virtual void ConfirmString(std::wstring_view)=0;//no lock call
		virtual attrtext_line_iterator begin()const=0;//lock call
		virtual attrtext_line_iterator end()const = 0;//lock call
		virtual std::wstring_view GetTitle()const = 0;//no lock call
		virtual size_t GetLineCount()const=0;
		virtual size_t GetViewCount()const=0;//no lock call
		virtual void SetPageSize(size_t count)=0;//no lock call
		virtual size_t GetViewStart()const=0;
		virtual void SetViewStart(size_t)=0;
		virtual uintptr_t AddTextChangeListener(std::function<void(ShellContext*)>)const=0;//no lock call
		virtual void RemoveTextChangeListener(uintptr_t)const = 0;//no lock call
		virtual uintptr_t AddLayoutChangeListener(std::function<void(ShellContext*)>)const = 0;//no lock call
		virtual void RemoveLayoutChangeListener(uintptr_t)const = 0;//no lock call
		virtual void Set256Color(const std::unordered_map<unsigned int,uint32_t>&)=0;//no lock call
		virtual void Set256Color(const std::unordered_map<unsigned int, uint32_t>&&)=0;//no lock call
		virtual void SetSystemColor(const std::unordered_map<unsigned int, uint32_t>&)=0;//no lock call
		virtual void SetSystemColor(const std::unordered_map<unsigned int, uint32_t>&&)=0;//no lock call
		virtual void Lock()=0;//no lock call
		virtual void Unlock()=0;//lock call
		virtual void Resize(UINT w,UINT h)=0;
		virtual const std::wstring& DefaultFont()const=0;
		virtual double FontSize()const =0;
		virtual bool UseTerminalEchoBack()const=0;
		virtual ~ShellContext() {};//no lock call
	};

}