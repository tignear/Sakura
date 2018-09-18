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
#include <tchar.h>
#include "ShellContextFactory.h"
#include "SakuraConfig.h"
#include "ShellContext.h"
#include "ConsoleWindowTextAreaDirect2D.h"
#include "TextBuilder.h"
#include "TextDrawer.h"
#include "ConsoleWindowContext.h"
#include "Dpi.h"
#include "tsf/TextStore.h"
namespace tignear::sakura {
	class ConsoleWindowTextArea :public tsf::TextStore, public ITfContextOwnerCompositionSink {
	private:
		static constexpr UINT_PTR CallAsyncTimerId = 0x01;
		static constexpr LPCTSTR m_className = _T("ConsoleWindow.TextArea");
		static bool m_registerstate;
		struct LockHolder {
			ShellContext& context;
			LockHolder(ShellContext& context) :context(context) {
				context.Lock();
			}
			~LockHolder() {
				context.Unlock();
			}
		};
		std::unique_ptr<ConsoleWindowTextAreaDirect2D> m_d2d;
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
		FLOAT m_originY = 0;
		FLOAT m_originX = 0;
		FLOAT m_text_width = 0;
		const win::dpi::Dpi& m_dpi;
		void Init(DIP x, DIP y, DIP w, DIP h, HMENU m, ID2D1Factory* d2d_f, IDWriteFactory* dwrite_f, std::shared_ptr<tignear::sakura::cwnd::Context>);
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		void OnSetFocus();
		void OnPaint();
		void DrawShellText();
		void OnSize();
		void OnChar(WPARAM);
		void OnTimer();
		void OnKeyDown(WPARAM);
		void UpdateText(ShellContext*, std::vector<ShellContext::TextUpdateInfoLine>);
		void UpdateText();
		void CaretUpdate();
		void BlinkUpdate();
		void ConfirmCommand();
		bool UseTerminalEchoBack();

		Microsoft::WRL::ComPtr<IDWriteTextLayout1> GetLayout(ShellContext::attrtext_line& line);
		std::pair<bool,Microsoft::WRL::ComPtr<IDWriteTextLayout1>> BuildLayout(ShellContext::attrtext_line& line);
		Microsoft::WRL::ComPtr<IDWriteTextLayout1> BuildCurosorYLayoutWithX();
		//Microsoft::WRL::ComPtr<IDWriteTextLayout1> GetInputtingStringLayout();
		//Microsoft::WRL::ComPtr<IDWriteTextLayout1> BuildInputtingStringLayout();
		//void ResetInputtingStringLayout();
		void Selection(std::function<void(LONG&, LONG&, TsActiveSelEnd&, bool&)>) override;
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
		explicit ConsoleWindowTextArea(const win::dpi::Dpi& dpi) :m_dpi(dpi) {}

	public:

