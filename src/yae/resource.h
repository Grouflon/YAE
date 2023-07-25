#pragma once

#include <yae/types.h>

namespace yae {
namespace resource {

template <typename T> T* find(const char* _name);
template <typename T> T* findOrCreate(const char* _name);
template <typename T> T* findOrCreateFile(const char* _path);
template <typename T> T* findOrCreateFromFile(const char* _path);

YAE_API Resource* findOrCreateFromFile(const char* _path);
YAE_API void saveToFile(Resource* _resource, const char* _path);
YAE_API void deleteResourceFile(Resource* _resource);

} // namespace resource
} // namespace yae

#include "resource.inl"
