#pragma once
#include <memory>
#include <unordered_map>
#include <atomic>
#include "IOCPMgr.h"
#include "ShellContext.h"
#include "ansi/BasicAttributeText.h"
#include "ansi/AnsiParser.h"
#include "tstring.h"
namespace tignear::sakura {
	class BasicShellContext:public ShellContext {
	private:
		struct Attribute {

		};
		//ansi parser call backs
		friend BasicShellContext& ansi::parse<BasicShellContext>(std::wstring_view,BasicShellContext&);
		void FindCSI(std::wstring_view);
		void FindString(std::wstring_view);
		//ansi parser work helpers
		void RemoveRows(size_t count);
		void RemoveRowsR(size_t count);
		void RemoveColumns(size_t count);
		void RemoveColumnsR(size_t count);
		//class members
		constexpr static unsigned int BUFFER_SIZE = 4096;
		static std::atomic_uintmax_t m_process_count;
		std::shared_ptr<iocp::IOCPMgr> m_iocpmgr;
		HANDLE m_childProcess;
		HANDLE m_out_pipe;
		HANDLE m_in_pipe;
		HWND m_hwnd;
		unsigned int m_origin_cursorY;
		//std::list<std::list<ansi::AttributeText*>>::iterator m_originY_itr;
		unsigned int m_cursorX, m_cursorY;
		mutable std::unordered_map<std::uintptr_t,std::function<void(ShellContext*)>> m_text_change_listeners;
		static bool IOWorkerStart(std::shared_ptr<BasicShellContext>);
		static bool OutputWorker(std::shared_ptr<BasicShellContext>);
		static bool OutputWorkerHelper(DWORD cnt,std::shared_ptr<BasicShellContext>);
		void AddString(std::wstring_view);
		bool m_buffer_rebuild;
		std::wstring m_buffer;
		std::list<std::list<ansi::AttributeText*>> m_text;
		bool Init(stdex::tstring);
		//out pipe temp
		std::string m_outbuf;
		Attribute m_current_attr;
	public:
		BasicShellContext(std::shared_ptr<iocp::IOCPMgr> iocpmgr):
			m_buffer_rebuild(false),
			m_iocpmgr(iocpmgr),
			m_outbuf(BUFFER_SIZE, '\0'),
			m_text(),
			m_cursorX(0),
			m_cursorY(0),
			m_current_attr()
		{}
		~BasicShellContext() {
			CloseHandle(m_out_pipe);
			CloseHandle(m_in_pipe);
			CloseHandle(m_childProcess);
		}
		static std::shared_ptr<BasicShellContext> Create(stdex::tstring,std::shared_ptr<iocp::IOCPMgr>);
		void InputChar(WPARAM c) override;
		void InputKey(WPARAM keycode) override;
		void InputKey(WPARAM keycode, unsigned int count) override;
		void InputString(std::wstring_view) override;
		void ConfirmString(std::wstring_view) override;
		const std::list<std::list<ansi::AttributeText*>>& GetText()const override;
		std::wstring_view GetString()const override;
		unsigned int GetCursorX()const override;
		unsigned int GetCursorY()const override;
		uintptr_t AddTextChangeListener(std::function<void(ShellContext*)>)const override;
		void RemoveTextChangeListener(uintptr_t)const override;
		uintptr_t AddCursorChangeListener(std::function<void(ShellContext*)>)const override;
		void RemoveCursorChangeListener(uintptr_t)const override;

	};

}