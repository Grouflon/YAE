#pragma once

#include <core/types.h>

#include <SDL.h>

#define YAE_SDL_VERIFY(_instruction) { int __result = _instruction; YAE_ASSERT_MSGF(__result >= 0, "SDL Error (%s): %s", #_instruction, SDL_GetError()); }
