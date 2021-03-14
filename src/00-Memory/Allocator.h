#pragma once

#include <00-Type/IntTypes.h>

namespace yae {

class Allocator
{
public:
	Allocator(size_t _size);
	~Allocator();

	void* allocate(size_t _size);
	void deallocate(void* _memory);

private:
	struct Block
	{
		size_t size;
		Block* next;
		bool used;
		alignas(sizeof(size_t)) u8 data[1];
	};

	constexpr inline static size_t _align(size_t _n) { return (_n + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1); }
	constexpr inline static size_t _getHeaderSize() { return sizeof(Block) - sizeof(size_t); }
	constexpr inline static size_t _getAllocationSize(size_t _size) { return _size + _getHeaderSize(); }
	constexpr inline static Block* _getHeader(void* _data) { return (Block*)((u8*)_data - _getHeaderSize()); }
	constexpr inline static size_t _getMinimumBlockSize() { return _getAllocationSize(_align(1)); }

	void _check();

	void* m_memory = nullptr;
	size_t m_memorySize = 0;
	size_t m_allocableSize = 0;
	size_t m_allocationCount = 0;

	Block* m_firstBlock = nullptr;
};

};
