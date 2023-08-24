#pragma once

#ifndef TEST_API
#define TEST_API
#endif

#include <yae/types.h>

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

TEST_API void initModule(yae::Program* _program, yae::Module* _module);
TEST_API void shutdownModule(yae::Program* _program, yae::Module* _module);
TEST_API void getDependencies(const char*** _outModuleNames, int* _outModuleCount);

#ifdef __cplusplus
}
#endif
