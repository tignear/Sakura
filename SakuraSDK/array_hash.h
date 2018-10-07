#pragma once
#include <functional>
namespace tignear::stdex {
	template<class T>
	struct array_hash{
		[[nodiscard]] size_t operator()(T* from, size_t size)const noexcept {
			static const constexpr auto init = std::hash<nullptr_t>{}();
			static const constexpr auto hash = std::hash<T>{};
			auto result = init;
			for (size_t i = 0; i < size; ++i) {
				result = result * 31 + hash(from[i]);
			}
			return result;
		}
	};
}