#pragma once
#include <vector>
#include <Windows.h>
namespace tignear::win {
#pragma warning(disable:4505)
	static void transferTo(HANDLE in,HANDLE out,DWORD bufsize=4096) {
		std::vector<char> buf;
		buf.reserve(bufsize);
		DWORD read=0;
		while (ReadFile(in, buf.data(), bufsize, &read, NULL)) {
			DWORD write=0;
			for (DWORD writed = 0; WriteFile(out, buf.data()+writed, read, &write, NULL)&&writed<read; writed += write) {}
		}

	}
#pragma warning(default:4505)

}