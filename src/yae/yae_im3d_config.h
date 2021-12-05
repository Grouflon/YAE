#pragma once

#include <yae/types.h>
#include <yae/memory.h>

// User-defined assertion handler (default is cassert assert()).
#define IM3D_ASSERT(e) YAE_ASSERT(e)

// User-defined malloc/free. Define both or neither (default is cstdlib malloc()/free()).
#define IM3D_MALLOC(size) ::yae::toolAllocator().allocate(size)
#define IM3D_FREE(ptr) ::yae::toolAllocator().deallocate(ptr)

// User-defined API declaration (e.g. __declspec(dllexport)).
#define IM3D_API YAELIB_API