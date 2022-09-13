#include "memory.h"

#include <yae/math.h>

#include <cstdlib>
#include <cstring>

#define YAE_ALLOCATION_CALLSTACK_CAPTURE 1

#if YAE_DEBUG
#define YAE_INITIALIZE_MEMORY 1
#define YAE_CHECK_MEMORY 1
#else
#define YAE_INITIALIZE_MEMORY 0
#define YAE_CHECK_MEMORY 0
#endif

#if YAE_ALLOCATION_CALLSTACK_CAPTURE
#include <yae/callstack.h>
#include <yae/containers/HashMap.h>

namespace yae {

class CallStackAllocator : public Allocator
{
public:
	CallStackAllocator() {}
	virtual void* allocate(size_t _size, u8 _align = DEFAULT_ALIGN) override { return malloc(_size); }
	virtual void* reallocate(void* _memory, size_t _size, u8 _align = DEFAULT_ALIGN) override { return realloc(_memory, _size); }
	virtual void deallocate(void* _memory) override { free(_memory); }
};
static CallStackAllocator s_callstackAllocator;

struct AllocationCallStack
{
	size_t size;
	StackFrame frames[32];
	u16 frameCount;
};
HashMap<size_t, AllocationCallStack>& allocationCaptures()
{
	static HashMap<size_t, AllocationCallStack>* s_allocationCaptures = nullptr;
	if (s_allocationCaptures == nullptr)
	{
		s_allocationCaptures = new HashMap<size_t, AllocationCallStack>(&s_callstackAllocator);
	}
	return *s_allocationCaptures;
}

} // namespace yae

#define YAE_ALLOCATION_CALLSTACK_CAPTURE_PRINT(_allocator)\
	if (_allocator->getAllocationCount() > 0)\
	{\
		printf("%zu Memory Leaks:\n", _allocator->getAllocationCount());\
		for(auto entry : allocationCaptures())\
		{\
			printf("address: 0x%p (%zu bytes)\n", (void*)entry.key, entry.value.size);\
			callstack::print(entry.value.frames, entry.value.frameCount);\
			printf("\n");\
		}\
	}

#define YAE_ALLOCATION_CALLSTACK_CAPTURE_IN(_memory, _size)\
	{\
		AllocationCallStack capture;\
		capture.size = _size;\
		capture.frameCount = callstack::capture(capture.frames, 32);\
		allocationCaptures().set(size_t(_memory), capture);\
	}

#define YAE_ALLOCATION_CALLSTACK_CAPTURE_OUT(_memory)\
	{\
		allocationCaptures().remove(size_t(_memory));\
	}

#else
#define YAE_ALLOCATION_CALLSTACK_CAPTURE_PRINT(_allocator)
#define YAE_ALLOCATION_CALLSTACK_CAPTURE_IN(_memory, _size)
#define YAE_ALLOCATION_CALLSTACK_CAPTURE_OUT(_memory)
#endif

namespace yae {

const u8 HEADER_PAD_VALUE = 0xFAu;

FixedSizeAllocator::FixedSizeAllocator(size_t _size)
{
	m_allocableSize = _size;
	m_memorySize = _size + _getHeaderSize();
	m_memory = malloc(m_memorySize);
	YAE_ASSERT(m_memory != nullptr);

#if YAE_INITIALIZE_MEMORY
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
	YAE_ALLOCATION_CALLSTACK_CAPTURE_PRINT(this);

	YAE_ASSERT_MSGF(m_allocationCount == 0, "Allocations count == %zu, memory leak detected", m_allocationCount);
	free(m_memory);
}

// @OPTIM: align all allocations on 4, so we can use a u32 as HEADER_PAD_VALUE and do less operations
void* FixedSizeAllocator::allocate(size_t _size, u8 _align)
{
	if (_size == 0)
		return nullptr;

	if (_align < 4)
	{
		_align = 4;
	}
	YAE_ASSERT_MSGF(_align % 4 == 0, "All alignments values must be multiples of 4 (%d given)", _align);
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
	YAE_ASSERT_MSGF(streakStartBlock != nullptr, "Out of memory! %zu / %zu, %zu requested", m_allocatedSize, m_allocableSize, _size);
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

#if YAE_INITIALIZE_MEMORY
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

#if YAE_CHECK_MEMORY
	check();
#endif

	YAE_ALLOCATION_CALLSTACK_CAPTURE_IN(dataStart, _size);

	return dataStart;
}


void* FixedSizeAllocator::reallocate(void* _memory, size_t _size, u8 _align)
{
	if (_memory == nullptr)
		return allocate(_size, _align);

	Header* block = m_firstBlock;
	while (block != nullptr)
	{
		void* data = _getData(block);
		void* dataStart = _getDataStart(block, block->alignment);
		if (dataStart == _memory)
		{
			// @NOTE: this is not an actual realloc, but it will do for now	
			size_t size = block->size - size_t((u8*)(dataStart) - (u8*)(data));
			void* newMemory = allocate(_size, _align);
			memcpy(newMemory, _memory, min(u64(size), u64(_size)));
			deallocate(_memory);
			return newMemory;
		}
		block = block->next;
	}
	YAE_ASSERT_MSG(false, "The memory to reallocate is unknown to this allocator.");
	return nullptr;
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

#if YAE_INITIALIZE_MEMORY
			memset(data, 0xDD, block->size);
#endif
#if YAE_CHECK_MEMORY
			check();
#endif

			YAE_ALLOCATION_CALLSTACK_CAPTURE_OUT(_memory);

			return;
		}
		block = block->next;
	}
	YAE_ASSERT_MSG(false, "Memory block not found.");
}


