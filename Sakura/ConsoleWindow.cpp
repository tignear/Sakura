#include "stdafx.h"
#include "ConsoleWindow.h"
#include "FailToThrow.h"
using tignear::sakura::ConsoleWindow;
using tignear::sakura::cwnd::Context;
std::unique_ptr<ConsoleWindow> ConsoleWindow::Create(HINSTANCE hinst, HWND pwnd, int x, int y, int w, int h, HMENU hmenu, ITfThreadMgr* threadmgr, TfClientId cid, ITfCategoryMgr* cate_mgr, ITfDisplayAttributeMgr* attr_mgr, ID2D1Factory* d2d_f, IDWriteFactory* dwrite_f) {
	auto r = std::make_unique<ConsoleWindow>();
	r->m_hinst = hinst;
	FailToThrowB(RegisteConsoleWindowrClass(hinst));
	r->m_parent_hwnd = pwnd;
	CreateWindowEx(0, m_classname, NULL, WS_OVERLAPPED | WS_CHILD | WS_VISIBLE, x, y, w, h, pwnd,hmenu, hinst, r.get());
	ConsoleWindowTextArea::Create(hinst,r->m_hwnd, 0, 0, w, h, m_hmenu_textarea, threadmgr, cid, cate_mgr, attr_mgr, d2d_f, dwrite_f, &(r->m_textarea));
	return r;
 }
bool ConsoleWindow::RegisteConsoleWindowrClass(HINSTANCE hinst) {
	if (m_registerstate) return true;
	WNDCLASSEX wcex;

	ZeroMemory((LPVOID)&wcex, sizeof(WNDCLASSEX));

	//ウィンドウクラスを登録
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hinst;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = m_classname;
	wcex.hIconSm = NULL;
	if (FAILED(RegisterClassEx(&wcex)))return false;
	return true;
}
LRESULT CALLBACK ConsoleWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static ConsoleWindow* self;
	switch (message)
	{
	case WM_CREATE:
		self=static_cast<ConsoleWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		self->m_hwnd = hwnd;
		self->OnCreate();
		break;
	case WM_SETFOCUS:
		if (self->m_textarea) {
			if (SetFocus(self->m_textarea->GetHWnd()) == NULL) {
				break;
			}
		}
		break;
	case WM_SIZING:
	case WM_SIZE:
		if (self->m_textarea) {
			RECT rect;
			GetClientRect(self->GetHWnd(), &rect);
			SetWindowPos(self->m_textarea->GetHWnd(), 0, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOOWNERZORDER);
		}
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}
void ConsoleWindow::OnCreate() {

}
HWND ConsoleWindow::GetHWnd() {
	return m_hwnd;
}
void ConsoleWindow::SetContext(std::shared_ptr<Context> p) {
	m_textarea->SetConsoleContext(std::move(p));
}
//static fields
bool ConsoleWindow::m_registerstate = false;