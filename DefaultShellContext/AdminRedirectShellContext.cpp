#include "stdafx.h"
#include <ProcessTree.h>
#include <ModuleFilePath.h>
#include <shellapi.h>
#include "AdminRedirectShellContext.h"
#include "GetHwndFromPid.h"
using tignear::stdex::tstring;
using tignear::win32::GetHwndFromProcess;
namespace tignear::sakura {
	bool AdminRedirectShellContext::CreateShell(std::shared_ptr<AdminRedirectShellContext> s,tstring execute,tstring cmdstr, const Options& opt) {
		//http://yamatyuu.net/computer/program/sdk/base/cmdpipe1/index.html
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;
		tstring out_pipename = _T("\\\\.\\pipe\\tignear.sakura.BasicShellContext.out.");
		auto str_process_id = stdex::to_tstring(GetCurrentProcessId());
		auto str_process_cnt = stdex::to_tstring(m_process_count);
		out_pipename += str_process_id;
		out_pipename += _T(".");
		out_pipename += str_process_cnt;

		++m_process_count;

		s->m_out_pipe = CreateNamedPipe(out_pipename.c_str(), PIPE_ACCESS_INBOUND | PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE, 2, BUFFER_SIZE, BUFFER_SIZE, 1000, &sa);
		if (s->m_out_pipe == INVALID_HANDLE_VALUE) {
			//auto le = GetLastError();
			return false;
		}

		if (opt.current_directory) {
			
			cmdstr.insert(0,_T("-c=")+tstring(opt.current_directory)+_T(" "));
		}
		(cmdstr +=_T(" "))+= out_pipename;
		(cmdstr += _T(" ")) += stdex::to_tstring(GetCurrentProcessId());
		(cmdstr += _T(" ")) += stdex::to_tstring(reinterpret_cast<uintptr_t>(s.get()));
		SHELLEXECUTEINFO info{};
		info.lpFile = execute.c_str();
		info.lpParameters = cmdstr.c_str();
		info.cbSize = sizeof(info);
		info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC;
		info.nShow = SW_HIDE;
		s->m_iocpmgr->Attach(s->m_out_pipe);
		if (!ShellExecuteEx(&info)||!info.hProcess) {
#pragma warning(disable:4189)
			auto e = GetLastError();
			return false;
		}
		
		s->m_childProcess = info.hProcess;
		s->m_thread=std::thread([s, pid = GetProcessId(s->m_childProcess), hProcess = s->m_childProcess]() {
			WaitForSingleObject(hProcess, INFINITE);
			s->NotifyExit();
		});

		return true;
	}
	bool AdminRedirectShellContext::WorkerStart(std::shared_ptr<AdminRedirectShellContext> s) {
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
	std::shared_ptr<AdminRedirectShellContext> AdminRedirectShellContext::Create(
		HINSTANCE hinst,
		tstring cmdstr,
		std::shared_ptr<iocp::IOCPMgr> iocpmgr,
		unsigned int codepage,
		std::unordered_map<unsigned int, uint32_t> colorsys,
		std::unordered_map<unsigned int, uint32_t> color256,
		bool use_terminal_echoback,
		std::vector<std::wstring> fontmap,
		double fontsize,
		const Options& opt
	) {
		Attribute attr{};
		attr.frColor.type = ColorType::ColorSystem;
		attr.frColor.color_system = 30;
		attr.bgColor.type = ColorType::ColorSystem;
		attr.bgColor.color_system = 47;
		auto r = std::make_shared<AdminRedirectShellContext>(iocpmgr, codepage, colorsys, color256, use_terminal_echoback, fontmap, fontsize, attr);
		r->SetSystemColor(colorsys);
		r->Set256Color(color256);
		if (CreateShell(r,tstring(win::GetModuleFilePath(hinst)/"AdminRedirectShellContext.exe") ,cmdstr, opt))
		{
			if (!r->WorkerStart(r)) {
				r.reset();
				return r;
			}
			return r;
		}
		else
		{
			r.reset();
			return r;
		}
	}
	LRESULT AdminRedirectShellContext::OnMessage(LPARAM param) {

		m_hwnd=reinterpret_cast<HWND>(param);

		return 0;
	}
	AdminRedirectShellContext::~AdminRedirectShellContext() {
		if (m_childProcess) {
			CloseHandle(m_childProcess);
		}
		if (m_thread.joinable()) {
			m_thread.detach();
		}

	}
	void AdminRedirectShellContext::Terminate() {
		win32::TerminateProcessTree(win32::ProcessTree(GetProcessId(m_childProcess)));
		if (m_thread.joinable()) {
			m_thread.join();
		}
	}
}