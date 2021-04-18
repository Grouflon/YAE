#pragma once

#include <types.h>
#include <memory.h>

namespace yae {

class Allocator;
class Logger;
class ResourceManager;
class Application;
class Profiler;

struct YAELIB_API Context
{
	Allocator* defaultAllocator = nullptr;
	Allocator* scratchAllocator = nullptr;
	Allocator* toolAllocator = nullptr; // Will be null in standalone

	Logger* logger = nullptr;
	ResourceManager* resourceManager = nullptr;
	Application* application = nullptr;
	Profiler* profiler = nullptr;
};

extern Context g_context;
YAELIB_API Context& context();



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
