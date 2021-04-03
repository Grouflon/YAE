#pragma once

#include <types.h>

namespace yae {

class Allocator;
class Logger;
class ResourceManager;

struct YAELIB_API Context
{
	Allocator* defaultAllocator = nullptr;
	Allocator* scratchAllocator = nullptr;

	Logger* logger = nullptr;
	ResourceManager* resourceManager = nullptr;
};

extern Context g_context;
YAELIB_API Context& context();

} // namespace yae
