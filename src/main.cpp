#include <stdio.h>
#include <cstdlib>

//#include <GLFW/glfw3.h>
#include <mirror.h>

#include <context.h>
#include <memory.h>
#include <log.h>
#include <resource.h>
#include <application.h>

int main(int _argc, char** _argv)
{
    yae::FixedSizeAllocator allocator(1024*1024*32);
	yae::FixedSizeAllocator toolAllocator(1024*1024*32);
	yae::context().defaultAllocator = &allocator;
    yae::context().scratchAllocator = &allocator;
	yae::context().toolAllocator = &toolAllocator;

    yae::Logger logger;
    /*
    logger.setCategoryVerbosity("glfw", yae::LogVerbosity_Verbose);
    logger.setCategoryVerbosity("vulkan", yae::LogVerbosity_Verbose);
    logger.setCategoryVerbosity("vulkan_internal", yae::LogVerbosity_Log);
    logger.setCategoryVerbosity("resource", yae::LogVerbosity_Verbose);
    */
    yae::context().logger = &logger;

    yae::ResourceManager resourceManager;
    yae::context().resourceManager = &resourceManager;

    yae::Application application;
    application.init("yae", 800u, 600u, _argv, _argc);
    application.run();
    application.shutdown();

    return EXIT_SUCCESS;
}
