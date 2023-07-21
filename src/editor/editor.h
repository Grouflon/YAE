#pragma once

#ifndef EDITOR_API
#define EDITOR_API
#endif

#include <yae/types.h>

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

EDITOR_API void onModuleLoaded(yae::Program* _program, yae::Module* _module);
EDITOR_API void onModuleUnloaded(yae::Program* _program, yae::Module* _module);
EDITOR_API void initModule(yae::Program* _program, yae::Module* _module);
EDITOR_API void shutdownModule(yae::Program* _program, yae::Module* _module);
EDITOR_API void beforeInitApplication(yae::Application* _application);
EDITOR_API void afterInitApplication(yae::Application* _application);
EDITOR_API void onApplicationModuleReloaded(yae::Application* _application, yae::Module* _module);
EDITOR_API void updateApplication(yae::Application* _application, float _dt);
EDITOR_API void beforeShutdownApplication(yae::Application* _application);
EDITOR_API void afterShutdownApplication(yae::Application* _application);
EDITOR_API bool onSerializeApplicationSettings(yae::Application* _application, yae::Serializer* _serializer);

#ifdef __cplusplus
}
#endif
