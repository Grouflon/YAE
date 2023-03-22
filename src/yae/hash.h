#pragma once

#include <yae/types.h>

namespace yae {
namespace hash {

YAE_API u32 hash32(const void* _data, size_t _size);
YAE_API u32 hashString(const char* _str);

} // namespace hash
} // namespace yae
