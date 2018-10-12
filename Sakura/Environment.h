#pragma once
// Check windows

#if _WIN32 || _WIN64
#if _WIN64
#define PLATFORM_X64
#else
#define PLATFORM_X86
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define PLATFORM_X64
#else
#define PLATFORM_X86
#endif
#endif
namespace tignear::environment{
	enum class CPU_ARCH {
		X64,X86
	};
	constexpr CPU_ARCH arch() {
#ifdef  PLATFORM_X64
		return CPU_ARCH::X64;
#elif PLATFORM_X86
		return CPU_ARCH::X86;
#else
		static_assert("not defined cpu arch");
#endif
	}
}