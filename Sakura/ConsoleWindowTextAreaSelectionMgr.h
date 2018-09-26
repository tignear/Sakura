#pragma once
#include <string>
#include <deque>
#include <ShellContext.h>
namespace tignear::sakura::cwnd {
	class SelectionMgr {
		std::deque<wchar_t> selection_buf;
		std::shared_ptr<ShellContext> shell;
		long now_cur_line=0;//relative
		long now_cur_x=0;
		bool to_right=0;
		std::wstring extractStr(ShellContext::attrtext_line& line) {
			std::wstring r;
			for (auto& e : line) {
				r+=e.textW();
			}
			return r;
		}
	public:
		SelectionMgr(std::shared_ptr<ShellContext> s):shell(s) {

		}
		void left() {
			selection_buf.clear();
		}
		void shift_left() {
			if (selection_buf.empty()) {
				to_right = false;
				now_cur_x =static_cast<long>(shell->GetCursorXWStringPos());
			}
			if (!to_right) {
				if (now_cur_x == 0) {
					auto itr = shell->GetCursorYItr();
					--now_cur_line;
					std::advance(itr, now_cur_line);
					auto str = extractStr(*itr);
					now_cur_x =static_cast<long>(str.length());
					selection_buf.push_front('\n');
				}
				else {
					--now_cur_x;
					auto itr = shell->GetCursorYItr();
					std::advance(itr, now_cur_line);
					auto str = extractStr(*itr);
					selection_buf.push_front(str[now_cur_x]);
				}
			}
			else {
				selection_buf.pop_back();
				if (now_cur_x == 0) {
					--now_cur_line;
					auto itr = shell->GetCursorYItr();
					std::advance(itr, now_cur_line);
					auto str = extractStr(*itr);
					now_cur_x = static_cast<long>(str.length());
				}
				else {
					--now_cur_x;
				}
			}
		}
		void right() {
			selection_buf.clear();
		}
		void shift_right() {
			if (selection_buf.empty()) {
				to_right = true;
				now_cur_x = static_cast<long>(shell->GetCursorXWStringPos());
			}
			auto itr = shell->GetCursorYItr();
			std::advance(itr, now_cur_line);
			auto str = extractStr(*itr);
			if (to_right) {

				if (now_cur_x == str.length()) {
					selection_buf.push_back(L'\n');
					now_cur_x = 0;
					++now_cur_line;
				}
				else {
					selection_buf.push_back(str[now_cur_x]);
					++now_cur_x;
				}
			}
			else
			{
				selection_buf.pop_front();
				if (now_cur_x == str.length()) {
					++now_cur_line;
					now_cur_x = 0;
				}
				else {
					++now_cur_x;

				}
			}
		}
		std::wstring_view get() {
			std::wstring wstr;
			wstr.reserve(selection_buf.size());
			for (auto&& e:selection_buf) {
				wstr += e;
			}
			return wstr;
		}
	};
}