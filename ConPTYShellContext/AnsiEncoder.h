#pragma once
#include <string>
#include <Windows.h>
namespace tignear::ansi {
	static inline std::string encode(WPARAM wp) {
		switch(wp){
		case VK_UP:
			return "\x1b[A";
		case VK_DOWN:
			return "\x1b[B";
		case VK_RIGHT:
			return "\x1b[C";
		case VK_LEFT:
			return "\x1b[D";
		case VK_RETURN:
			return "\n";
		case VK_SPACE:
			return " ";
		default:
			return "";
		}
	}
}