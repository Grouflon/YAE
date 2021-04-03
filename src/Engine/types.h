#pragma once

// Int Types
#include <stdint.h>

typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

// DLL EXPORT
#ifdef YAELIB_EXPORT
#define YAELIB_API __declspec(dllexport)
#else
#define YAELIB_API __declspec(dllimport)
#endif

// MACROS
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
#else
#define YAE_DEBUG_BREAK raise(SIGTRAP)
#endif

// ASSERTS
#ifdef _DEBUG
#define YAE_ASSERT(_cond) if (!(_cond)) YAE_DEBUG_BREAK;
#define YAE_ASSERT_MSG(_cond, _msg) if (!(_cond)) YAE_DEBUG_BREAK;
#define YAE_ASSERT_MSGF(cond, _fmt, ...) if (!(cond)) YAE_DEBUG_BREAK;
#else
#define YAE_ASSERT(cond)
#define YAE_ASSERT_MSG(_cond, _msg)
#define YAE_ASSERT_MSGF(cond, _fmt, ...)
#endif

// BASE EXTENSIONS
template <typename T, size_t N>
constexpr size_t countof(const T(&_array)[N])
{
	return N;
}
