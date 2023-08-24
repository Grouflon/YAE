#pragma once

#include <yae/types.h>

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

EDITOR_API void initModule(yae::Program* _program, yae::Module* _module);
EDITOR_API void shutdownModule(yae::Program* _program, yae::Module* _module);
EDITOR_API void onModuleReloaded(yae::Program* _program, yae::Module* _module);
EDITOR_API void startProgram(yae::Program* _program, yae::Module* _module);
EDITOR_API void stopProgram(yae::Program* _program, yae::Module* _module);
EDITOR_API void getDependencies(const char*** _outModuleNames, int* _outModuleCount);

#ifdef __cplusplus
}
#endif
