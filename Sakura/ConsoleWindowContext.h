#pragma once
#include <Windows.h>
#include <msctf.h>
#include <memory>
#include "ShellContext.h"
namespace tignear::sakura::cwnd {
	struct TextAreaContext {
		//friend ConsoleWindowTextArea;
		TextAreaContext(bool use_terminal_echoback) :
			inputarea_selection_start(0),
			inputarea_selection_end(0),
			selend(TS_AE_NONE),
			allarea_selection_start(0),
			allarea_selection_end(0),
			interim_char(false),
			use_terminal_echoback(use_terminal_echoback)
		{}
		LONG inputarea_selection_start;
		LONG inputarea_selection_end;
		TsActiveSelEnd selend;
		LONG allarea_selection_start;
		LONG allarea_selection_end;
		std::wstring input_string;
		bool interim_char;
		bool use_terminal_echoback;
	};
	struct Context {
		Context(std::shared_ptr<ShellContext> shell, bool use_terminal_echoback) :shell(shell), textarea_context(use_terminal_echoback) {

		}
		std::shared_ptr<ShellContext> shell;
		TextAreaContext textarea_context;
	};
}
