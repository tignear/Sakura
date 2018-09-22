#pragma once
#include <Windows.h>
namespace tignear {
	inline constexpr const void FailToThrowHR(HRESULT hr) {
		if (FAILED(hr))
		{
			throw hr;
		}
	}
	inline constexpr const void FailToThrowB(bool r) {
		if (!r)
		{
			throw r;
		}
	}
}
