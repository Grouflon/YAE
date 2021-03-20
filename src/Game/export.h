#pragma once

#ifdef YAEGAME_EXPORT
#define YAEGAME_API __declspec(dllexport)
#else
#define YAEGAME_API __declspec(dllimport)
#endif
