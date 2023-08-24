#pragma once

#include <core/containers/Array.h>

#include <yae/types.h>

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

YAE_API void initModule(yae::Program* _program, yae::Module* _module);
YAE_API void shutdownModule(yae::Program* _program, yae::Module* _module);
YAE_API void onModuleReloaded(yae::Program* _program, yae::Module* _module);
YAE_API void startProgram(yae::Program* _program, yae::Module* _module);
YAE_API void stopProgram(yae::Program* _program, yae::Module* _module);
YAE_API void updateProgram(yae::Program* _program, yae::Module* _module);

#ifdef __cplusplus
}
#endif
