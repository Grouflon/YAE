#pragma once

#include <core/types.h>

namespace yae {
namespace hash {

CORE_API u32 hash32(const void* _data, size_t _size);
CORE_API u32 hashString(const char* _str);

template <typename T>
u32 hash32(const T& _data);

} // namespace hash
} // namespace yae

#include "hash.inl"
