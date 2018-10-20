#pragma once
#include <stdexcept>
#include <Windows.h>
namespace tignear::sakura::conread {
	namespace exe2context{
		
		constexpr const static UINT CREATE_VIEW = WM_APP;
		constexpr const static UINT UPDATE_CARET=WM_APP+1;
		constexpr const static UINT UPDATE_REGION= WM_APP + 2;
		constexpr const static UINT UPDATE_SIMPLE= WM_APP + 3;
		constexpr const static UINT UPDATE_SCROLL= WM_APP + 4;
		constexpr const static UINT END_OF_MESSAGE= UPDATE_SCROLL;
	};
	namespace context2exe {
		constexpr const static UINT INPUT_CHAR = WM_APP;
		constexpr const static UINT INPUT_KEY = WM_APP+1;
		constexpr const static UINT END_OF_MESSAGE = INPUT_KEY;

	}

	struct MappingInfo {
		static const constexpr auto TITLE_LENGTH = 4096;
		unsigned short allocateWidth;
		unsigned short allocateHeight;
		unsigned short cursorX;
		unsigned short cursorY;
		unsigned short viewBeginY;
		unsigned short viewSize;
		unsigned short width;
		unsigned short height;
		wchar_t title[TITLE_LENGTH];
	};
	class MappingView {
		HANDLE handle;
		MappingInfo* m_info;
		wchar_t* m_buf;//allocateWidth*allocateHeight
		WORD* m_attributes;//allocateWidth*allocateHeight
	public:

		~MappingView() {
			if (handle != NULL&&handle!=INVALID_HANDLE_VALUE) {
				CloseHandle(handle);
				UnmapViewOfFile(m_info);
			}
		}
		MappingView(HANDLE handle, MappingInfo* info, wchar_t* buf, WORD* attr) :handle(handle), m_info(info), m_buf(buf),m_attributes(attr) {}
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
			m_attributes = from.m_attributes;
			from.m_attributes = nullptr;
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
		const wchar_t* buf()const {
			return m_buf;
		}
		WORD* attribute() {
			return m_attributes;
		}
		const WORD* attribute()const{
			return m_attributes;
		}
		operator bool()const{
			return handle&& m_info&&m_buf;
		}
	};
#pragma warning(disable:4505)

	static MappingView CreateMappingViewW(const wchar_t* name,unsigned short width, unsigned short height){
		auto size =static_cast<DWORD>(sizeof(MappingInfo)+ width * height*(sizeof(wchar_t)+sizeof(uint32_t)));
		auto handle=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL, PAGE_READWRITE,0,size,name);
		if (handle == INVALID_HANDLE_VALUE) {
			throw std::runtime_error(__FILE__);
		}
		char* p=reinterpret_cast<char*>(MapViewOfFile(handle,FILE_MAP_WRITE,0,0,0));
		MappingInfo* info = reinterpret_cast<MappingInfo*>(p);
		info->allocateWidth = width;
		info->allocateHeight = height;
		wchar_t* buf= reinterpret_cast<wchar_t*>(p + sizeof(MappingInfo));
		WORD* attr = reinterpret_cast<WORD*>(buf + width * height * sizeof(wchar_t));
		return MappingView{handle, info,buf,attr };
	}
	static MappingView OpenMappingViewW(const wchar_t* name) {
		HANDLE handle=OpenFileMappingW(FILE_MAP_READ, FALSE, name);
		if (handle == INVALID_HANDLE_VALUE) {
			throw std::runtime_error(__FILE__);
		}
		char* p = reinterpret_cast<char*>(MapViewOfFile(handle, FILE_MAP_READ, 0, 0, 0));
		if (p==nullptr) {
			return MappingView{ INVALID_HANDLE_VALUE, {},nullptr,nullptr };
		}
		MappingInfo* info = reinterpret_cast<MappingInfo*>(p);
		wchar_t* buf = reinterpret_cast<wchar_t*>(p + sizeof(MappingInfo));
		WORD* attr = reinterpret_cast<WORD*>(buf + info->allocateWidth * info->allocateHeight * sizeof(wchar_t));
		return MappingView{ handle, info,buf,attr };
	}
#pragma warning(default:4505)

}