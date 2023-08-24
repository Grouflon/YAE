#pragma once

#include <core/types.h>

namespace yae {
namespace containers {

template <typename T> const T* find(const BaseArray<T>& _array, const T& _value);
template <typename T> T* find(BaseArray<T>& _array, const T& _value);

template <typename T> u32 remove(Array<T>& _array, const T& _value);
template <typename T> u32 remove(DataArray<T>& _array, const T& _value);

} // namespace containers 
} // namespace yae

#include "containers.inl"
