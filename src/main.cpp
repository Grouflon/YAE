#include <context.h>
#include <memory.h>
#include <log.h>
#include <resource.h>
#include <application.h>
#include <profiling.h>

#include <mirror.h>
#include <imgui.h>

#include <stdio.h>
#include <cstdlib>

void* ImGuiMemAlloc(size_t _size, void* _userData)
{
    return yae::context().toolAllocator->allocate(_size);
}

void ImGuiMemFree(void* _ptr, void* _userData)
{
    return yae::context().toolAllocator->deallocate(_ptr);
}

int main(int _argc, char** _argv)
{
    ImGui::SetAllocatorFunctions(&ImGuiMemAlloc, &ImGuiMemFree, nullptr);

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

    yae::Profiler profiler(&toolAllocator);
    yae::context().profiler = &profiler;

    yae::Application application;
    yae::context().application = &application;

    application.init("yae", 800u, 600u, _argv, _argc);
    application.run();
    application.shutdown();

    return EXIT_SUCCESS;
}
