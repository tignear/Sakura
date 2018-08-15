#pragma once
#include <Windows.h>
namespace {
	struct Info {
		DWORD pid;
		HWND hwnd;
	};
	BOOL CALLBACK EnumWindowsProc(
		HWND hwnd,      // �e�E�B���h�E�̃n���h��
		LPARAM lParam   // �A�v���P�[�V������`�̒l
	) {
		auto info = reinterpret_cast<Info*>(lParam);
		DWORD pid;
		GetWindowThreadProcessId(hwnd,&pid);
		if (info->pid == pid) {
			info->hwnd = hwnd;
			return FALSE;
		}
		return TRUE;
	}
}
namespace tignear::win32 {
	
	HWND GetHwndFromProcess(DWORD pid){
		Info info;
		info.pid = pid;
		EnumWindows(EnumWindowsProc,reinterpret_cast<LPARAM>(&info));
		return info.hwnd;
	}
	
}