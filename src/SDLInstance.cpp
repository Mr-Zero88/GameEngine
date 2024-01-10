#pragma once

#include <SDL.h>
#include <SDL_syswm.h>
#include <stdexcept>
#include <map>
#include <vector>

class SDLInstance
{
public:
  std::map<Uint32, std::vector<SDL_WindowEvent> *> *windowEvents;
  SDLInstance()
  {
    windowEvents = new std::map<Uint32, std::vector<SDL_WindowEvent> *>();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "SDL_Init - SDL could not initialize! SDL Error: %s\n", SDL_GetError());
      throw std::runtime_error(buf);
    }
  }

  ~SDLInstance()
  {
    SDL_Quit();
  }

  void Update()
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_WINDOWEVENT:
        windowEvents->at(event.window.windowID)->push_back(event.window);
        break;
      }
    }
  }
};