void FixedSizeAllocator::check()
{
	size_t totalSize = 0;
	Header* block = m_firstBlock;
	while (block != nullptr)
	{
		u8* data = _getData(block);
		YAE_ASSERT(!block->next || block->next == (Header*)(data + block->size));
		YAE_ASSERT(m_allocatedSize == m_allocableSize || block->size != 0);

		totalSize += _getBlockSize(block);
		block = block->next;
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
	YAE_ALLOCATION_CALLSTACK_CAPTURE_PRINT(this);

	YAE_ASSERT_MSGF(m_allocationCount == 0, "Allocations count == %zu, memory leak detected", m_allocationCount);
}


void* MallocAllocator::allocate(size_t _size, u8 _align)
{
	if (_size == 0)
		return nullptr;

	size_t blockSize = _size + _align + sizeof(Header);
	Header* block = (Header*)malloc(blockSize);
	block->size = _size + _align;
	block->alignment = _align;

	u8* data = _getData(block);
	u8* dataStart = _getDataStart(block);

#if YAE_INITIALIZE_MEMORY
	memset(data, 0x00, block->size);
#endif

	memset(data, HEADER_PAD_VALUE, dataStart - data);

	++m_allocationCount;
	m_allocatedSize += blockSize;

	YAE_ALLOCATION_CALLSTACK_CAPTURE_IN(dataStart, _size);

	return dataStart;
}


void* MallocAllocator::reallocate(void* _memory, size_t _size, u8 _align)
{
	if (_memory == nullptr)
	{
		return allocate(_size, _align);
	}

	size_t newBlockSize = _size + _align + sizeof(Header);
	void* newMemory = allocate(newBlockSize, _align);

	Header* previousBlock = _getHeader(_memory);
	size_t previousBlockDataSize = _getDataSize(previousBlock);
	memcpy(newMemory, _memory, min(u64(previousBlockDataSize), u64(_size)));

	deallocate(_memory);

	return newMemory;
}


void MallocAllocator::deallocate(void* _memory)
{
	if (_memory == nullptr)
		return;

	Header* block = _getHeader(_memory);
	YAE_ASSERT(block != nullptr);

	u8* data = (u8*)block + sizeof(Header);
#if YAE_INITIALIZE_MEMORY
	memset(data, 0xDD, block->size);
#endif

	size_t blockSize = sizeof(Header) + block->size;
	free(block);

	--m_allocationCount;
	m_allocatedSize -= blockSize;

	YAE_ALLOCATION_CALLSTACK_CAPTURE_OUT(_memory);
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

u8* MallocAllocator::_getData(Header* _header)
{
	return (u8*)_header + sizeof(Header);
}

u8* MallocAllocator::_getDataStart(Header* _header)
{
	u8* data = _getData(_header);
	return (u8*)memory::alignForward(data, _header->alignment);
}

size_t MallocAllocator::_getDataSize(Header* _header)
{
	return _header->size - _header->alignment;
}

MallocAllocator& mallocAllocator()
{
	static MallocAllocator s_mallocAllocator;
	return s_mallocAllocator;
}

} // namespace yae
