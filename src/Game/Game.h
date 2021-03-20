#pragma once

#include "export.h"

#include <mirror.h>

#include <00-Type/IntTypes.h>

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

YAEGAME_API void OnLibraryLoaded();
YAEGAME_API void OnLibraryUnloaded();

YAEGAME_API void InitGame();
YAEGAME_API void UpdateGame();
YAEGAME_API void ShutdownGame();

#ifdef __cplusplus
}
#endif

struct ConfigData
{
	yae::u16 windowX;
	yae::u16 windowY;
	yae::u16 windowWidth;
	yae::u16 windowHeight;
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
