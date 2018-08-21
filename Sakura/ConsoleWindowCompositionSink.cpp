#include "stdafx.h"
#include "ConsoleWindow.h"
#include "FailToThrow.h"
using tignear::sakura::ConsoleWindow;
using Microsoft::WRL::ComPtr;
HRESULT ConsoleWindow::OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk) {
	OutputDebugString(_T("TSF:OnStartComposition\n"));
	*pfOk = TRUE;
	ComPtr<ITfRange> range;
	pComposition->GetRange(&range);
	ComPtr<ITfRangeACP> rangeAcp;
	range.As(&rangeAcp);
	LONG  len;
	rangeAcp->GetExtent(&m_composition_start_pos, &len);
	for (auto i = 0; i < len; i++) {
		m_console->shell->InputKey(VK_BACK);
	}
	return S_OK;
}
HRESULT ConsoleWindow::OnUpdateComposition(ITfCompositionView *pComposition, ITfRange *pRangeNew) {
	OutputDebugString(_T("TSF:OnUpdateComposition\n"));
	ComPtr<IEnumTfRanges> enumRanges;
	FailToThrowHR(m_composition_prop->EnumRanges(m_edit_cookie, &enumRanges, NULL));
	ComPtr<ITfRange> range;
	while (enumRanges->Next(1, &range, NULL)==S_OK) {
		VARIANT var;
		VariantInit(&var);
		FailToThrowHR(m_composition_prop->GetValue(m_edit_cookie, range.Get(), &var));
		if (var.boolVal) {
			ComPtr<ITfRangeACP> rangeAcp;
			LONG start, len;
			range.As(&rangeAcp);
			rangeAcp->GetExtent(&start, &len);
			std::wstring buf(len, L'\0');
			ULONG cnt;
			rangeAcp->GetText(m_edit_cookie,0,buf.data(),len,&cnt);
			OutputDebugStringW(buf.c_str());
			m_last_composition_string = buf;
			VariantClear(&var);
			break;
		}
		VariantClear(&var);
	}
	UpdateText();
	return S_OK;
}

HRESULT ConsoleWindow::OnEndComposition(ITfCompositionView *pComposition) {
	OutputDebugString(_T("TSF:OnEndComposition\n"));
	ComPtr<ITfRange> range;
	pComposition->GetRange(&range);
	ComPtr<ITfRangeACP> rangeAcp;
	range.As(&rangeAcp);
	LONG start, len;
	rangeAcp->GetExtent(&start, &len);
	if (start != m_composition_start_pos||len!=0) {
		m_console->shell->InputString(m_last_composition_string);
		if (!UseTerminalEchoBack()) {
			PushAsyncCallQueue(true, [this]() {
				auto oldend = static_cast<LONG>(InputtingString().length());
				InputtingString().clear();
				SelectionStart() = 0;
				SelectionEnd() = 0;
				TS_TEXTCHANGE tc = { 0,oldend,0 };
				m_sink->OnTextChange(0,&tc);
			});
		}

	}
	m_last_composition_string.clear();
	return S_OK;
}
