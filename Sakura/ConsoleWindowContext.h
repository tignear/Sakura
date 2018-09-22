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
			allarea_selection_start(0),
			allarea_selection_end(0),
			interim_char(false)
		{}
		LONG inputarea_selection_start;
		LONG inputarea_selection_end;
		TsActiveSelEnd selend;
		LONG allarea_selection_start;
		LONG allarea_selection_end;
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
