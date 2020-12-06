#include <stdio.h>

#include <SDL.h>

#include <Log.h>

int main(int _argc, char** _argv)
{
  yae::g_logger.setCategoryVerbosity("SDL", yae::LogVerbosity_Verbose);

  int result = SDL_Init(SDL_INIT_TIMER|SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER);
  if (result != 0)
  {
    YAE_ERRORF_CAT("SDL", "Failed to initialize SDL: %s", SDL_GetError());
    return 1;
  }
  else
  {
    YAE_VERBOSE_CAT("SDL", "Successfully initialized SDL");
  }

  SDL_Quit();
  YAE_VERBOSE_CAT("SDL", "Successfully shut down SDL");

  return 0;
}
