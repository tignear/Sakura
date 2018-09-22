#pragma once
#include <d2d1.h>
#include <wrl.h>
#include <optional>
#include <memory>
namespace tignear {
	class Direct2D {
	protected:
		virtual void InitResource()=0;
	public:
		virtual ID2D1Factory* GetFactory()=0;
		virtual ID2D1RenderTarget* GetRenderTarget() = 0;
		virtual void ReCreate() {
			InitResource();
		};
		virtual void ReSize()=0;
		virtual ~Direct2D() {};
	};
	class Direct2DWithHWnd:virtual public Direct2D{
		const HWND m_hwnd;
		struct constructor_tag { explicit constructor_tag() = default; }; //make_unique
		ID2D1Factory* const m_factory;
		Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_target;
	protected:
		void InitResource()override;
		Direct2DWithHWnd(ID2D1Factory* d2d1_f, HWND hwnd):m_factory(d2d1_f), m_hwnd(hwnd) {}
	public:
		void ReCreate();
		void ReSize();
		Direct2DWithHWnd(constructor_tag, ID2D1Factory* d2d1_f, HWND hwnd):Direct2DWithHWnd(d2d1_f, hwnd) {}
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