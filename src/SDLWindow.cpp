#pragma once

#include <SDL.h>
#include <SDL_syswm.h>
#include <vector>

#include "./SDLInstance.cpp"
#include "./RenderTarget.cpp"

class SDLWindow
{
private:
  int width;
  int height;
  SDLInstance *sdlInstance;
  SDL_Window *m_pCompanionWindow;
  Uint32 windowID;

public:
  RenderTarget *renderTarget;
  bool isOpen;
  SDLWindow(SDLInstance *sdlInstance, const char *title, int x, int y, int w, int h)
  {
    this->width = w;
    this->height = h;
    this->sdlInstance = sdlInstance;

    m_pCompanionWindow = SDL_CreateWindow(title, x, y, w, h, SDL_WINDOW_SHOWN);
    if (m_pCompanionWindow == NULL)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "SDL_CreateWindow - Window could not be created! SDL Error: %s\n", SDL_GetError());
      throw std::runtime_error(buf);
    }
    windowID = SDL_GetWindowID(m_pCompanionWindow);
    sdlInstance->windowEvents->insert_or_assign(windowID, new std::vector<SDL_WindowEvent>());
    isOpen = true;
  }

  ~SDLWindow()
  {
    SDL_DestroyWindow(m_pCompanionWindow);
  }

  void Update()
  {
    auto eventList = sdlInstance->windowEvents->at(windowID);
    for (int i = 0; i < eventList->size(); i++)
    {
      SDL_WindowEvent event = eventList->at(i);
      switch (event.event)
      {
      case SDL_WINDOWEVENT_CLOSE:
        isOpen = false;
        break;

      default:
        break;
      }
    }
    eventList->clear();
  }

  void SetWindowTitle(const char *title)
  {
    SDL_SetWindowTitle(m_pCompanionWindow, title);
  }
};