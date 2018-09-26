#pragma once
#include <Windows.h>
#include <msctf.h>
#include <memory>
#include "ConsoleWindowTextAreaSelectionMgr.h"
#include "ShellContext.h"
namespace tignear::sakura::cwnd {
	enum class Direction {
		Left,Right,None
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
