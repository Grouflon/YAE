#include <yae/context.h>
#include <yae/memory.h>
#include <yae/log.h>
#include <yae/resource.h>
#include <yae/application.h>
#include <yae/profiling.h>
#include <yae/game_module.h>

#include <mirror/mirror.h>
#include <imgui/imgui.h>

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

    yae::FixedSizeAllocator allocator = yae::FixedSizeAllocator(1024*1024*32);
    yae::FixedSizeAllocator scratchAllocator = yae::FixedSizeAllocator(1024*1024*32);
    yae::FixedSizeAllocator toolAllocator = yae::FixedSizeAllocator(1024*1024*32);

	yae::context().defaultAllocator = &allocator;
    yae::context().scratchAllocator = &scratchAllocator;
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

    yae::loadGameAPI();

    YAE_CAPTURE_START("init");
    application.init("yae", 800u, 600u, _argv, _argc);
    YAE_CAPTURE_STOP("init");

    {
        yae::String dump;
        profiler.dumpCapture("init", dump);
        printf(dump.c_str());
        printf("\n");
    }

    application.run();

    {
        yae::String dump;
        profiler.dumpCapture("frame", dump);
        printf(dump.c_str());
        printf("\n");
    }

    YAE_CAPTURE_START("shutdown");
    application.shutdown();
    YAE_CAPTURE_STOP("shutdown");

    {
        yae::String dump;
        profiler.dumpCapture("shutdown", dump);
        printf(dump.c_str());
        printf("\n");
    }

    resourceManager.flushResources();

    yae::unloadGameAPI();

    return EXIT_SUCCESS;
}
