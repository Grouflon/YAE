#include "types.h"

#include <yae/program.h>
#include <yae/application.h>

namespace yae {

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
	return program().defaultAllocator();
}


Allocator& scratchAllocator()
{
	return program().scratchAllocator();
}


Allocator& toolAllocator()
{
	return program().toolAllocator();
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


VulkanRenderer& renderer()
{
	return app().renderer();
}


} // namespace yae
