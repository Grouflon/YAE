#pragma once

// INCLUDEs
#include <stdint.h> // for int types
#include <cstdio> // for printf

// BASE TYPES
typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

// CONSTANTS
#define SMALL_NUMBER (1.e-4f)
#define VERY_SMALL_NUMBER (1.e-7F)

// BUILD DEFINES
#ifndef YAE_DEBUG
#define YAE_DEBUG 0
#endif

#ifndef YAE_RELEASE
#define YAE_RELEASE 0
#endif

#ifndef YAE_PLATFORM_WINDOWS
#define YAE_PLATFORM_WINDOWS 0
#endif

#ifndef YAE_PLATFORM_WEB
#define YAE_PLATFORM_WEB 0
#endif

#ifndef CORE_API
#define CORE_API
#endif

// MACROS
#include <cstring>
#ifndef __FILENAME__
#define __FILENAME__ \
	(strrchr(__FILE__,'/') \
	? strrchr(__FILE__,'/')+1 \
	: (strrchr(__FILE__,'\\') \
	? strrchr(__FILE__,'\\')+1 \
	: __FILE__ \
	))
#endif

#ifdef _MSC_VER
#include <intrin.h>
#define YAE_DEBUG_BREAK __debugbreak()
#elif YAE_PLATFORM_WEB
#include <cassert>
#define YAE_DEBUG_BREAK assert(false)
#else
#define YAE_DEBUG_BREAK raise(SIGTRAP)
#endif

#ifndef YAE_ASSERT_ENABLED
#if YAE_RELEASE
#define YAE_ASSERT_ENABLED 0
#else
#define YAE_ASSERT_ENABLED 1
#endif
#endif

// RENDERERS
#ifndef YAE_IMPLEMENTS_RENDERER_OPENGL
	#define YAE_IMPLEMENTS_RENDERER_OPENGL 0
#endif

#ifndef YAE_IMPLEMENTS_RENDERER_VULKAN
	#define YAE_IMPLEMENTS_RENDERER_VULKAN 0
#endif

// ASSERTS (should not depend on any engine construct)
#if YAE_ASSERT_ENABLED
#define YAE_ASSERT(_cond)					do {if (!(_cond)) { printf("Assert failed: %s\n", #_cond);       YAE_DEBUG_BREAK; }} while(0)
#define YAE_ASSERT_MSG(_cond, _msg)			do {if (!(_cond)) { printf("Assert failed: %s\n", _msg);         YAE_DEBUG_BREAK; }} while(0)
#define YAE_ASSERT_MSGF(_cond, _fmt, ...)	do {if (!(_cond)) { printf("Assert failed: %s\n", #_cond); printf(_fmt, __VA_ARGS__); printf("\n"); YAE_DEBUG_BREAK; }} while(0)
#define YAE_VERIFY(_cond) 					do {if (!(_cond)) { printf("Verify failed: %s\n", #_cond);       YAE_DEBUG_BREAK; }} while(0)
#define YAE_VERIFY_MSG(_cond)				do {if (!(_cond)) { printf("Verify failed: %s\n", _msg);         YAE_DEBUG_BREAK; }} while(0)
#define YAE_VERIFY_MSGF(_cond)				do {if (!(_cond)) { printf("Verify failed: %s\n", #_cond); printf(_fmt, __VA_ARGS__); printf("\n"); YAE_DEBUG_BREAK; }} while(0)
#else
#define YAE_ASSERT(cond)
#define YAE_ASSERT_MSG(_cond, _msg)
#define YAE_ASSERT_MSGF(_cond, _fmt, ...)
#define YAE_VERIFY(_cond) 					_cond
#define YAE_VERIFY_MSG(_cond)				_cond
#define YAE_VERIFY_MSGF(_cond)				_cond
#endif

// BASE EXTENSIONS
template <typename T, size_t N>
constexpr size_t countof(const T(&)[N])
{
	return N;
}

// FORWARD DECLARATIONS & GLOBAL GETTERS
namespace yae {

class Allocator;
class Program;
//class Application;
class Module;
//class ResourceManager;
class Logger;
class Profiler;
//class Renderer;
//class InputSystem;
class Serializer;
//class Resource;

CORE_API Program& program();
CORE_API Allocator& defaultAllocator();
CORE_API Allocator& scratchAllocator();
CORE_API Allocator& toolAllocator();
CORE_API Profiler& profiler();
CORE_API Logger& logger();

CORE_API void setAllocators(Allocator* _defaultAllocator, Allocator* _scratchAllocator, Allocator* _toolAllocator);

extern Allocator* g_defaultAllocator;
extern Allocator* g_scratchAllocator;
extern Allocator* g_toolAllocator;

} // namespace yae

// BASE INCLUDES
#include <core/string_types.h>
#include <core/logging.h>
#include <core/profiling.h>
#include <core/StringHash.h>