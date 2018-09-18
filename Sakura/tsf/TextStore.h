#pragma once
#include <queue>
#include <msctf.h>
#include "TextStoreLock.h"
namespace tignear::tsf {
	class TextStore:public ITextStoreACP {
		TextStoreLock m_lock;
		Microsoft::WRL::ComPtr<ITextStoreACPSink> m_sink;
		DWORD m_sinkmask = 0;
		std::atomic<DWORD> m_request_lock_async;
		std::recursive_mutex m_queue_lock;
		std::queue<std::function<void()>> m_write_queue;
		std::queue<std::function<void()>> m_read_queue;
	protected:
		Microsoft::WRL::ComPtr<ITextStoreACPSink> Sink() {
			return m_sink;
		}
		static constexpr const TsViewCookie m_viewcookie = 0x01;

		TextStoreLock& Lock() {
			return m_lock;
		}
		virtual HWND GetHWnd() = 0;
		virtual void Selection(std::function<void(LONG&, LONG&,TsActiveSelEnd&,bool&)>) = 0;
		virtual void Selection(std::function<void(LONG&, LONG&)>)=0;
		virtual void InputtingString(std::function<void(std::wstring&)>) = 0;
		virtual const std::wstring& InputtingString()const=0;
		virtual LONG SelectionStart()const=0;
		virtual LONG SelectionEnd()const=0;
		virtual TsActiveSelEnd ActiveSelEnd()const=0;
		virtual bool InterimChar()const =0;
		virtual ~TextStore() {}
	public:
		/*
		ItextStoreAcp
		*/
		HRESULT STDMETHODCALLTYPE GetWnd(
			TsViewCookie vcView,
			HWND *phwnd
		);
		HRESULT STDMETHODCALLTYPE AdviseSink(
			REFIID riid,
			IUnknown * io_unknown_cp,
			DWORD i_mask
		);
		HRESULT STDMETHODCALLTYPE UnadviseSink(IUnknown *punk);
		HRESULT STDMETHODCALLTYPE RequestLock(
			DWORD dwLockFlags,
			HRESULT *phrSession
		);
		HRESULT STDMETHODCALLTYPE GetStatus(TS_STATUS *pdcs);
		HRESULT STDMETHODCALLTYPE GetActiveView(TsViewCookie *pvcView);
		HRESULT STDMETHODCALLTYPE QueryInsert(
			LONG  acpTestStart,
			LONG  acpTestEnd,
			ULONG cch,
			LONG  *pacpResultStart,
			LONG  *pacpResultEnd
		);
		HRESULT STDMETHODCALLTYPE GetSelection(
			ULONG ulIndex,
			ULONG ulCount,
			TS_SELECTION_ACP *pSelection,
			ULONG *pcFetched
		);
		HRESULT STDMETHODCALLTYPE SetSelection(
			ULONG ulCount,
			const TS_SELECTION_ACP *pSelection
		);
		HRESULT STDMETHODCALLTYPE GetText(
			LONG       acpStart,
			LONG       acpEnd,
			WCHAR      *pchPlain,
			ULONG      cchPlainReq,
			ULONG      *pcchPlainRet,
			TS_RUNINFO *prgRunInfo,
			ULONG      cRunInfoReq,
			ULONG      *pcRunInfoRet,
			LONG       *pacpNext
		);
		HRESULT STDMETHODCALLTYPE SetText(
			DWORD         dwFlags,
			LONG          acpStart,
			LONG          acpEnd,
			const WCHAR   *pchText,
			ULONG         cch,
			TS_TEXTCHANGE *pChange
		);
		HRESULT STDMETHODCALLTYPE GetEndACP(
			LONG *pacp
		);
		HRESULT STDMETHODCALLTYPE GetScreenExt(
			TsViewCookie vcView,
			RECT         *prc
		);
		HRESULT STDMETHODCALLTYPE FindNextAttrTransition(LONG acpStart, LONG acpHalt, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags, LONG *pacpNext, BOOL *pfFound, LONG *plFoundOffset);
		HRESULT STDMETHODCALLTYPE GetTextExt(
			TsViewCookie vcView,
			LONG         acpStart,
			LONG         acpEnd,
			RECT         *prc,
			BOOL         *pfClipped
		)=0;
		HRESULT STDMETHODCALLTYPE InsertTextAtSelection(
			DWORD         dwFlags,
			const WCHAR   *pchText,
			ULONG         cch,
			LONG          *pacpStart,
			LONG          *pacpEnd,
			TS_TEXTCHANGE *pChange
		);
		HRESULT STDMETHODCALLTYPE GetFormattedText(
			LONG acpStart,
			LONG acpEnd,
			IDataObject **ppDataObject
		) {
			return E_NOTIMPL;
		}
		HRESULT STDMETHODCALLTYPE GetEmbedded(
			LONG acpPos,
			REFGUID rguidService,
			REFIID riid,
			IUnknown **ppunk
		) {
			return E_NOTIMPL;
		}
		HRESULT STDMETHODCALLTYPE InsertEmbedded(
			DWORD dwFlags,
			LONG acpStart,
			LONG acpEnd,
			IDataObject *pDataObject,
			TS_TEXTCHANGE *pChange
		) {
			return E_NOTIMPL;
		}
		HRESULT STDMETHODCALLTYPE RetrieveRequestedAttrs(
			ULONG      ulCount,
			TS_ATTRVAL *paAttrVals,
			ULONG      *pcFetched
		) {
			*pcFetched = 0;
			return S_OK;
		}
		HRESULT STDMETHODCALLTYPE RequestSupportedAttrs(
			DWORD           dwFlags,
			ULONG           cFilterAttrs,
			const TS_ATTRID *paFilterAttrs
		) {
			return E_NOTIMPL;
		}
		HRESULT STDMETHODCALLTYPE RequestAttrsAtPosition(
			LONG            acpPos,
			ULONG           cFilterAttrs,
			const TS_ATTRID *paFilterAttrs,
			DWORD           dwFlags
		) {
			return S_OK;
		}
		HRESULT STDMETHODCALLTYPE RequestAttrsTransitioningAtPosition(
			LONG acpPos,
			ULONG cFilterAttrs,
			const TS_ATTRID *paFilterAttrs,
			DWORD dwFlags) {
			return E_NOTIMPL;
		}
		HRESULT STDMETHODCALLTYPE InsertEmbeddedAtSelection(DWORD dwFlags,
			IDataObject *pDataObject,
			LONG *pacpStart,
			LONG *pacpEnd,
			TS_TEXTCHANGE *pChange
		) {
			return E_NOTIMPL;
		}
		HRESULT STDMETHODCALLTYPE GetACPFromPoint(
			TsViewCookie vcView,
			const POINT *pt,
			DWORD dwFlags,
			LONG *pacp)
		{
			return E_NOTIMPL;
		}
		HRESULT STDMETHODCALLTYPE QueryInsertEmbedded(
			const GUID *pguidService,
			const FORMATETC *pFormatEtc,
			BOOL *pfInsertable
		) {
			return E_NOTIMPL;
		}
		HRESULT InsertTextAtSelectionInternal(
			DWORD         dwFlags,
			const WCHAR   *pchText,
			ULONG         cch,
			LONG          *pacpStart,
			LONG          *pacpEnd,
			TS_TEXTCHANGE *pChange
		);
		/*
		*/
		template <class R>
		R CallWithAppLock(bool write, std::function<R()> fn) {
			if (write) {
				return m_lock.WriteLockToCallApp<R>(fn);
			}
			else {
				return m_lock.ReadLockToCallApp<R>(fn);
			}
		}
		void CallWithAppLock(bool write, std::function<void()> fn) {
			if (write) {
				return m_lock.WriteLockToCallApp(fn);
			}
			else {
				return m_lock.ReadLockToCallApp(fn);
			}
		}
		void RequestAsyncLock(DWORD);
		void PushAsyncCallQueue(bool write, std::function<void()>);
		void CallAsync();
		void UnregisterTextStore() {
			m_sink.Reset();
		}
	};
}