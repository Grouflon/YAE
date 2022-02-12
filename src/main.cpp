#include <yae/memory.h>
#include <yae/program.h>
#include <yae/application.h>

#include <stdio.h>
#include <cstdlib>

#define APPLICATION_NAME "yae | " YAE_CONFIG

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
