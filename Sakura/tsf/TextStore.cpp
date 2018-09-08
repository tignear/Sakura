#include "stdafx.h"
#include <olectl.h>
#include <thread>
#include <algorithm>
#include <msctf.h>
#include <wrl.h>
#include "TextStore.h"
#include "../FailToThrow.h"
#undef min

using Microsoft::WRL::ComPtr;
using tignear::FailToThrowHR;
namespace tignear::tsf {


	HRESULT STDMETHODCALLTYPE TextStore::GetWnd(
		TsViewCookie vcView,
		HWND         *phwnd
	) {
		if (vcView == m_viewcookie) {
			*phwnd = GetHWnd();
			return S_OK;
		}
		else {
			return E_INVALIDARG;
		}
	}
	HRESULT TextStore::AdviseSink(
		REFIID   riid,
		IUnknown *punk,
		DWORD    dwMask
	)
	{
		if (riid != IID_ITextStoreACPSink)return E_INVALIDARG;
		if (m_sink) {
			//ä˘Ç…ìoò^Ç≥ÇÍÇƒÇ¢ÇÈÅB
			if (m_sink.Get() == punk) {
				//ìoò^Ç≥ÇÍÇƒÇÈÇÃÇ∆Ç®ÇÒÇ»Ç∂
				m_sinkmask = dwMask;
				return S_OK;
			}
			else
			{
				//ìoò^Ç≥ÇÍÇƒÇÈÇÃÇ∆à·Ç§ÇÃ
				return CONNECT_E_ADVISELIMIT;
			}
		}
		else
		{
			//ìoò^
			auto hr = punk->QueryInterface(IID_PPV_ARGS(&m_sink));
			if (FAILED(hr))return E_INVALIDARG;
			m_sinkmask = dwMask;
			return S_OK;
		}
	}
	HRESULT TextStore::UnadviseSink(
		IUnknown *punk
	) {
		if (m_sink.Get() == punk) {
			m_sink.Reset();
			m_sinkmask = NULL;
			return S_OK;
		}
		else
		{
			return CONNECT_E_NOCONNECTION;
		}
	}
	HRESULT TextStore::RequestLock(DWORD dwLockFlags, HRESULT *phrSession)
	{
		OutputDebugString(_T("TSF:RequestLock\n"));

		if (!m_sink)return E_UNEXPECTED;
		if ((dwLockFlags&TS_LF_SYNC) == TS_LF_SYNC) {
			OutputDebugString(_T("TSF:SyncLock\n"));

			auto wr = (dwLockFlags&TS_LF_READWRITE) == TS_LF_READWRITE;
			//ìØä˙ÉçÉbÉN
			if (wr) {
				std::optional<HRESULT> r = m_lock.TryWriteLockToCallTS([this, dwLockFlags]()->HRESULT {
					return m_sink->OnLockGranted(dwLockFlags);
				});
				*phrSession = r.value_or(TS_E_SYNCHRONOUS);
			}
			else {
				std::optional<HRESULT> r = m_lock.TryReadLockToCallTS([this, dwLockFlags]() -> HRESULT {
					return m_sink->OnLockGranted(dwLockFlags);
				});
				*phrSession = r.value_or(TS_E_SYNCHRONOUS);
			}


		}
		else
		{
			//îÒìØä˙ÉçÉbÉN
			OutputDebugString(_T("TSF:ASyncLock\n"));
			RequestAsyncLock(dwLockFlags);

			*phrSession = TS_S_ASYNC;
		}
		CallAsync();
		return S_OK;
	}
	HRESULT TextStore::GetStatus(TS_STATUS *pdcs)
	{
		OutputDebugString(_T("TSF:GetStatus\n"));

		if (pdcs == 0) {
			return E_INVALIDARG;
		}
		pdcs->dwDynamicFlags = 0;
		pdcs->dwStaticFlags = TS_SS_NOHIDDENTEXT;
		return S_OK;
	}
	HRESULT TextStore::GetActiveView(
		TsViewCookie *pvcView
	)
	{
		OutputDebugString(_T("TSF:GetActiveView\n"));

		*pvcView = 1;
		return S_OK;
	}
	HRESULT TextStore::QueryInsert(
		LONG  acpTestStart,
		LONG  acpTestEnd,
		ULONG cch,
		LONG  *pacpResultStart,
		LONG  *pacpResultEnd
	)
	{
		OutputDebugString(_T("TSF:QueryInsert\n"));

		if (acpTestStart < 0
			|| acpTestStart > acpTestEnd
			|| acpTestEnd > static_cast<LONG>(InputtingString().length()))
		{
			return  E_INVALIDARG;
		}
		else
		{
			*pacpResultStart = acpTestStart;
			*pacpResultEnd = acpTestEnd;
			return S_OK;
		}
	}
	HRESULT TextStore::GetSelection(ULONG ulIndex, ULONG ulCount, TS_SELECTION_ACP *pSelection, ULONG *pcFetched)
	{
		OutputDebugString(_T("TSF:GetSelection\n"));
		if (!m_lock.IsLock(false))return TS_E_NOLOCK;
		if (pcFetched == 0)
		{
			return E_INVALIDARG;
		}
		*pcFetched = 0;
		if (pSelection == 0)
		{
			return E_INVALIDARG;
		}

		if (ulIndex == TF_DEFAULT_SELECTION)
		{
			ulIndex = 0;
		}
		else if (ulIndex > 1)
		{
			return E_INVALIDARG;
		}

		pSelection[0].acpStart = SelectionStart();
		pSelection[0].acpEnd = SelectionEnd();
		pSelection[0].style.fInterimChar = InterimChar();
		if (InterimChar())
		{
			pSelection[0].style.ase = TS_AE_NONE;
			pSelection[0].style.ase = TS_AE_NONE;
		}
		else
		{
			pSelection[0].style.ase = ActiveSelEnd();
		}
		*pcFetched = 1;
		return S_OK;
	}
	HRESULT TextStore::SetSelection(ULONG ulCount, const TS_SELECTION_ACP *pSelection)
	{
		OutputDebugString(_T("TSF:SetSelection\n"));

		if (!m_lock.IsLock(true)) {
			return TS_E_NOLOCK;
		}
		if (pSelection == 0) {
			return E_INVALIDARG;
		}

		if (ulCount > 1) {
			return E_INVALIDARG;
		}
		if (pSelection->acpStart < 0) {
			return E_INVALIDARG;

		}
		if (static_cast<size_t>(pSelection->acpEnd) > InputtingString().length()) {
			return E_INVALIDARG;
		}
		Selection([pSelection](auto& start,auto& end,auto& ase,auto& ic) {
			start = pSelection->acpStart;
			end = pSelection->acpEnd;
			ic = pSelection->style.fInterimChar;
			ase= pSelection->style.ase;
		}
		);
		//UpdateText();
		return S_OK;
	}
	HRESULT TextStore::GetText(
		LONG       acpStart,
		LONG       acpEnd,
		WCHAR      *pchPlain,
		ULONG      cchPlainReq,
		ULONG      *pcchPlainRet,
		TS_RUNINFO *prgRunInfo,
		ULONG      cRunInfoReq,
		ULONG      *pcRunInfoRet,
		LONG       *pacpNext
	)
	{
		OutputDebugString(_T("TSF:GetText\n"));

		if (!m_lock.IsLock(false)) {
			return TS_E_NOLOCK;
		}
		if ((cchPlainReq == 0) && (cRunInfoReq == 0))
		{
			return S_OK;
		}

		if (acpEnd == -1)
			acpEnd = static_cast<LONG>(InputtingString().length());

		acpEnd = std::min(acpEnd, acpStart + (int)cchPlainReq);
		if (acpStart != acpEnd) {
#pragma warning(push)
#pragma warning(disable:4996)
			InputtingString().copy(pchPlain, acpEnd - acpStart, acpStart);
#pragma warning(pop)
		}

		*pcchPlainRet = acpEnd - acpStart;
		if (cRunInfoReq)
		{
			prgRunInfo[0].uCount = acpEnd - acpStart;
			prgRunInfo[0].type = TS_RT_PLAIN;
			*pcRunInfoRet = 1;
		}

		*pacpNext = acpEnd;
		OutputDebugStringW(pchPlain);
		OutputDebugString(_T("\n"));
		return S_OK;
	}
	HRESULT TextStore::SetText(
		DWORD         dwFlags,
		LONG          acpStart,
		LONG          acpEnd,
		const WCHAR   *pchText,
		ULONG         cch,
		TS_TEXTCHANGE *pChange
	)
	{
		OutputDebugString(_T("TSF:SetText@"));
		OutputDebugStringW(pchText);
		OutputDebugString(_T("&"));
		OutputDebugStringW(std::to_wstring(cch).c_str());
		OutputDebugString(_T("\n"));
		LONG acpRemovingEnd;
		HRESULT r;
		InputtingString([this,&acpRemovingEnd,&r,acpStart,acpEnd, pChange, pchText,cch](auto& str) {
			if (acpStart > (LONG)str.length()) {
				r=E_INVALIDARG;
				return;
			}

			acpRemovingEnd = std::min(acpEnd, (LONG)str.length());
			OutputDebugStringW((L"erase count:" + std::to_wstring(acpRemovingEnd - acpStart) + L"\n").c_str());
			str.erase(acpStart, acpRemovingEnd - acpStart);
			str.insert(acpStart, pchText, cch);
			pChange->acpStart = acpStart;
			pChange->acpOldEnd = acpEnd;
			pChange->acpNewEnd = acpStart + cch;
			Selection([acpStart,cch](auto& start,auto& end) {
				start = acpStart;
				end = acpStart + cch;
			});
			r=S_OK;

		});
		return r;

		//UpdateText();
	}
	HRESULT TextStore::GetEndACP(
		LONG *pacp
	)
	{
		OutputDebugString(_T("TSF:GetEndACP\n"));

		if (!m_lock.IsLock(false)) {
			return TS_E_NOLOCK;
		}

		*pacp = SelectionEnd();
	
		return S_OK;
	}
	HRESULT TextStore::FindNextAttrTransition(LONG acpStart, LONG acpHalt, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags, LONG *pacpNext, BOOL *pfFound, LONG *plFoundOffset)
	{
		OutputDebugString(_T("TSF:FindNextAttrTransition\n"));

		*pacpNext = 0;
		*pfFound = FALSE;
		*plFoundOffset = 0;
		return S_OK;
	}
	HRESULT TextStore::GetScreenExt(
		TsViewCookie vcView,
		RECT         *prc
	) {
		OutputDebugString(_T("TSF:GetScreenExt\n"));

		GetWindowRect(GetHWnd(), prc);
		return S_OK;
	}
	
