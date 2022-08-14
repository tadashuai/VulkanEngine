#pragma once

#include <memory>
#include "Core/Ref.h"

#ifdef _WIN32
	/* Windows x64/x86 */
	#ifdef _WIN64
		/* Windows x64  */
		#define VE_PLATFORM_WINDOWS
	#else
		/* Windows x86 */
		#error "x86 Builds are not supported!"
	#endif
#else
	/* Unknown compiler/platform */
	#error "Unknown platform!"
#endif

#ifdef VE_DEBUG
	#ifdef VE_PLATFORM_WINDOWS
		#define VE_DEBUGBREAK() __debugbreak()
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
	#define VE_ENABLE_ASSERTS
#else
	#define VE_DEBUGBREAK()
#endif

#define VE_EXPAND_MACRO(x) x
#define VE_STRINGIFY_MACRO(x) #x

#define BIT(x) 1 << x

namespace VE
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope( Args&& ... args )
	{
		return std::make_unique<T>( std::forward<Args>( args )... );
	}
}

#include "Core/Log.h"
#include <filesystem>

#ifdef VE_ENABLE_ASSERTS
	#define VE_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { VE##type##ERROR(msg, __VA_ARGS__); VE_DEBUGBREAK(); } }
	#define VE_INTERNAL_ASSERT_WITH_MSG(type, check, ...) VE_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define VE_INTERNAL_ASSERT_NO_MSG(type, check) VE_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", VE_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)
	#define VE_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define VE_INTERNAL_ASSERT_GET_MACRO(...) VE_EXPAND_MACRO( VE_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, VE_INTERNAL_ASSERT_WITH_MSG, VE_INTERNAL_ASSERT_NO_MSG) )
	#define VE_ASSERT(...) VE_EXPAND_MACRO( VE_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#else
	#define VE_ASSERT(...)
#endif
