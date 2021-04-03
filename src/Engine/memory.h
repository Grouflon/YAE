#pragma once

#include <types.h>

namespace yae {

class YAELIB_API Allocator
{
public:
	static const u8 DEFAULT_ALIGN = 4;

	virtual void* allocate(size_t _size, u8 _align = DEFAULT_ALIGN) = 0;
	virtual void deallocate(void* _memory) = 0;

	template <typename T, typename ...Args>
	T* create(Args... _args)
	{
		return new (allocate(sizeof(T))) T(_args...);
	}

	template <typename T>
	void destroy(T* _memory)
	{
		if (_memory)
		{
			(_memory)->~T();
			deallocate(_memory);
		}
	}
};



class YAELIB_API FixedSizeAllocator : public Allocator
{
public:
	FixedSizeAllocator(size_t _size);
	virtual ~FixedSizeAllocator();

	virtual void* allocate(size_t _size, u8 _align = DEFAULT_ALIGN) override;
	virtual void deallocate(void* _memory) override;

private:
	struct Header
	{
		size_t size;
		Header* next;
		u8 alignment;
		bool used;
	};

	constexpr inline static size_t _getHeaderSize() { return sizeof(Header); }
	constexpr inline static size_t _getMinimumBlockSize() { return _getHeaderSize() + 1; }

	static Header* _getHeader(void* _data);
	static u8* _getData(Header* _header);
	static u8* _getDataStart(Header* _header);
	static size_t _getBlockSize(Header* _header);

	void _check();

	void* m_memory = nullptr;
	size_t m_memorySize = 0;
	size_t m_allocableSize = 0;
	size_t m_allocationCount = 0;
	size_t m_allocatedMemory = 0;

	Header* m_firstBlock = nullptr;
};



class YAELIB_API MallocAllocator : public Allocator
{
public:
	virtual ~MallocAllocator();

	virtual void* allocate(size_t _size, u8 _align = DEFAULT_ALIGN) override;
	virtual void deallocate(void* _memory) override;

private:
	struct Header
	{
		size_t size;
	};

	static Header* _getHeader(void* _data);

	size_t m_allocationCount = 0;
	size_t m_allocatedMemory = 0;
};



namespace memory {

constexpr inline YAELIB_API void* alignForward(void* _p, u8 _align)
{
	uintptr_t pi = uintptr_t(_p);
	const uint32_t mod = pi % _align;
	if (mod)
	{
		pi += (_align - mod);
	}
	return (void*)pi;
}

} // namespace memory

} // namespace yae
