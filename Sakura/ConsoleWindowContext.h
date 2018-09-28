#pragma once
#include <Windows.h>
#include <msctf.h>
#include <memory>
#include <dwrite_1.h>
#include <wrl.h>
#include "ShellContext.h"
namespace tignear::sakura::cwnd {
	struct LineResouce{
		Microsoft::WRL::ComPtr<IDWriteTextLayout1> layout;
		long selection_start=0;
		long selection_end=0;
		LineResouce(Microsoft::WRL::ComPtr<IDWriteTextLayout1> lay):layout(lay) {

		}
	};
	class SelectionMgr {
		std::deque<wchar_t> selection_buf;
		std::shared_ptr<ShellContext> shell;
		std::function<void()> callback = []() {};
		long now_cur_line = 0;//relative
		long now_cur_x = 0;
		bool to_right = 0;
		std::wstring extractStr(ShellContext::attrtext_line& line) {
			std::wstring r;
			for (auto& e : line) {
				r += e.textW();
			}
			return r;
		}
	public:
		SelectionMgr(std::shared_ptr<ShellContext> s) :shell(s) {
			s->AddTextChangeListener([this](auto&&,auto&&) {
				clear();
			});
		}
		void clear() {
			selection_buf.clear();
			for (auto&& e : shell->GetAll()) {
				auto&& p = std::static_pointer_cast<LineResouce>(e.resource());
				if (p&&(p->selection_start != 0 || p->selection_end != 0)) {
					p->selection_start = 0;
					p->selection_end = 0;
					p->layout.Reset();
				}
			}
			callback();
		}
		void left() {
			clear();
		}
		void shift_left() {
			if (selection_buf.empty()) {
				to_right = false;
				now_cur_x = static_cast<long>(shell->GetCursorXWStringPos());
				auto p = std::static_pointer_cast<LineResouce>(shell->GetCursorY().resource());
				p->selection_start = now_cur_x;
				p->selection_end = now_cur_x;
				p->layout.Reset();
			}
			if (!to_right) {
				if (now_cur_x == 0) {
					auto itr = shell->GetCursorYItr();
					--now_cur_line;
					std::advance(itr, now_cur_line);
					auto str = extractStr(*itr);
					now_cur_x = static_cast<long>(str.length());
					auto p = std::static_pointer_cast<LineResouce>(itr->resource());
					p->selection_start = now_cur_x;
					p->selection_end = now_cur_x;
					p->layout.Reset();
					selection_buf.push_front('\n');
				}
				else {
					--now_cur_x;
					auto itr = shell->GetCursorYItr();
					std::advance(itr, now_cur_line);
					auto p = std::static_pointer_cast<LineResouce>(itr->resource());
					p->selection_start = now_cur_x;
					p->layout.Reset();
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
					auto p = std::static_pointer_cast<LineResouce>(itr->resource());
					p->selection_end = now_cur_x;
					p->layout.Reset();
				}
				else {
					--now_cur_x;
					std::static_pointer_cast<LineResouce>(shell->GetCursorY().resource())->selection_end = now_cur_x;

				}
			}
			callback();
		}

		void right() {
			clear();
		}
		void shift_right() {
			if (selection_buf.empty()) {
				to_right = true;
				now_cur_x = static_cast<long>(shell->GetCursorXWStringPos());
				auto p = std::static_pointer_cast<LineResouce>(shell->GetCursorY().resource());
				p->selection_start = now_cur_x;
				p->selection_end = now_cur_x;
			}
			auto itr = shell->GetCursorYItr();
			std::advance(itr, now_cur_line);
			auto str = extractStr(*itr);
			if (to_right) {
				if (now_cur_x == str.length()) {
					selection_buf.push_back(L'\n');
					now_cur_x = 0;
					auto p = std::static_pointer_cast<LineResouce>(shell->GetCursorY().resource());
					p->selection_start = now_cur_x;
					p->selection_end = now_cur_x;
					++now_cur_line;
				}
				else {
					selection_buf.push_back(str[now_cur_x]);
					++now_cur_x;
					auto p = std::static_pointer_cast<LineResouce>(shell->GetCursorY().resource());
					p->selection_end = now_cur_x;
				}
			}
			else
			{
				selection_buf.pop_front();
				if (now_cur_x == str.length()) {
					++now_cur_line;
					now_cur_x = 0;
					auto p = std::static_pointer_cast<LineResouce>(shell->GetCursorY().resource());
					p->selection_start = now_cur_x;
					p->selection_end = now_cur_x;
				}
				else {
					++now_cur_x;
					auto p = std::static_pointer_cast<LineResouce>(shell->GetCursorY().resource());
					p->selection_start = now_cur_x;
				}

			}
			callback();
		}
		std::wstring get() {
			std::wstring wstr;
			wstr.reserve(selection_buf.size());
			for (auto&& e : selection_buf) {
				wstr += e;
			}
			return wstr;
		}
		void setSelectionChangeCallback(std::function<void()> cb) {
			callback = cb;
		}
	};
	struct TextAreaContext {
		//friend ConsoleWindowTextArea;
		TextAreaContext(std::shared_ptr<ShellContext> s) :
			inputarea_selection_start(0),
			inputarea_selection_end(0),
			selend(TS_AE_NONE),
			interim_char(false),
			sel_mgr(s)
		{}
		SelectionMgr sel_mgr;
		LONG inputarea_selection_start;
		LONG inputarea_selection_end;
		TsActiveSelEnd selend;
		std::wstring input_string;
		bool interim_char;
	};
	class Context {

	public:
		std::shared_ptr<ShellContext> shell;
		TextAreaContext textarea_context;
		Context(std::shared_ptr<ShellContext> shell) :shell(shell), textarea_context(shell) {

		}
		~Context() {
			//shell->RemoveExitListener(exitListener_removekey);
		}

	};
}
