#pragma once

#include <memory.h>
#include <context.h>

namespace yae {

// Std allocators for std containers
template <class T>
struct DefaultAllocator
{
	typedef T value_type;
	DefaultAllocator() = default;
 	template <class U> constexpr DefaultAllocator(const DefaultAllocator<U>&) noexcept {}
	T* allocate(std::size_t _n) noexcept
	{
		return (T*)context().defaultAllocator->allocate(_n * sizeof(T));
	}
	void deallocate(T* _p, std::size_t) noexcept
	{
		context().defaultAllocator->deallocate(_p);
	}
};
template <class T, class U>
bool operator==(const DefaultAllocator<T>&, const DefaultAllocator<U>&) { return true; }
template <class T, class U>
bool operator!=(const DefaultAllocator<T>&, const DefaultAllocator<U>&) { return false; }

} // namespace yae
