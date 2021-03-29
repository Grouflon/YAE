#pragma once

#include <export.h>

namespace yae {

class YAELIB_API Allocator
{
public:
	virtual void* allocate(size_t _size) = 0;
	virtual void deallocate(void* _memory) = 0;
};

};
