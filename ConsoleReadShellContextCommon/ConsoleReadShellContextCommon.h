#pragma once
#include <stdexcept>
#include <Windows.h>
namespace tignear::sakura {
	struct MappingInfo {
		unsigned short allocateWidth;
		unsigned short allocateHeight;
		unsigned short cursorX;
		unsigned short cursorY;
		unsigned short viewBeginY;
		unsigned short viewSize;
		unsigned short width;
		unsigned short height;
	};
	class MappingView {
		HANDLE handle;
		MappingInfo* m_info;
		wchar_t* m_buf;//allocateWidth*allocateHeight
	public:

		~MappingView() {
			if (handle != NULL) {
				CloseHandle(handle);
				UnmapViewOfFile(m_info);
			}
		}
		MappingView(HANDLE handle, MappingInfo* info, wchar_t* buf) :handle(handle), m_info(info), m_buf(buf) {}
		MappingView(const MappingView&) = delete;
		MappingView(MappingView&& from)noexcept {
			*this = std::move(from);
		}
		MappingView& operator=(MappingView&& from) noexcept {
			handle = from.handle;
			from.handle = NULL;
			m_info = from.m_info;
			from.m_info = nullptr;
			m_buf = from.m_buf;
			from.m_buf = nullptr;
			return *this;
		}
		MappingView():handle(NULL),m_info(nullptr),m_buf(nullptr){}
		MappingInfo* info() {
			return m_info;
		}
		const MappingInfo* info() const{
			return m_info;
		}
		wchar_t* buf() {
			return m_buf;
		}
		operator bool()const{
			return handle&& m_info&&m_buf;
		}
	};
#pragma warning(disable:4505)

	static MappingView CreateMappingViewW(const wchar_t* name,unsigned short width, unsigned short height){
		auto size =static_cast<DWORD>(sizeof(MappingInfo)+ width * height*sizeof(wchar_t));
		auto handle=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL, PAGE_READWRITE,0,size,name);
		if (handle == INVALID_HANDLE_VALUE) {
			throw std::runtime_error(__FILE__);
		}
		char* p=reinterpret_cast<char*>(MapViewOfFile(handle,FILE_MAP_WRITE,0,0,0));
		MappingInfo* info = reinterpret_cast<MappingInfo*>(p);
		info->allocateWidth = width;
		info->allocateHeight = height;
		wchar_t* buf= reinterpret_cast<wchar_t*>(p + sizeof(MappingInfo));
		return MappingView{handle, info,buf };
	}
	static MappingView OpenMappingViewW(const wchar_t* name) {
		HANDLE handle=OpenFileMappingW(FILE_MAP_READ, FALSE, name);
		if (handle == INVALID_HANDLE_VALUE) {
			throw std::runtime_error(__FILE__);
		}
		char* p = reinterpret_cast<char*>(MapViewOfFile(handle, FILE_MAP_READ, 0, 0, 0));
		if (p==nullptr) {
#pragma warning(disable:4189)
			auto e = GetLastError();
			OutputDebugStringW(L"");
		}
		MappingInfo* info = reinterpret_cast<MappingInfo*>(p);
		wchar_t* buf = reinterpret_cast<wchar_t*>(p + sizeof(MappingInfo));
		return MappingView{ handle, info,buf };
	}
#pragma warning(default:4505)

}