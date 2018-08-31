#pragma once
#include <functional>
#include <Windows.h>
#include <process.h>
#include <memory>
#include <thread>
namespace tignear::sakura::iocp {
	struct IOCPInfo {
		OVERLAPPED overlapped;
		std::function<void(DWORD)> callback;
	};
	//https://docs.microsoft.com/en-us/windows/desktop/fileio/i-o-completion-ports
	//https://www.keicode.com/windows/win06.php
	//http://d.hatena.ne.jp/paserry/20090811/p1
	class IOCPMgr {
	private:

		static unsigned int STDMETHODCALLTYPE WorkerThreadFunc(PVOID arg) {
			IOCPInfo* info;
			IOCPMgr* self = static_cast<IOCPMgr*>(arg);
			LPOVERLAPPED pol = NULL;
			DWORD cbNumberOfBytesTransferred = 0;
			ULONG_PTR uCompletionKey;
			BOOL bRet;
			while (1) {

				bRet = GetQueuedCompletionStatus(
					self->m_iocp,
					&cbNumberOfBytesTransferred,
					&uCompletionKey,
					&pol,
					INFINITE);

				if (!bRet) {
					continue;
				}


				if (COMPKEY_EXIT == uCompletionKey) {
					delete pol;
					return 0;
				}
				info = (IOCPInfo*)CONTAINING_RECORD(pol, IOCPInfo, overlapped);

				// コンテキストに応じた処理
				info->callback(cbNumberOfBytesTransferred);
				delete info;
			}
			return 0;
		}
	public:
		IOCPMgr(): m_thread_count(std::thread::hardware_concurrency()) {
			m_iocp = CreateIoCompletionPort(
				INVALID_HANDLE_VALUE,
				NULL,
				NULL,
				m_thread_count);
			if (!m_iocp) {
				std::terminate();
			}
			for (auto i = 0U; i < m_thread_count; ++i) {
				UINT uThreadId;
				_beginthreadex(
					NULL,
					NULL,
					WorkerThreadFunc,
					(void*)this,
					NULL,
					&uThreadId);
			}
		}
		~IOCPMgr() {
			for (auto i = 0U; i < m_thread_count; ++i) {
				PostQueuedCompletionStatus(m_iocp, 0, COMPKEY_EXIT,new OVERLAPPED());
			}
			CloseHandle(m_iocp);
		}
		HANDLE Attach(HANDLE handle) {
			return CreateIoCompletionPort(handle, m_iocp, NULL, 0);
		}
	private:
		HANDLE m_iocp;
		unsigned int m_thread_count;
		static constexpr ULONG_PTR COMPKEY_EXIT = 1307418660UL;
	};
}