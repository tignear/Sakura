#pragma once
#include <d2d1.h>
#include <wrl.h>
#include <optional>
#include <memory>
namespace tignear {
	struct Direct2D {
		virtual ID2D1Factory* GetFactory()=0;
		virtual ID2D1RenderTarget* GetRenderTarget() = 0;
		virtual void ReCreate()=0;
		virtual void ReSize()=0;
		virtual ~Direct2D() {};
	};
	class Direct2DWithHWnd:Direct2D{
	private:
		HWND m_hwnd;
		void InitInternal();
		void InitResource();
		struct _constructor_tag { explicit _constructor_tag() = default; }; //make_unique
		ID2D1Factory* m_factory;
		Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_target;
	public:
		void ReCreate();
		void ReSize();
		Direct2DWithHWnd(_constructor_tag) {}
		static std::unique_ptr<Direct2DWithHWnd> Create(ID2D1Factory*, HWND);
		ID2D1Factory* GetFactory()override {
			return m_factory;
		}
		ID2D1RenderTarget* GetRenderTarget()override {
			return m_target.Get();
		}
		ID2D1HwndRenderTarget* GetHwndRenderTarget() {
			return m_target.Get();
		}
	};
}