#pragma once
#include <Windows.h>
#include "AttributeText.h"
namespace tignear::sakura {
	class ShellContext {
	public:
		virtual void InputKey(WPARAM keycode)=0;
		virtual void InputChar(WPARAM charcode) = 0;
		virtual void InputString(std::wstring wstr) = 0;
		virtual std::list<LineText> GetText() = 0;
		virtual unsigned int GetCursorX()=0;
		virtual unsigned int GetCursorY()=0;
	};

}