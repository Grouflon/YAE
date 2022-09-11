#include "types.h"

#include <yae/program.h>
#include <yae/application.h>

namespace yae {

Allocator* g_defaultAllocator = nullptr;
Allocator* g_scratchAllocator = nullptr;
Allocator* g_toolAllocator = nullptr;

Program& program()
{
	YAE_ASSERT(Program::s_programInstance != nullptr);
	return *Program::s_programInstance;
}


Application& app()
{
	return program().currentApplication();
}


Allocator& defaultAllocator()
{
	YAE_ASSERT(g_defaultAllocator != nullptr);
	return *g_defaultAllocator;
}


Allocator& scratchAllocator()
{
	YAE_ASSERT(g_scratchAllocator != nullptr);
	return *g_scratchAllocator;
}


Allocator& toolAllocator()
{
	YAE_ASSERT(g_toolAllocator != nullptr);
	return *g_toolAllocator;
}


Profiler& profiler()
{
	return program().profiler();
}


Logger& logger()
{
	return program().logger();
}


ResourceManager& resourceManager()
{
	return program().resourceManager();
}


InputSystem& input()
{
	return app().input();
}


Renderer& renderer()
{
	return app().renderer();
}

void setAllocators(Allocator* _defaultAllocator, Allocator* _scratchAllocator, Allocator* _toolAllocator)
{
	g_defaultAllocator = _defaultAllocator;
	g_defaultAllocator = _scratchAllocator;
	g_defaultAllocator = _toolAllocator;
}


} // namespace yae
