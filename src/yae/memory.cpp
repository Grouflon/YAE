#include "memory.h"

#include <cstdlib>
#include <cstring>

#define YAE_ALLOCATION_CALLSTACK_CAPTURE 0

#if YAE_ALLOCATION_CALLSTACK_CAPTURE
#include <yae/callstack.h>
#include <yae/containers.h>
#endif

namespace yae {

#if YAE_ALLOCATION_CALLSTACK_CAPTURE
struct AllocationCallStack
{
	StackFrame frames[32];
	u16 frameCount;
};
static HashMap<size_t, AllocationCallStack> g_allocationCaptures(&s_untrackedMallocAllocator);
#endif

const u8 HEADER_PAD_VALUE = 0xFAu;

FixedSizeAllocator::FixedSizeAllocator(size_t _size)
{
	m_allocableSize = _size;
	m_memorySize = _size + _getHeaderSize();
	m_memory = malloc(m_memorySize);
	YAE_ASSERT(m_memory != nullptr);

#if YAE_DEBUG
	memset(m_memory, 0, m_memorySize);
#endif

	m_firstBlock = (Header*)m_memory;
	m_firstBlock->size = m_allocableSize;
	m_firstBlock->next = nullptr;
	m_firstBlock->alignment = DEFAULT_ALIGN;
	m_firstBlock->used = false;
}



FixedSizeAllocator::~FixedSizeAllocator()
{
#if YAE_ALLOCATION_CALLSTACK_CAPTURE
	if (m_allocationCount > 0)
	{
		printf("%d Memory Leaks:\n", u32(m_allocationCount));
		for(auto entry : g_allocationCaptures)
		{
			printCallstack(entry.value.frames, entry.value.frameCount);
			printf("\n");
		}
	}
#endif
	YAE_ASSERT_MSGF(m_allocationCount == 0, "Allocations count == %d, memory leak detected", m_allocationCount);
	free(m_memory);
}

// @OPTIM: align all allocations on 4, so we can use a u32 as HEADER_PAD_VALUE and do less operations
void* FixedSizeAllocator::allocate(size_t _size, u8 _align)
{
	if (_align < 4)
	{
		_align = 4;
	}
	YAE_ASSERT(_align % 4 == 0);
	_size = ((_size + 3)/4)*4;
	size_t requestedSize = _size;

	Header* block = m_firstBlock;
	Header* streakStartBlock = nullptr; 
	Header* streakEndBlock = nullptr;
	size_t availableSize = 0;

	while (block != nullptr)
	{
		if (!block->used)
		{
			availableSize = block->size;
			requestedSize = _size + size_t(_getDataStart(block, _align) - _getData(block));

			streakStartBlock = block;
			streakEndBlock = streakStartBlock;
			while (streakEndBlock->next != nullptr && !streakEndBlock->next->used && availableSize < requestedSize)
			{
				streakEndBlock = streakEndBlock->next;
				size_t headerSize = _getHeaderSize();
				size_t blockSize = _getBlockSize(streakEndBlock);
				availableSize += blockSize;
				YAE_ASSERT(!streakEndBlock->next || _getData(streakStartBlock) + availableSize == (u8*)streakEndBlock->next);
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
	YAE_ASSERT_MSGF(streakStartBlock != nullptr, "Out of memory! %d / %d, %d requested", m_allocatedSize, m_allocableSize, _size);
	size_t remainingSize = availableSize - requestedSize;
	u8* streakStartBlockData = _getData(streakStartBlock);

	if (remainingSize < _getMinimumBlockSize())
	{
		streakStartBlock->size = availableSize;
		streakStartBlock->next = streakEndBlock->next;
	}
	else
	{
		Header* nextBlock = (Header*)(streakStartBlockData + requestedSize);
		nextBlock->next = streakEndBlock->next;
		nextBlock->size = remainingSize - _getHeaderSize();
		nextBlock->alignment = DEFAULT_ALIGN;
		nextBlock->used = false;

		streakStartBlock->size = requestedSize;
		streakStartBlock->next = nextBlock;
	}
	streakStartBlock->alignment = _align;
	streakStartBlock->used = true;

#if YAE_DEBUG
	// Reset memory
	memset(streakStartBlockData, 0x00, streakStartBlock->size);
#endif

	// Align data
	u8* dataStart = _getDataStart(streakStartBlock, _align);
	memset(streakStartBlockData, HEADER_PAD_VALUE, dataStart - streakStartBlockData);
	YAE_ASSERT(((u8*)streakStartBlock->next) - dataStart >= i64(_size));
	YAE_ASSERT(!streakStartBlock->next || streakStartBlock->next == (Header*)(streakStartBlockData + streakStartBlock->size));
	m_allocatedSize = m_allocatedSize + _getBlockSize(streakStartBlock);
	++m_allocationCount;

#if YAE_DEBUG
	check();
#endif

#if YAE_ALLOCATION_CALLSTACK_CAPTURE
	{
		AllocationCallStack capture;
		capture.frameCount = captureCallstack(capture.frames, 32);
		g_allocationCaptures.set(size_t(dataStart), capture);
	}
#endif

	return dataStart;
}



void FixedSizeAllocator::deallocate(void* _memory)
{
	if (_memory == nullptr)
		return;

	Header* block = m_firstBlock;
	while (block != nullptr)
	{
		void* data = _getData(block);
		void* dataStart = _getDataStart(block, block->alignment);
		if (dataStart == _memory)
		{
			YAE_ASSERT(block->used);
			block->used = false;
			--m_allocationCount;
			m_allocatedSize = m_allocatedSize - _getBlockSize(block);

#if YAE_DEBUG
			memset(data, 0xDD, block->size);
			check();
#endif

#if YAE_ALLOCATION_CALLSTACK_CAPTURE
			{
				g_allocationCaptures.remove(size_t(dataStart));
			}
#endif

			return;
		}
		block = block->next;
	}
	YAE_ASSERT(false);
}


size_t FixedSizeAllocator::getAllocationSize(void* _memory) const
{
	if (_memory == nullptr)
		return SIZE_NOT_TRACKED;

	Header* block = m_firstBlock;
	while (block != nullptr)
	{
		void* data = _getData(block);
		void* dataStart = _getDataStart(block, block->alignment);
		if (dataStart == _memory)
		{
			return block->size - size_t((u8*)(dataStart) - (u8*)(data));
		}
		block = block->next;
	}

	return SIZE_NOT_TRACKED;
}


void FixedSizeAllocator::check()
{
	size_t totalSize = 0;
	size_t currentBlock = 0;
	Header* block = m_firstBlock;
	while (block != nullptr)
	{
		u8* data = _getData(block);
		YAE_ASSERT(!block->next || block->next == (Header*)(data + block->size));

		totalSize += _getBlockSize(block);
		block = block->next;

		++currentBlock;
	}
	YAE_ASSERT(totalSize == m_memorySize);
}


FixedSizeAllocator::Header* FixedSizeAllocator::_getHeader(void* _data)
{
	u8* data = (u8*)_data;
	while(*(data - 1) == HEADER_PAD_VALUE)
	{
		--data;
	}
	return (Header*)(data - _getHeaderSize());
}


u8* FixedSizeAllocator::_getData(Header* _header)
{
	return (u8*)_header + _getHeaderSize();
}


u8* FixedSizeAllocator::_getDataStart(Header* _header, u8 _alignment)
{
	return (u8*)memory::alignForward(_getData(_header), _alignment);
}



size_t FixedSizeAllocator::_getBlockSize(Header* _header)
{
	return _getHeaderSize() + _header->size;
}


MallocAllocator::MallocAllocator() : Allocator()
{
}


MallocAllocator::~MallocAllocator()
{
	YAE_ASSERT_MSGF(m_allocationCount == 0, "Allocations count == %d, memory leak detected", m_allocationCount);
}


void* MallocAllocator::allocate(size_t _size, u8 _align)
{
	size_t blockSize = _size + _align + sizeof(Header);
	Header* block = (Header*)malloc(blockSize);
	block->size = _size + _align;

	u8* data = (u8*)block + sizeof(Header);
	u8* dataStart = (u8*)memory::alignForward(data, _align);

#if YAE_DEBUG
	memset(data, 0x00, block->size);
#endif

	memset(data, HEADER_PAD_VALUE, dataStart - data);

	++m_allocationCount;
	m_allocatedSize += blockSize;

	return dataStart;
}


void MallocAllocator::deallocate(void* _memory)
{
	if (_memory == nullptr)
		return;

	Header* block = _getHeader(_memory);
	YAE_ASSERT(block != nullptr);

	u8* data = (u8*)block + sizeof(Header);
#if YAE_DEBUG
	memset(data, 0xDD, block->size);
#endif

	size_t blockSize = sizeof(Header) + block->size;
	free(block);

	--m_allocationCount;
	m_allocatedSize -= blockSize;
}


MallocAllocator::Header* MallocAllocator::_getHeader(void* _data)
{
	u8* data = (u8*)_data;
	while(*(data - 1) == HEADER_PAD_VALUE)
	{
		--data;
	}
	return (Header*)(data - sizeof(Header));
}

namespace memory {

MallocAllocator& mallocAllocator()
{
	static MallocAllocator s_mallocAllocator;
	return s_mallocAllocator;
}

} // namespace memory

} // namespace yae
