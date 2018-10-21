#pragma once
#include <shellapi.h>
#include <ProcessTree.h>
#include <BasicShellContext.h>
#include <strconv.h>
#include "AnsiEncoder.h"
namespace tignear::sakura {

	class ConPTYShellContext :public BasicShellContext {
	public:
		struct Options {
			LPVOID environment;
			LPCTSTR current_directory;
		};
	private:
		HANDLE m_in_pipe=NULL;
		HANDLE m_childProcess;
		std::thread m_thread;
		static bool Init(std::shared_ptr<ConPTYShellContext> s,const stdex::tstring& execute, stdex::tstring cmdstr,const Options& opt) {
			using stdex::tstring;
			using stdex::to_tstring;
			//http://yamatyuu.net/computer/program/sdk/base/cmdpipe1/index.html
			SECURITY_ATTRIBUTES sa;
			sa.nLength = sizeof(sa);
			sa.bInheritHandle = TRUE;
			sa.lpSecurityDescriptor = NULL;
			auto str_process_id = stdex::to_tstring(GetCurrentProcessId());
			auto str_process_cnt = stdex::to_tstring(m_process_count);

			tstring out_pipename = _T("\\\\.\\pipe\\tignear.sakura.ConPTYShellContext.out.");
			out_pipename += str_process_id;
			out_pipename += _T(".");
			out_pipename += str_process_cnt;
			tstring in_pipename = _T("\\\\.\\pipe\\tignear.sakura.ConPTYShellContext.in.");
			in_pipename += str_process_id;
			in_pipename += _T(".");
			in_pipename += str_process_cnt;
			++m_process_count;

			s->m_out_pipe = CreateNamedPipe(out_pipename.c_str(), PIPE_ACCESS_INBOUND | PIPE_ACCESS_OUTBOUND| FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE, 2, BUFFER_SIZE, BUFFER_SIZE, 1000, &sa);

			if (s->m_out_pipe == INVALID_HANDLE_VALUE) {
				return false;
			}
			s->m_in_pipe = CreateNamedPipe(in_pipename.c_str(), PIPE_ACCESS_INBOUND | PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE, 2, BUFFER_SIZE, BUFFER_SIZE, 1000, &sa);
			if (s->m_in_pipe == INVALID_HANDLE_VALUE) {
				return false;
			}

			if (opt.current_directory) {

				cmdstr.insert(0, _T("-c=") + tstring(opt.current_directory) + _T(" "));
			}
			(cmdstr += _T(" ")) += in_pipename;
			(cmdstr += _T(" ")) += out_pipename;
			SHELLEXECUTEINFO info{};
			info.lpFile = execute.c_str();
			info.lpParameters = cmdstr.c_str();
			info.cbSize = sizeof(info);
			info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC;
			info.nShow = SW_HIDE;
			s->m_iocpmgr->Attach(s->m_out_pipe);
			if (!ShellExecuteEx(&info) || !info.hProcess) {
#pragma warning(disable:4189)
				auto e = GetLastError();
				return false;
			}
#pragma warning(default:4189)

			s->m_childProcess = info.hProcess;
			s->m_thread = std::thread([s, pid = GetProcessId(s->m_childProcess), hProcess = s->m_childProcess]() {
				WaitForSingleObject(hProcess, INFINITE);
				s->NotifyExit();
			});

			return true;
		}
	public:
		ConPTYShellContext(
			std::shared_ptr<iocp::IOCPMgr> iocpmgr,
			unsigned char ambiguous_size,

			const ansi::ColorTable& c_sys,
			const ansi::ColorTable& c_256,
			bool use_terminal_echoback,
			std::vector<std::wstring> fontmap,
			double fontsize,
			Attribute& def
		) :BasicShellContext(iocpmgr, ambiguous_size,65001,c_sys,c_256,use_terminal_echoback,fontmap,fontsize,def) {

		}
		void InputChar(WPARAM c, LPARAM)override {
			if (c == '\b') {
				c = 0x7f;//DEL
			}
			auto w=std::make_shared<std::string>(wide_to_utf8(std::wstring(1,static_cast<wchar_t>(c))));
			auto buf = w->c_str();
			auto len =static_cast<DWORD>(w->length());
			auto info = new iocp::IOCPInfo{
				{},[w](DWORD){}
			};
			WriteFile(m_in_pipe,buf,len,NULL,&info->overlapped);
		}
		void InputKey(WPARAM keycode, LPARAM)override {
			
			auto w = std::make_shared<std::string>(ansi::encode(keycode));
			if (w->empty()) {
				return;
			}
			auto buf = w->c_str();
			auto len = static_cast<DWORD>(w->length());
			auto info = new iocp::IOCPInfo{
				{},[w](DWORD) {}
			};
			WriteFile(m_in_pipe, buf, len, NULL, &info->overlapped);
		}
		void InputKey(WPARAM keycode, unsigned int count)override {
			for (auto i = 0U; i < count; i++) {
				InputKey(keycode, static_cast<LPARAM>(0));

			}
		}
		void InputString(std::wstring_view v)override {
			auto w = std::make_shared<std::string>(wide_to_cp(v,65001));
			auto buf =w->c_str();
			auto len = static_cast<DWORD>(w->length());
			auto info = new iocp::IOCPInfo{
				{},[w](DWORD) {}
			};
			WriteFile(m_in_pipe, buf, len, NULL, &info->overlapped);
		}
		static bool WorkerStart(std::shared_ptr<ConPTYShellContext> s) {
			auto info = new iocp::IOCPInfo{
				{},
				[s](DWORD) {
					s->OutputWorker(s);
				}
			};
			if (!ConnectNamedPipe(s->m_out_pipe, &info->overlapped)) {
				auto e = GetLastError();
				if (e != ERROR_IO_PENDING) {
					return false;
				}
			}
			return true;
		}

		static std::shared_ptr<ConPTYShellContext> Create(
			const stdex::tstring& exe,
			stdex::tstring cmd,
			
			std::shared_ptr<iocp::IOCPMgr> iocpmgr,
			unsigned char ambiguous_size,
			const ansi::ColorTable& c_sys,
			const ansi::ColorTable& c_256,
			bool use_terminal_echoback,
			std::vector<std::wstring> fontmap,
			double fontsize,
			const Options& opt) {
			Attribute attr{};
			attr.frColor.type = ColorType::ColorSystem;
			attr.frColor.color_system = 30;
			attr.bgColor.type = ColorType::ColorSystem;
			attr.bgColor.color_system = 47;
			auto r = std::make_shared<ConPTYShellContext>(iocpmgr, ambiguous_size, c_sys, c_256, use_terminal_echoback, fontmap, fontsize,attr);
			
			if (!Init(r,exe,cmd,opt)) {
				r.reset();
				return r;
			}

			if (!WorkerStart(r)) {
				r.reset();
				return r;
			}
			return r;
		}
		void Terminate()override{
			win32::TerminateProcessTree(win32::ProcessTree(GetProcessId(m_childProcess)));

			if (m_thread.joinable()) {
				m_thread.join();
			}
		}
		virtual ~ConPTYShellContext() {
			if (m_in_pipe) {
				CloseHandle(m_in_pipe);
				m_in_pipe = NULL;
			}
			if (m_childProcess) {
				CloseHandle(m_childProcess);
				m_in_pipe = NULL;
			}
			if (m_thread.joinable()) {
				m_thread.join();
			}
		}

	};
}