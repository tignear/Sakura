#pragma once
#include <functional>
#undef max
namespace tignear::stdex {
	//http://neocat.hatenablog.com/entry/20160521/1463842415
	//‚ð‰ü•Ï
	template <typename T>
	class LoopTemplate {
		T max, cnt;
	public:
		LoopTemplate(T max) : max(max), cnt(0) {}
		LoopTemplate(T min, T max) : max(max), cnt(min) {}
		LoopTemplate& begin() {
			return *this;
		}
		LoopTemplate& operator++() {
			cnt++;
			return *this;
		}
		T& operator*() {
			return cnt;
		}
		bool operator!=(LoopTemplate&) {
			return cnt < max;
		}
		LoopTemplate& end() {
			return *this;
		}
	};
	using Loop=LoopTemplate<size_t>;
	inline void For(size_t cnt, std::function<void(void)> fn) {
#pragma warning(disable:4189)
		for (auto i : Loop(cnt)) {
			fn();
		}
#pragma warning(default:4189)
	}
}
constexpr std::size_t operator "" _z(size_t n)
{
	return n;
}