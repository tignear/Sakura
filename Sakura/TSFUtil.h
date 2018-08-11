#pragma once
#include <wrl.h>
#include <msctf.h>
namespace {
	using Microsoft::WRL::ComPtr;
}
namespace tignear::tsf::util{
	const constexpr bool failed(const HRESULT hr) {
		return FAILED(hr);
	}
}