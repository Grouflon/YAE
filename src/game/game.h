#pragma once

#include "export.h"

#include <mirror/mirror.h>

#include <yae/types.h>
#include <yae/resource.h>

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

YAEGAME_API void onLibraryLoaded();
YAEGAME_API void onLibraryUnloaded();

YAEGAME_API void initGame();
YAEGAME_API void shutdownGame();
YAEGAME_API void updateGame(float _dt);

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
struct YAELIB_API ResourceIDGetter<SuperResource>
{
	static ResourceID GetId(const char* _path) { return ResourceID(_path); }
};
}
