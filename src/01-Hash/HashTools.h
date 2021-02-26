#pragma once

#include <00-Type/IntTypes.h>

namespace yae {
  u32 Hash32(const void* _data, size_t _size);
  u32 HashString(const char* _str);
}
