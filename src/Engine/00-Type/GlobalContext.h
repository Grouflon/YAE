#pragma once

#include <export.h>

namespace yae
{
	class Logger;
	class ResourceManager;
	class Allocator;

	struct YAELIB_API GlobalContext
	{
		Allocator* defaultAllocator = nullptr;
		Allocator* scratchAllocator = nullptr;

		Logger* logger = nullptr;
		ResourceManager* resourceManager = nullptr;
	};

	extern GlobalContext g_context;

	YAELIB_API GlobalContext* GetGlobalContext();
}