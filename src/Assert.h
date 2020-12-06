#pragma once

#ifdef _MSC_VER
#include <intrin.h>
#define YAE_DEBUG_BREAK __debugbreak()
#else
#define YAE_DEBUG_BREAK raise(SIGTRAP)
#endif

#ifdef _DEBUG
#define YAE_ASSERT(cond) if (!(cond)) YAE_DEBUG_BREAK;
#else
#define YAE_ASSERT(cond)
#endif
