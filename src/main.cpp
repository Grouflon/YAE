#include <yae/memory.h>
#include <yae/program.h>
#include <yae/logger.h>
#include <yae/Application.h>

#include <mirror.h>
#include <stdio.h>
#include <cstdlib>

#define YAE_EDITOR 1

int main(int _argc, char** _argv)
{
    // Init mirror reflection
    mirror::InitTypes();
    mirror::GetTypeSet().addTypeName(mirror::GetType<u8>(), "u8");
    mirror::GetTypeSet().addTypeName(mirror::GetType<u16>(), "u16");
    mirror::GetTypeSet().addTypeName(mirror::GetType<u32>(), "u32");
    mirror::GetTypeSet().addTypeName(mirror::GetType<u64>(), "u64");
    mirror::GetTypeSet().addTypeName(mirror::GetType<i8>(), "i8");
    mirror::GetTypeSet().addTypeName(mirror::GetType<i16>(), "i16");
    mirror::GetTypeSet().addTypeName(mirror::GetType<i32>(), "i32");
    mirror::GetTypeSet().addTypeName(mirror::GetType<i64>(), "i64");
    
    // Init Allocators
    yae::FixedSizeAllocator allocator = yae::FixedSizeAllocator(1024*1024*32);
    yae::FixedSizeAllocator scratchAllocator = yae::FixedSizeAllocator(1024*1024*32);
    yae::FixedSizeAllocator toolAllocator = yae::FixedSizeAllocator(1024*1024*32);
    yae::setAllocators(&allocator, &scratchAllocator, &toolAllocator);

    // Init Program
    {
        yae::Program program;

        program.logger().setCategoryVerbosity("OpenGL", yae::LogVerbosity_Verbose);
        program.logger().setCategoryVerbosity("resource", yae::LogVerbosity_Verbose);
        //program.logger().setCategoryVerbosity("input", yae::LogVerbosity_Verbose);
        //program.logger().setCategoryVerbosity("SDL", yae::LogVerbosity_Verbose);

#if YAE_RELEASE == 0
        program.registerModule("test");
#endif

#if YAE_EDITOR
        yae::Module* editorModule = program.registerModule("editor");
#endif
        yae::Module* gameModule = program.registerModule("game");

        program.init((const char**)_argv, _argc);

#if YAE_EDITOR
        yae::Application app = yae::Application("yae Editor | " YAE_CONFIG, 800u, 600u);
        app.addModule(editorModule);
        (void)gameModule;
#else
        yae::Application app = yae::Application("yae Game | " YAE_CONFIG, 800u, 600u);
        app.addModule(gameModule);
#endif
        program.registerApplication(&app);
        program.run();
        program.unregisterApplication(&app);

        program.shutdown();
    }

    return EXIT_SUCCESS;
}
