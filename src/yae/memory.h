#pragma once

#include <stdio.h>

#include <yae/types.h>
#include <yae/math.h>

namespace yae {

namespace memory {

inline YAELIB_API void* alignForward(void* _p, u8 _align)
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

class YAELIB_API Allocator
{
public:
	static const u8 DEFAULT_ALIGN = 4;
	static const size_t SIZE_NOT_TRACKED = ~size_t(0);

	virtual void* allocate(size_t _size, u8 _align = DEFAULT_ALIGN) = 0;
	virtual void* reallocate(void* _memory, size_t _size, u8 _align = DEFAULT_ALIGN) = 0;
	virtual void deallocate(void* _memory) = 0;

	virtual size_t getAllocationCount() const { return SIZE_NOT_TRACKED; }
	virtual size_t getAllocatedSize() const { return SIZE_NOT_TRACKED; }
	virtual size_t getAllocableSize() const { return SIZE_NOT_TRACKED; }

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
	virtual void* reallocate(void* _memory, size_t _size, u8 _align = DEFAULT_ALIGN) override;
	virtual void deallocate(void* _memory) override;

	virtual size_t getAllocationCount() const override { return m_allocationCount; }
	virtual size_t getAllocatedSize() const override { return m_allocatedSize; }
	virtual size_t getAllocableSize() const override { return m_allocableSize; }

	void check();

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
	static u8* _getDataStart(Header* _header, u8 _alignment);
	static size_t _getBlockSize(Header* _header);


	void* m_memory = nullptr;
	size_t m_memorySize = 0;
	size_t m_allocableSize = 0;
	size_t m_allocationCount = 0;
	size_t m_allocatedSize = 0;

	Header* m_firstBlock = nullptr;
};


class YAELIB_API MallocAllocator : public Allocator
{
public:
	MallocAllocator();
	virtual ~MallocAllocator();

	virtual void* allocate(size_t _size, u8 _align = DEFAULT_ALIGN) override;
	virtual void* reallocate(void* _memory, size_t _size, u8 _align = DEFAULT_ALIGN) override;
	virtual void deallocate(void* _memory) override;

	virtual size_t getAllocationCount() const override { return m_allocationCount; }
	virtual size_t getAllocatedSize() const override { return m_allocatedSize; }

private:
	struct Header
	{
		size_t size;
		u8 alignment;
	};

	static Header* _getHeader(void* _data);
	static u8* _getData(Header* _header);
	static u8* _getDataStart(Header* _header);
	static size_t _getDataSize(Header* _header);

	size_t m_allocationCount = 0;
	size_t m_allocatedSize = 0;
};


template <size_t BUFFER_SIZE>
class InlineAllocator : public Allocator
{
public:
	InlineAllocator()
		: m_cursor(0)
	{
	}

	virtual ~InlineAllocator()
	{
	}

	virtual void* allocate(size_t _size, u8 _align = DEFAULT_ALIGN) override
	{
		u8* start = (u8*)memory::alignForward(m_buffer + m_cursor, _align);
		u8* bufferEnd = m_buffer + BUFFER_SIZE;
		size_t availableSize = bufferEnd - start;
		YAE_ASSERT_MSGF(availableSize >= _size, "Out of memory: %zu available, %zu requested", availableSize, _size);
		m_cursor = (start + _size) - m_buffer;
		return start;
	}

	virtual void* reallocate(void* _memory, size_t _size, u8 _align = DEFAULT_ALIGN) override
	{
		if (_memory == nullptr)
		{
			return allocate(_size, _align);
		}

		for (size_t i = 0; i <= m_cursor; ++i)
		{
			if (m_buffer + i == _memory)
			{
				u8* start = (u8*)memory::alignForward(m_buffer + i, _align);
				u8* bufferEnd = m_buffer + BUFFER_SIZE;
				size_t availableSize = bufferEnd - start;
				YAE_ASSERT_MSGF(availableSize >= _size, "Out of memory: %zu available, %zu requested", availableSize, _size);
				if (start != _memory)
				{
					u8* previousStart = (u8*)_memory;
					u8* previousEnd = m_buffer + m_cursor;
					memcpy(m_buffer + m_cursor, m_buffer + i, yae::min((i64)_size, (i64)(previousEnd - previousStart)));
				}
				m_cursor = (start + _size) - m_buffer;
				return start;
			}
		}
		YAE_ASSERT_MSGF(false, "Can't reallocate, Memory %p not found in allocator", _memory);
		return nullptr;
	}

	virtual void deallocate(void* _memory) override
	{
		// do nothing, we can't go back on this allocator
	}

private:
	u8 m_buffer[BUFFER_SIZE];
	size_t m_cursor;
};

typedef InlineAllocator<32> InlineAllocator32;
typedef InlineAllocator<64> InlineAllocator64;
typedef InlineAllocator<128> InlineAllocator128;
typedef InlineAllocator<256> InlineAllocator256;
typedef InlineAllocator<512> InlineAllocator512;

YAELIB_API MallocAllocator& mallocAllocator();

} // namespace yae
