#pragma once

#include <stdint.h>

namespace yae {
  uint32_t Hash32(const void* _data, size_t _size);
  uint32_t HashString(const char* _str);
}
