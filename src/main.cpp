#include <core/memory.h>
#include <core/Program.h>
#include <core/logger.h>

#include <mirror.h>
#include <stdio.h>
#include <cstdlib>


int main(int _argc, char** _argv)
{
    // Init mirror reflection
    mirror::InitNewTypes();
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

        // Modules
        program.registerModule("yae");
        program.registerModule("game");

        // Loop
        program.init((const char**)_argv, _argc);
        program.run();
        program.shutdown();
    }

    return EXIT_SUCCESS;
}
