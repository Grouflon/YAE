#pragma once

#ifdef _MSC_VER
#include <intrin.h>
#define YAE_DEBUG_BREAK __debugbreak()
#else
#define YAE_DEBUG_BREAK raise(SIGTRAP)
#endif

#ifdef _DEBUG
#define YAE_ASSERT(_cond) if (!(_cond)) YAE_DEBUG_BREAK;
#define YAE_ASSERT_MSG(_cond, _msg) if (!(_cond)) YAE_DEBUG_BREAK;
#define YAE_ASSERT_MSGF(cond, _fmt, ...) if (!(cond)) YAE_DEBUG_BREAK;
#else
#define YAE_ASSERT(cond)
#define YAE_ASSERT_MSG(_cond, _msg)
#define YAE_ASSERT_MSGF(cond, _fmt, ...)
#endif
