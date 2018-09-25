#pragma once
#include <Windows.h>
#include <msctf.h>
#include <memory>
#include "ShellContext.h"
namespace tignear::sakura::cwnd {
	struct TextAreaContext {
		//friend ConsoleWindowTextArea;
		TextAreaContext() :
			inputarea_selection_start(0),
			inputarea_selection_end(0),
			selend(TS_AE_NONE),
			interim_char(false)
		{}
		size_t selection_start_line;
		size_t selection_end_line;
		LONG selection_start_x;
		LONG selection_end_x;
		std::wstring selection_buffer;
		LONG inputarea_selection_start;
		LONG inputarea_selection_end;
		TsActiveSelEnd selend;
		std::wstring input_string;
		bool interim_char;
	};
	class Context {

	public:
		Context(std::shared_ptr<ShellContext> shell) :shell(shell), textarea_context() {

		}
		~Context() {
			//shell->RemoveExitListener(exitListener_removekey);
		}
		std::shared_ptr<ShellContext> shell;
		TextAreaContext textarea_context;
	};
}
