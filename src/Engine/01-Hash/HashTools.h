#pragma once

#include <export.h>

#include <00-Type/IntTypes.h>

namespace yae {
	YAELIB_API u32 Hash32(const void* _data, size_t _size);
	YAELIB_API u32 HashString(const char* _str);
}
