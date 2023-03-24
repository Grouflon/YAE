#pragma once

#include <yae/types.h>

namespace yae {
namespace resource {

template <typename T> T* findOrCreate(const char* _name);
template <typename T> T* findOrCreateFile(const char* _path);

} // namespace resource
} // namespace yae

#include "resource.inl"
