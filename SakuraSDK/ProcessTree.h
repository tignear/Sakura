#pragma once
#include <tlhelp32.h>
#include <Windows.h>
#include <vector>
#include <unordered_map>
#include <TlHelp32.h>
namespace tignear::win32 {
	struct ProcessTreeEntry {
		DWORD pid;
		ProcessTreeEntry& parent;
		std::list<ProcessTreeEntry> children;
		ProcessTreeEntry(DWORD pid, ProcessTreeEntry& parent) :pid(pid), parent(parent) {}
		explicit ProcessTreeEntry(DWORD pid) :pid(pid), parent(*this) {}
		explicit ProcessTreeEntry():ProcessTreeEntry(0){}//root
		ProcessTreeEntry(const ProcessTreeEntry& from) = delete;
		ProcessTreeEntry(ProcessTreeEntry&&) = default;
	};
	static const ProcessTreeEntry* FindProcessTreeEntryFromPid(const ProcessTreeEntry& root, DWORD pid) {
		if (root.pid == pid) {
			return &root;
		}
		for (auto&& e:root.children) {
			if (FindProcessTreeEntryFromPid(e, pid)) {
				return &e;
			}
		}
		return nullptr;
	}
	static ProcessTreeEntry* FindProcessTreeEntryFromPid(ProcessTreeEntry& root, DWORD pid) {
		if (root.pid == pid) {
			return &root;
		}
		for (auto&& e:root.children) {
			if (FindProcessTreeEntryFromPid(e, pid)) {
				return &e;
			}
		}
		return nullptr;
	}
	static void AppendEntry(ProcessTreeEntry& target,std::unordered_multimap < DWORD, DWORD>& buf) {
		auto range = buf.equal_range(target.pid);
		for (auto itr = range.first; itr != range.second; ++itr) {
			if (itr->first== itr->second) {
				continue;
			}
			auto&& e=target.children.emplace_back(itr->second, target);
			
			AppendEntry(e, buf);
		}
	}
	static ProcessTreeEntry ProcessTree(DWORD pid) {
		
		HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		PROCESSENTRY32 pe = {};
		pe.dwSize = sizeof(PROCESSENTRY32);
		std::unordered_multimap <DWORD,DWORD> buf;
		if (Process32First(h, &pe)) {
			do {
				buf.insert({ pe.th32ParentProcessID, pe.th32ProcessID });
			} while (Process32Next(h, &pe));
		}
		ProcessTreeEntry root(pid);
		AppendEntry(root, buf);
		CloseHandle(h);
		return root;
	}
	static void TerminateProcessTree(const ProcessTreeEntry& tree) {
		for (auto&& e: tree.children) {
			TerminateProcessTree(e);
		}
		HANDLE handle=OpenProcess(PROCESS_TERMINATE, FALSE, tree.pid);
		if (handle) {
			TerminateProcess(handle,EXIT_FAILURE);
		}
	}
}