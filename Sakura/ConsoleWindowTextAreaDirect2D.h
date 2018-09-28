#pragma once
#include <wrl.h>
#include <dwrite_1.h>
#include "Direct2D.h"
namespace tignear::sakura {
	class ConsoleWindowTextAreaDirect2D:virtual public Direct2D {
		template <class T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;


	protected:
		virtual void InitResource()override;
	public:
		ComPtr<IDWriteTextLayout1> inputtingstring_layout;
		ComPtr<ID2D1SolidColorBrush> red;
		ComPtr<ID2D1SolidColorBrush> transparency;
	};
	class ConsoleWindowTextAreaDirect2DWithHWnd :virtual public ConsoleWindowTextAreaDirect2D,virtual public Direct2DWithHWnd {
		struct constructor_tag { explicit constructor_tag() = default; }; //make_unique
	protected:
		void InitResource()override;
	public:
		ID2D1Factory* GetFactory()override;
		ID2D1RenderTarget* GetRenderTarget()override;
		void ReCreate()override;
		void ReSize()override;
		ConsoleWindowTextAreaDirect2DWithHWnd(constructor_tag,ID2D1Factory* d2d1_f, HWND hwnd):Direct2DWithHWnd(d2d1_f,hwnd){}
		static std::unique_ptr<ConsoleWindowTextAreaDirect2DWithHWnd> Create(ID2D1Factory* d2d1_f, HWND hwnd);
	};
}