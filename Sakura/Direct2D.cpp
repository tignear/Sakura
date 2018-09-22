#include "stdafx.h"
#include "failtothrow.h"
#include "Direct2D.h"
using Microsoft::WRL::ComPtr;
using tignear::Direct2DWithHWnd;


void tignear::Direct2DWithHWnd::InitResource()
{
	D2D1_RENDER_TARGET_PROPERTIES prop{ D2D1_RENDER_TARGET_TYPE_DEFAULT ,{ DXGI_FORMAT_UNKNOWN ,D2D1_ALPHA_MODE_UNKNOWN },0,0,D2D1_RENDER_TARGET_USAGE_NONE,D2D1_FEATURE_LEVEL_10 };
	RECT rc;
	GetClientRect(m_hwnd,&rc);
	D2D1_HWND_RENDER_TARGET_PROPERTIES prop2{ m_hwnd,{ static_cast<UINT32>(rc.right - rc.left),
		static_cast<UINT32>(rc.bottom - rc.top) },D2D1_PRESENT_OPTIONS_NONE };
	FailToThrowHR(m_factory->CreateHwndRenderTarget(&prop, &prop2, &m_target));
}
std::unique_ptr<Direct2DWithHWnd> Direct2DWithHWnd::Create(ID2D1Factory* d2d1_f,HWND hwnd){
	FailToThrowB(hwnd != NULL);
	auto d = std::make_unique<Direct2DWithHWnd>(constructor_tag{},d2d1_f,hwnd);
	d->InitResource();
	return d;
}
void Direct2DWithHWnd::ReCreate() {
	InitResource();
}
void Direct2DWithHWnd::ReSize() {
	RECT rc;
	GetClientRect(m_hwnd, &rc);
	GetHwndRenderTarget()->Resize({static_cast<UINT32>(rc.right - rc.left),static_cast<UINT32>(rc.bottom - rc.top )});
}