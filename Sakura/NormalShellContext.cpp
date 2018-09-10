#include "stdafx.h"
#include "NormalShellContext.h"
#include "GetHwndFromPid.h"

using tignear::stdex::tstring;
using tignear::win32::GetHwndFromProcess;
namespace tignear::sakura {
	bool NormalShellContext::Init(tstring cmdstr, const Options& opt) {
		//http://yamatyuu.net/computer/program/sdk/base/cmdpipe1/index.html
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;
		tstring out_pipename = _T("\\\\.\\pipe\\tignear.sakura.BasicShellContext.out.");
#ifdef UNICODE
		auto str_thread_id = std::to_wstring(GetCurrentThreadId());
		auto str_process_cnt = std::to_wstring(m_process_count);
#else
		auto str_thread_id = std::to_string(GetCurrentThreadId());
		auto str_process_cnt = std::to_string(m_process_count);
#endif
		out_pipename += str_thread_id;
		out_pipename += _T(".");
		out_pipename += str_process_cnt;

		++m_process_count;

		m_out_pipe = CreateNamedPipe(out_pipename.c_str(), PIPE_ACCESS_INBOUND | PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE, 2, BUFFER_SIZE, BUFFER_SIZE, 1000, &sa);
		if (m_out_pipe == INVALID_HANDLE_VALUE) {
			//auto le = GetLastError();
			return false;
		}
		auto out_client_pipe = CreateFile(out_pipename.c_str(),
			GENERIC_WRITE | GENERIC_READ,
			0,
			&sa,
			OPEN_EXISTING,
			NULL,
			NULL);

		if (out_client_pipe == INVALID_HANDLE_VALUE) {
			return false;
		}
		PROCESS_INFORMATION pi{};
		STARTUPINFO si{};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.hStdOutput = out_client_pipe;
		si.hStdError = out_client_pipe;
		si.hStdInput = NULL;
		si.wShowWindow = SW_HIDE;
		if (opt.use_count_chars) {
			si.dwFlags |= STARTF_USECOUNTCHARS;
			si.dwXCountChars = opt.x_count_chars;
			si.dwXCountChars = opt.y_count_chars;
		}
		if (opt.use_size) {
			si.dwXSize = opt.width;
			si.dwYSize = opt.height;
		}
		auto len = cmdstr.length();
		auto cmd = std::make_unique<TCHAR[]>(len + 1);
#pragma warning(disable:4996)
		cmdstr.copy(cmd.get(), len);
#pragma warning(default:4996)
		m_iocpmgr->Attach(m_out_pipe);

		if (!CreateProcess(NULL, cmd.get(), &sa, &sa, TRUE, CREATE_NEW_CONSOLE, opt.environment, opt.current_directory, &si, &pi)) {
			auto le = GetLastError();
			OutputDebugStringW(std::to_wstring(le).c_str());
			return false;
		}
		CloseHandle(out_client_pipe);
		CloseHandle(pi.hThread);
		m_childProcess = pi.hProcess;
		std::thread th([this, pid = pi.dwProcessId, hProcess = pi.hProcess]() {
			while ((!m_hwnd) && WaitForSingleObject(hProcess, 200) == WAIT_TIMEOUT) {
				m_hwnd = GetHwndFromProcess(pid);
			}
			WaitForSingleObject(hProcess, INFINITE);
			NotifyExit();
		});
		th.detach();
		return true;
	}
	std::shared_ptr<NormalShellContext> NormalShellContext::Create(
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
		auto r = std::make_shared<NormalShellContext>(iocpmgr, codepage, colorsys, color256, use_terminal_echoback, fontmap, fontsize, attr);
		r->SetSystemColor(colorsys);
		r->Set256Color(color256);
		if (r->Init(cmdstr, opt))
		{
			if (!r->IOWorkerStart(r)) {
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
}