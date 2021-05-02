#pragma once

#include <yae/types.h>

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

} // namespace yae
