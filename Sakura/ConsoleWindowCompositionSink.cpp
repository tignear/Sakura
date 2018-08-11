#include "stdafx.h"
#include "ConsoleWindow.h"
#include "FailToThrow.h"
using tignear::sakura::ConsoleWindow;
using Microsoft::WRL::ComPtr;
HRESULT ConsoleWindow::OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk) {
	OutputDebugString(_T("TSF:OnStartComposition\n"));
	*pfOk = TRUE;
	UpdateText();
	return S_OK;
}
HRESULT ConsoleWindow::OnUpdateComposition(ITfCompositionView *pComposition, ITfRange *pRangeNew) {
	OutputDebugString(_T("TSF:OnUpdateComposition\n"));
	UpdateText();
	return S_OK;
}

HRESULT ConsoleWindow::OnEndComposition(ITfCompositionView *pComposition) {
	OutputDebugString(_T("TSF:OnEndComposition\n"));
	UpdateText();
	return S_OK;
}
