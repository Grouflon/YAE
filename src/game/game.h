#pragma once

#include "export.h"

#include <mirror/mirror.h>

#include <yae/types.h>

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

YAEGAME_API void onLibraryLoaded();
YAEGAME_API void onLibraryUnloaded();

YAEGAME_API void initGame();
YAEGAME_API void updateGame(float _dt);
YAEGAME_API void shutdownGame();

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