	HRESULT TextStore::InsertTextAtSelection(
		DWORD         dwFlags,
		const WCHAR   *pchText,
		ULONG         cch,
		LONG          *pacpStart,
		LONG          *pacpEnd,
		TS_TEXTCHANGE *pChange
	) {
		OutputDebugString(_T("TSF:InsertTextAtSelection"));
		auto r = InsertTextAtSelectionInternal(dwFlags, pchText, cch, pacpStart, pacpEnd, pChange);
		//UpdateText();
		return r;
	}
	HRESULT TextStore::InsertTextAtSelectionInternal(
		DWORD         dwFlags,
		const WCHAR   *pchText,
		ULONG         cch,
		LONG          *pacpStart,
		LONG          *pacpEnd,
		TS_TEXTCHANGE *pChange
	) {

		if (dwFlags & TS_IAS_QUERYONLY)
		{
			if (!m_lock.IsLock(false)) {
				return TS_E_NOLOCK;
			}
			*pacpStart = SelectionStart();
			*pacpEnd = SelectionEnd();
			if (pChange) {
				pChange->acpStart = SelectionStart();
				pChange->acpOldEnd = SelectionEnd();
				pChange->acpNewEnd = SelectionStart() + cch;
			}
		}
		else
		{
			if (!m_lock.IsLock(true)) {
				return TS_E_NOLOCK;
			}
			LONG acpStart = SelectionStart();
			LONG acpEnd = SelectionEnd();
			LONG length = acpEnd - acpStart;
			if (pChange)
			{
				pChange->acpStart = acpStart;
				pChange->acpOldEnd = acpEnd;
				pChange->acpNewEnd = acpStart + cch;
			}
			else {
				return E_INVALIDARG;
			}

			if (length != 0) {
				InputtingString([acpStart, length](auto& str) {
					str.erase(acpStart, length);
				});
				acpStart -= length;
				acpEnd -= length;
			}
			if (!pchText) {
				InputtingString([acpStart, pchText, cch](auto&str) {
					str.insert(acpStart, pchText, cch);
				}); 
			}


			if (pacpStart)
			{
				*pacpStart = acpStart;
			}

			if (pacpEnd)
			{
				*pacpEnd = acpStart + cch;
			}

			Selection([acpStart, cch](auto& start,auto& end) {
				start= acpStart;
				end = acpStart + cch;
			});

		}
		return S_OK;

	}

