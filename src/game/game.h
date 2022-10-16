#pragma once

#ifndef GAME_API
#define GAME_API
#endif

#include <yae/types.h>
#include <yae/resource.h>

#include <mirror/mirror.h>

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

GAME_API void onModuleLoaded(yae::Program* _program, yae::Module* _module);
GAME_API void onModuleUnloaded(yae::Program* _program, yae::Module* _module);
GAME_API void initModule(yae::Program* _program, yae::Module* _module);
GAME_API void shutdownModule(yae::Program* _program, yae::Module* _module);
GAME_API void initApplication(yae::Application* _application);
GAME_API void updateApplication(yae::Application* _application, float _dt);
GAME_API void shutdownApplication(yae::Application* _application);

#ifdef __cplusplus
}
#endif

struct ConfigData
{
	u16 windowX;
	u16 windowY;
	u16 windowWidth;
	u16 windowHeight;
	bool fullscreen;
	float someFloat;
	double someDouble;
	char someChar;

	MIRROR_CLASS_NOVIRTUAL(ConfigData)
	(
		MIRROR_MEMBER(windowX)()
		MIRROR_MEMBER(windowY)()
		MIRROR_MEMBER(windowWidth)()
		MIRROR_MEMBER(windowHeight)()
		MIRROR_MEMBER(fullscreen)()
		MIRROR_MEMBER(someFloat)()
		MIRROR_MEMBER(someDouble)()
		MIRROR_MEMBER(someChar)()
	);
};

namespace yae {
class SuperResource : public Resource
{
	MIRROR_CLASS(SuperResource)
	(
		MIRROR_PARENT(yae::Resource)
	);

public:
	SuperResource(const char* _path) : Resource(_path) {}
	virtual~ SuperResource() {}

	virtual void _doLoad() override {}
	virtual void _doUnload() override {}
};

template <>
struct YAE_API ResourceIDGetter<SuperResource>
{
	static ResourceID GetId(const char* _path) { return ResourceID(_path); }
};
}
