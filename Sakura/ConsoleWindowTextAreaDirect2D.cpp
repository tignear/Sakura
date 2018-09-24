#include "stdafx.h"
#include <dwrite_1.h>
#include "ConsoleWindowTextAreaDirect2D.h"
using Microsoft::WRL::ComPtr;
namespace tignear::sakura {

	void ConsoleWindowTextAreaDirect2D::InitResource() {
		auto&& t = GetRenderTarget();
		inputtingstring_layout.Reset();
		t->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Red, 1.0f),
			&red
		);
		t->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.0f), &transparency);
	}
	std::unique_ptr<ConsoleWindowTextAreaDirect2DWithHWnd> ConsoleWindowTextAreaDirect2DWithHWnd::Create(ID2D1Factory* d2d1_f, HWND hwnd) {
		auto r = std::make_unique<ConsoleWindowTextAreaDirect2DWithHWnd>(constructor_tag{}, d2d1_f, hwnd);
		r->InitResource();
		return r;
	}
	void ConsoleWindowTextAreaDirect2DWithHWnd::InitResource() {
		Direct2DWithHWnd::InitResource();
		ConsoleWindowTextAreaDirect2D::InitResource();
	}
	
	ID2D1Factory* ConsoleWindowTextAreaDirect2DWithHWnd::GetFactory() {
		return Direct2DWithHWnd::GetFactory();
	}
	ID2D1RenderTarget* ConsoleWindowTextAreaDirect2DWithHWnd::GetRenderTarget() {
		return Direct2DWithHWnd::GetRenderTarget();
	}
	void ConsoleWindowTextAreaDirect2DWithHWnd::ReCreate() {
		return Direct2DWithHWnd::ReCreate();
	}
	void ConsoleWindowTextAreaDirect2DWithHWnd::ReSize() {
		return Direct2DWithHWnd::ReSize();
	}
}
//
