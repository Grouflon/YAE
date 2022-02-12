#include <yae/memory.h>
#include <yae/program.h>
#include <yae/application.h>

#include <stdio.h>
#include <cstdlib>

#if YAE_DEBUG
#define APPLICATION_NAME "yae | Debug"
#elif YAE_RELEASE
#define APPLICATION_NAME "yae | Release"
#else
#define APPLICATION_NAME "yae | Unknown"
#endif

int main(int _argc, char** _argv)
{
    yae::FixedSizeAllocator allocator = yae::FixedSizeAllocator(1024*1024*32);
    yae::FixedSizeAllocator scratchAllocator = yae::FixedSizeAllocator(1024*1024*32);
    yae::FixedSizeAllocator toolAllocator = yae::FixedSizeAllocator(1024*1024*32);

    yae::Program program(&allocator, &scratchAllocator, &toolAllocator);
    //yae::Program program(&yae::mallocAllocator(), &yae::mallocAllocator(), &yae::mallocAllocator());
    program.init(_argv, _argc);

    yae::Application app = yae::Application(APPLICATION_NAME, 800u, 600u);
    program.registerApplication(&app);

    program.run();

    program.unregisterApplication(&app);

    program.shutdown();

    return EXIT_SUCCESS;
}
