#pragma once

#include <core/types.h>

namespace yae {
namespace hash {

CORE_API u32 hash32(const void* _data, size_t _size);
CORE_API u32 hashString(const char* _str);

} // namespace hash
} // namespace yae