	void TextStore::RequestAsyncLock(DWORD dwLockFlags) {
		unsigned long expect = TS_LF_READ;
		m_request_lock_async.compare_exchange_strong(expect, dwLockFlags);
		unsigned long expect2 = 0;
		m_request_lock_async.compare_exchange_strong(expect2, dwLockFlags);
	}
	void TextStore::PushAsyncCallQueue(bool write, std::function<void()> fn) {
		std::lock_guard lock(m_queue_lock);
		if (write) {
			m_write_queue.push(fn);
		}
		else {
			m_read_queue.push(fn);
		}
	}
	void TextStore::CallAsync() {
		std::lock_guard lock(m_queue_lock);
		m_lock.TryWriteLockToCallApp([this] {
			while (!m_write_queue.empty()) {
				auto e = m_write_queue.front();
				e();
				m_write_queue.pop();
			}
		});
		m_lock.TryReadLockToCallApp([this] {
			while (!m_read_queue.empty()) {
				auto e = m_read_queue.front();
				e();
				m_read_queue.pop();
			}
		});
		auto flag = m_request_lock_async.exchange(0);
		if (flag != 0) {
			auto fn = [this, flag] {m_sink->OnLockGranted(flag); };
			if ((flag&TS_LF_READWRITE) == TS_LF_READWRITE) {
				OutputDebugString(_T("TSF:TryWriteLockToCallTS\n"));
				if (!m_lock.TryWriteLockToCallTS(fn)) {
					OutputDebugString(_T("TSF:TryWriteLockToCallTS@Failed!\n"));
					RequestAsyncLock(flag);
				}
			}
			else {
				OutputDebugString(_T("TSF:TryReadLockToCallTS\n"));
				if (!m_lock.TryReadLockToCallTS(fn)) {
					OutputDebugString(_T("TSF:TryReadLockToCallTS@Failed!\n"));
					RequestAsyncLock(flag);
				}

			}
		}
	}
}