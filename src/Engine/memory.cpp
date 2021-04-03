#include "memory.h"

#include <cstdlib>
#include <cstring>

namespace yae {

FixedSizeAllocator::FixedSizeAllocator(size_t _size)
{
	m_allocableSize = _size;
	m_memorySize = _size + _getHeaderSize();
	m_memory = malloc(m_memorySize);
	YAE_ASSERT(m_memory != nullptr);

#if YAE_DEBUG
	memset(m_memory, 0, m_memorySize);
#endif

	m_firstBlock = (Block*)m_memory;
	m_firstBlock->size = m_allocableSize;
	m_firstBlock->next = nullptr;
	m_firstBlock->used = false;
}



FixedSizeAllocator::~FixedSizeAllocator()
{
	YAE_ASSERT_MSGF(m_allocationCount == 0, "Allocations count == %d, memory leak detected", m_allocationCount);
	free(m_memory);
}



void* FixedSizeAllocator::allocate(size_t _size)
{
	YAE_ASSERT(_size <= m_allocableSize);

	size_t requestedSize = _align(_size);

	Block* block = m_firstBlock;
	Block* streakStartBlock = nullptr; 
	Block* streakEndBlock = nullptr;
	size_t availableSize = 0;

	while (block != nullptr)
	{
		if (!block->used)
		{
			availableSize = block->size;
			streakStartBlock = block;
			streakEndBlock = streakStartBlock;
			while (streakEndBlock->next != nullptr && !streakEndBlock->next->used && availableSize < requestedSize)
			{
				streakEndBlock = streakEndBlock->next;
				size_t allocationSize = _getAllocationSize(streakEndBlock->size);
				availableSize += allocationSize;
				YAE_ASSERT(!streakEndBlock->next || streakStartBlock->data + availableSize == (u8*)streakEndBlock->next);
			}

			if (availableSize >= requestedSize)
			{
				break; // Valid spot
			}
			else
			{
				block = streakEndBlock;
				streakStartBlock = nullptr;
				streakEndBlock = nullptr;
			}
		}
		block = block->next;
	}
	YAE_ASSERT_MSGF(streakStartBlock != nullptr, "Out of memory! %d / %d, %d requested", );

	size_t remainingSize = availableSize - requestedSize;
	if (remainingSize < _getMinimumBlockSize())
	{
		streakStartBlock->size = availableSize;
		streakStartBlock->next = streakEndBlock->next;
	}
	else
	{
		Block* nextBlock = (Block*)(streakStartBlock->data + requestedSize);
		nextBlock->next = streakEndBlock->next;
		nextBlock->size = remainingSize - _getHeaderSize();
		nextBlock->used = false;

		streakStartBlock->size = requestedSize;
		streakStartBlock->next = nextBlock;
	}
	streakStartBlock->used = true;
	YAE_ASSERT(!streakStartBlock->next || streakStartBlock->next == (Block*)(streakStartBlock->data + streakStartBlock->size));
	m_allocatedMemory = m_allocatedMemory + _getAllocationSize(streakStartBlock->size);
	++m_allocationCount;

#if YAE_DEBUG
	_check();
#endif
	return streakStartBlock->data;
}



void FixedSizeAllocator::deallocate(void* _memory)
{
	if (_memory == nullptr)
		return;

	Block* block = m_firstBlock;
	while (block != nullptr)
	{
		if (block->data == _memory)
		{
			YAE_ASSERT(block->used);
			block->used = false;
			--m_allocationCount;
			m_allocatedMemory = m_allocatedMemory - _getAllocationSize(block->size);

#if YAE_DEBUG
			memset(block->data, 0xDD, block->size);
			_check();
#endif
			return;
		}
		block = block->next;
	}
	YAE_ASSERT(false);
}



void FixedSizeAllocator::_check()
{
	size_t totalSize = 0;
	Block* block = m_firstBlock;
	while (block != nullptr)
	{
		totalSize += _getAllocationSize(block->size);
		block = block->next;
	}
	YAE_ASSERT(totalSize == m_memorySize);
}



void* MallocAllocator::allocate(size_t _size)
{
	return malloc(_size);
}



void MallocAllocator::deallocate(void* _memory)
{
	free(_memory);
}


} // namespace yae
