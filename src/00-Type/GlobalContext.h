#pragma once

namespace yae
{
	class Logger;
	class ResourceManager;

	struct GlobalContext
	{
		Logger* logger = nullptr;
		ResourceManager* resourceManager = nullptr;
	};

	extern GlobalContext g_context;
}