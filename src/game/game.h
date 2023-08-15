#pragma once

#ifndef GAME_API
#define GAME_API
#endif

#include <yae/types.h>

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

GAME_API void onModuleLoaded(yae::Program* _program, yae::Module* _module);
GAME_API void onModuleUnloaded(yae::Program* _program, yae::Module* _module);
GAME_API void initModule(yae::Program* _program, yae::Module* _module);
GAME_API void shutdownModule(yae::Program* _program, yae::Module* _module);
GAME_API void initApplication(yae::Application* _application, const char** _args, int _argCount);
GAME_API void updateApplication(yae::Application* _application, float _dt);
GAME_API void shutdownApplication(yae::Application* _application);
GAME_API bool onSerializeApplicationSettings(yae::Application* _application, yae::Serializer* _serializer);

#ifdef __cplusplus
}
#endif
