#pragma once
#include <atomic>
#include  <WTypesbase.h>
using DIP = FLOAT;
using PIXEL = UINT;
using DPI=UINT;
namespace tignear::win::dpi {
	class Dpi {
		double m_pixelperdip;
		double m_dipperpixel;
		DPI m_dpi;
	public:
		explicit Dpi(UINT dpi):m_pixelperdip(dpi/96.0), m_dipperpixel(96.0/dpi),m_dpi(dpi){
			
		}
		void SetDpi(UINT dpi) {
			m_dpi = dpi;
			m_pixelperdip=dpi / 96.0;
			m_dipperpixel=96.0 / dpi;
		}
		DPI GetDpi()const {
			return m_dpi;
		}
		PIXEL Pixel(DIP dip)const {
			return static_cast<PIXEL>(dip * m_pixelperdip+0.5);
		}
		DIP Dip(PIXEL pixel)const {
			return static_cast<DIP>(pixel * m_dipperpixel);
		}
	};
#pragma warning(disable:4505)
	static inline HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	static const void SetHWndFont(HWND hwnd) {
		SendMessage(hwnd, WM_SETFONT, (WPARAM)font, 0);
	}
#pragma warning(default:4505)

}