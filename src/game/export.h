#pragma once

#ifndef YAEGAME_API
	#ifdef YAEGAME_EXPORT
		#define YAEGAME_API __declspec(dllexport)
	#else
		#define YAEGAME_API __declspec(dllimport)
	#endif
#endif
