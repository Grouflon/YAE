#pragma once

#include <types.h>

namespace yae {

YAELIB_API void loadGameAPI();
YAELIB_API void unloadGameAPI();

void watchGameAPI();

void initGame();
void updateGame();
void shutdownGame();

} // namespace yae