		static bool RegisterConsoleWindowTextAreaClass(HINSTANCE hinst);//call once.but automatic call when create.
		static void Create(HINSTANCE, HWND, const win::dpi::Dpi& dpi, DIP x, DIP y, DIP w, DIP h, HMENU, ITfThreadMgr* threadmgr, TfClientId, ITfCategoryMgr* cate_mgr, ITfDisplayAttributeMgr* attr_mgr, ID2D1Factory*, IDWriteFactory*, std::shared_ptr<tignear::sakura::cwnd::Context> console, ConsoleWindowTextArea**);
		inline static void Create(HWND parent, const win::dpi::Dpi& dpi, DIP x, DIP y, DIP w, DIP h, HMENU menu, ITfThreadMgr* threadmgr, TfClientId cid, ITfCategoryMgr* cate_mgr, ITfDisplayAttributeMgr* attr_mgr, ID2D1Factory* d2d_f, IDWriteFactory* dwrite_f, std::shared_ptr<tignear::sakura::cwnd::Context> console, ConsoleWindowTextArea** pr) {
			return Create(reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parent, GWLP_HINSTANCE)), parent, dpi, x, y, w, h, menu, threadmgr, cid, cate_mgr, attr_mgr, d2d_f, dwrite_f, console, pr);
		}
		const HWND GetParentHwnd()
		{
			return m_parentHwnd;
		}
		void OnDpiChange();

		HWND GetHWnd()override
		{
			return m_textarea_hwnd;
		}

		ULONG STDMETHODCALLTYPE AddRef()override {
			++m_ref_cnt;
			return m_ref_cnt;
		}
		ULONG STDMETHODCALLTYPE Release()override {
			--m_ref_cnt;
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


		void SetViewPosition(size_t t) {
			m_console->shell->SetViewStart(t);
		}
		D2D1_SIZE_F GetAreaDip()const {
			return m_d2d->GetRenderTarget()->GetSize();
		}

		FLOAT GetTextWidthDip() {
			auto lay = BuildCurosorYLayoutWithX();
			DWRITE_TEXT_METRICS met{};
			lay->GetMetrics(&met);
			return std::max(m_text_width,  met.width);
		}
		void SetOriginX(FLOAT origX) {
			m_originX = origX;
			InvalidateRect(m_textarea_hwnd, NULL, FALSE);
		}
		FLOAT GetOriginX() {
			return m_originX;
		}
		size_t GetPageSize() {
			return static_cast<size_t>(GetAreaDip().height / m_linespacing + 1);
		}
		~ConsoleWindowTextArea() {
			OutputDebugStringA("aaa");
		}
		void UnregisterTextStore() {
			m_docmgr->Pop(0);
			m_docmgr.Reset();
			m_attribute_mgr.Reset();
			m_attr_prop.Reset();
			m_category_mgr.Reset();
			m_context.Reset();
			m_composition_prop.Reset();
			m_threadmgr.Reset();
			m_console.reset();
			TextStore::UnregisterTextStore();
		}
	};
}
namespace tignear::sakura {
	class ConsoleWindow {
		static constexpr HMENU m_hmenu_textarea=(HMENU)0x01;
		static constexpr HMENU m_hmenu_column_scrollbar =(HMENU)0x02;
		static constexpr HMENU m_hmenu_row_scrollbar = (HMENU)0x03;
		static constexpr DIP m_scrollbar_width = 15;
		
		Microsoft::WRL::ComPtr<ConsoleWindowTextArea> m_textarea;
		HWND m_parent_hwnd;
		HWND m_hwnd;
		HWND m_tab_hwnd;
		HWND m_scrollbar_column_hwnd;
		HWND m_scrollbar_row_hwnd;
		HINSTANCE m_hinst;
		static bool m_registerstate;
		static bool RegisterConsoleWindowClass(HINSTANCE hinst);
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		//void OnLayoutChange(ShellContext* shell,bool,bool);
		void UpdateScrollBar();
		std::shared_ptr<cwnd::Context> m_console;
		void SetConsoleContext(std::shared_ptr<cwnd::Context>);
		std::function<std::shared_ptr<cwnd::Context>(DIP w, DIP h)> m_getContext;
		const win::dpi::Dpi& m_dpi;
		struct constructor_tag { explicit constructor_tag() = default; };
	public:
		ConsoleWindow(constructor_tag,const win::dpi::Dpi& dpi) :m_dpi(dpi){}
		~ConsoleWindow() {
			m_textarea->UnregisterTextStore();
		}
		static constexpr UINT WM_UPDATE_SCROLLBAR = WM_USER + 0x0001;
		static constexpr LPCTSTR m_classname=_T("ConsoleWindow");
		static std::unique_ptr<ConsoleWindow> Create(HINSTANCE,
			HWND,
			const win::dpi::Dpi& dpi,
			DIP x, DIP y, DIP w, DIP dip_h,
			HMENU,
			ITfThreadMgr* threadmgr, 
			TfClientId,
			ITfCategoryMgr* cate_mgr,
			ITfDisplayAttributeMgr* attr_mgr,
			ID2D1Factory*,
			IDWriteFactory*,
			std::function<std::shared_ptr<cwnd::Context>(DIP w,DIP h)> getContext);
		void ReGetConsoleContext();
		HWND GetHWnd();
		void OnDpiChange();
	};
}


