#pragma once
#include <Windows.h>
#include "ansi/AttributeText.h"
namespace tignear::sakura {
	class ShellContext {
	public:
		virtual void InputKey(WPARAM keycode)=0;
		virtual void InputKey(WPARAM keycode,unsigned int count) = 0;
		virtual void InputChar(WPARAM charcode) = 0;
		virtual void InputString(std::wstring_view wstr) = 0;
		virtual void ConfirmString(std::wstring_view)=0;
		const virtual std::list<std::list<ansi::AttributeText>>& GetViewText()const = 0;
		virtual std::wstring_view GetViewString()const=0;
		virtual size_t GetViewLineCount()const=0;
		virtual void SetViewLineCount(size_t count)=0;
		virtual uintptr_t AddTextChangeListener(std::function<void(ShellContext*)>)const=0;
		virtual void RemoveTextChangeListener(uintptr_t)const = 0;
		virtual uintptr_t AddCursorChangeListener(std::function<void(ShellContext*)>)const = 0;
		virtual void RemoveCursorChangeListener(uintptr_t)const = 0;
	};

}