#pragma once
#include <Windows.h>
#include <memory>
#include <functional>
#include <numeric>
#include <atomic>
#include <shared_mutex>
#include <msctf.h>
#include <optional>
#include <queue>
#include <wrl.h>
#include <chrono>
#include "ShellContext.h"
#include "Direct2D.h"
#include "TextBuilder.h"
#include "TextDrawer.h"
#include "ConsoleWindowContext.h"
#include "tsf/TextStore.h"
namespace tignear::sakura {
	class ConsoleWindowTextArea :public tsf::TextStore,public ITfContextOwnerCompositionSink {
	private:
		static constexpr UINT_PTR CallAsyncTimerId = 0x01;
		static constexpr LPCTSTR m_className = _T("ConsoleWindow.TextArea");
		static bool m_registerstate;
		struct LockHolder {
			ShellContext& context;
			LockHolder(ShellContext& context):context(context) {
				context.Lock();
			}
			~LockHolder() {
				context.Unlock();
			}
		};
		std::unique_ptr<Direct2DWithHWnd> m_d2d;
		std::unique_ptr<tignear::dwrite::TextBuilder> m_tbuilder;
		Microsoft::WRL::ComPtr<tignear::dwrite::DWriteDrawer> m_drawer;
		Microsoft::WRL::ComPtr<ITfDocumentMgr> m_docmgr;
		Microsoft::WRL::ComPtr<ITfProperty> m_attr_prop;
		Microsoft::WRL::ComPtr<ITfProperty> m_composition_prop;
		TfEditCookie m_edit_cookie;
		Microsoft::WRL::ComPtr<ITfCategoryMgr> m_category_mgr;
		Microsoft::WRL::ComPtr<ITfDisplayAttributeMgr> m_attribute_mgr;
		Microsoft::WRL::ComPtr<ITfContext> m_context;
		Microsoft::WRL::ComPtr<ITfThreadMgr> m_threadmgr;
		TfClientId m_clientId;
		HINSTANCE m_hinst;
		HWND m_parentHwnd;
		HWND m_textarea_hwnd;

		bool m_caret_display;
		bool m_fast_blink_display;
		bool m_slow_blink_display;
		std::chrono::steady_clock::time_point m_caret_update_time;
		std::chrono::steady_clock::time_point m_fast_blink_update_time;
		std::chrono::steady_clock::time_point m_slow_blink_update_time;
		ULONG m_ref_cnt = 0;
		std::shared_ptr<cwnd::Context> m_console;
		LONG m_composition_start_pos;
		std::wstring m_last_composition_string;
		void Init(int x, int y, int w, int h, HMENU m, ID2D1Factory* d2d_f, IDWriteFactory* dwrite_f, std::shared_ptr<tignear::sakura::cwnd::Context>);
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		ConsoleWindowTextArea() {}
		void OnSetFocus();
		void OnPaint();
		void OnSize();
		void OnChar(WPARAM);
		void OnTimer();
		void OnKeyDown(WPARAM);
		void UpdateText();
		void CaretUpdate();
		void BlinkUpdate();
		void ConfirmCommand();
		bool UseTerminalEchoBack();
		void Selection(std::function<void(LONG&,LONG&, TsActiveSelEnd&,bool&)>) override;
		void Selection(std::function<void(LONG&, LONG&)>)override;
		void InputtingString(std::function<void(std::wstring&)>)override;
		const std::wstring& InputtingString()const override;
		LONG SelectionStart()const override;
		LONG SelectionEnd()const override;
		TsActiveSelEnd ActiveSelEnd()const override;
		bool InterimChar()const override;
		FLOAT m_linespacing;
		FLOAT m_baseline;
		uintptr_t m_layout_change_listener_removekey;
		uintptr_t m_text_change_listener_removekey;

	public:
		// void OnSize(ConsoleWindow*, LPARAM);
		static bool RegisterConsoleWindowTextAreaClass(HINSTANCE hinst);//call once.but automatic call when create.
		static void Create(HINSTANCE, HWND, int x, int y, unsigned int w, unsigned int h, HMENU, ITfThreadMgr* threadmgr, TfClientId, ITfCategoryMgr* cate_mgr, ITfDisplayAttributeMgr* attr_mgr, ID2D1Factory*, IDWriteFactory*, std::shared_ptr<tignear::sakura::cwnd::Context> console, ConsoleWindowTextArea**);
		inline static void Create(HWND parent, int x, int y,unsigned int w,unsigned int h, HMENU menu, ITfThreadMgr* threadmgr, TfClientId cid, ITfCategoryMgr* cate_mgr, ITfDisplayAttributeMgr* attr_mgr, ID2D1Factory* d2d_f, IDWriteFactory* dwrite_f, std::shared_ptr<tignear::sakura::cwnd::Context> console,ConsoleWindowTextArea** pr) {
			return Create(reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parent, GWLP_HINSTANCE)), parent, x, y, w, h, menu, threadmgr, cid, cate_mgr, attr_mgr, d2d_f, dwrite_f, console,pr);
		}
		const HWND GetParentHwnd()
		{
			return m_parentHwnd;
		}
		HWND GetHWnd()override
		{
			return m_textarea_hwnd;
		}

		ULONG STDMETHODCALLTYPE AddRef()override {
			++m_ref_cnt;
			return m_ref_cnt;
		}
		ULONG STDMETHODCALLTYPE Release()override {
			m_ref_cnt--;
			if (m_ref_cnt <= 0) {
				delete this;
				return 0;
			}
			return m_ref_cnt;
		}
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv)override {
			*ppv = NULL;
			if (riid == IID_IUnknown || riid == IID_ITextStoreACP) {
				*ppv = dynamic_cast<ITextStoreACP*>(this);
				AddRef();
				return S_OK;
			}
			else if (riid == IID_ITfContextOwnerCompositionSink) {
				*ppv = dynamic_cast<ITfContextOwnerCompositionSink*>(this);
				AddRef();
				return S_OK;
			}
			else {
				ppv = nullptr;
				return E_NOINTERFACE;
			}
		}
		HRESULT STDMETHODCALLTYPE GetTextExt(
			TsViewCookie vcView,
			LONG         acpStart,
			LONG         acpEnd,
			RECT         *prc,
			BOOL         *pfClipped
		);
		/*
		ITfContextOwnerCompositionSink
		*/
		STDMETHODIMP OnStartComposition(
			ITfCompositionView *pComposition,
			BOOL *pfOk
		);
		STDMETHODIMP OnEndComposition(ITfCompositionView *pComposition);
		STDMETHODIMP OnUpdateComposition(
			ITfCompositionView *pComposition,
			ITfRange *pRangeNew
		);
		/*
		original functions
		*/
		void SetConsoleContext(std::shared_ptr<cwnd::Context> console);

		~ConsoleWindowTextArea() {
			m_docmgr->Pop(0);
		}
		void SetViewPosition(size_t t) {
			m_console->shell->SetViewStart(t);
		}
		D2D1_SIZE_F GetAreaDip() {
			return m_d2d->GetRenderTarget()->GetSize();
		}

		float GetTextWidthDip() {
			throw std::runtime_error("not impl");//TODO
		}

		size_t GetPageSize() {
			return static_cast<size_t>(GetAreaDip().height / m_linespacing);
		}
	};
}
