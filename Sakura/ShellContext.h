#pragma once
#include <Windows.h>
#include "ansi/AttributeText.h"
namespace tignear::sakura {
	class ShellContext {
	public:
		virtual void InputKey(WPARAM keycode)=0;//no lock call
		virtual void InputKey(WPARAM keycode,unsigned int count) = 0;//no lock call
		virtual void InputChar(WPARAM charcode) = 0;//no lock call
		virtual void InputString(std::wstring_view wstr) = 0;//no lock call
		virtual void ConfirmString(std::wstring_view)=0;//no lock call
		virtual std::list<std::list<ansi::AttributeText>>::const_iterator GetViewTextBegin()const = 0;//lock call
		virtual std::list<std::list<ansi::AttributeText>>::const_iterator GetViewTextEnd()const = 0;//lock call
		virtual std::list<std::list<ansi::AttributeText>>::size_type GetViewLineCount()const=0;//no lock call
		virtual std::wstring_view GetTitle()const =0;//no lock call
		virtual void SetViewLineCount(std::list<std::list<ansi::AttributeText>>::size_type count)=0;//no lock call
		virtual uintptr_t AddTextChangeListener(std::function<void(ShellContext*)>)const=0;//no lock call
		virtual void RemoveTextChangeListener(uintptr_t)const = 0;//no lock call
		virtual uintptr_t AddCursorChangeListener(std::function<void(ShellContext*)>)const = 0;//no lock call
		virtual void RemoveCursorChangeListener(uintptr_t)const = 0;//no lock call
		virtual void Set256Color(const std::unordered_map<unsigned int,uint32_t>&)=0;//no lock call
		virtual void Set256Color(const std::unordered_map<unsigned int, uint32_t>&&)=0;//no lock call
		virtual void SetSystemColor(const std::unordered_map<unsigned int, uint32_t>&)=0;//no lock call
		virtual void SetSystemColor(const std::unordered_map<unsigned int, uint32_t>&&)=0;//no lock call
		virtual void Lock()=0;//no lock call
		virtual void Unlock()=0;//lock call
	};

}