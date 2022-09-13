#include <yae/memory.h>
#include <yae/program.h>
#include <yae/application.h>
#include <yae/application.h>
#include <yae/logger.h>

#include <mirror.h>
#include <stdio.h>
#include <cstdlib>

#define APPLICATION_NAME "yae | " YAE_CONFIG

int main(int _argc, char** _argv)
{
    mirror::GetTypeSet().resolveTypes();

    yae::FixedSizeAllocator allocator = yae::FixedSizeAllocator(1024*1024*32);
    yae::FixedSizeAllocator scratchAllocator = yae::FixedSizeAllocator(1024*1024*32);
    yae::FixedSizeAllocator toolAllocator = yae::FixedSizeAllocator(1024*1024*32);
    yae::setAllocators(&allocator, &scratchAllocator, &toolAllocator);

    {
        yae::Program program;
        program.logger().setCategoryVerbosity("OpenGL", yae::LogVerbosity_Verbose);
        //program.logger().setCategoryVerbosity("input", yae::LogVerbosity_Verbose);
        program.init(_argv, _argc);

        yae::Application app = yae::Application(APPLICATION_NAME, 800u, 600u);
        program.registerApplication(&app);

        program.run();

        program.unregisterApplication(&app);

        program.shutdown();
    }

    return EXIT_SUCCESS;
}
