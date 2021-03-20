#pragma once

#include <export.h>

namespace yae
{
	class Logger;
	class ResourceManager;

	struct YAELIB_API GlobalContext
	{
		Logger* logger = nullptr;
		ResourceManager* resourceManager = nullptr;
	};

	extern GlobalContext g_context;

	YAELIB_API GlobalContext* GetGlobalContext();
}