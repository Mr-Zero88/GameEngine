#pragma once

#include <SDL.h>
#include <SDL_syswm.h>

#include "./RenderTarget.cpp"

class SDLWindow
{
private:
  int width;
  int height;
  SDL_Window *m_pCompanionWindow;
  Uint32 windowID;

public:
  RenderTarget *renderTarget;
  bool isOpen;
  SDLWindow(const char *title, int x, int y, int w, int h)
  {
    this->width = w;
    this->height = h;
    // if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    // {
    //   char buf[1024];
    //   sprintf_s(buf, sizeof(buf), "SDL_Init - SDL could not initialize! SDL Error: %s\n", SDL_GetError());
    //   throw std::runtime_error(buf);
    // }

    m_pCompanionWindow = SDL_CreateWindow(title, x, y, w, h, SDL_WINDOW_SHOWN);
    if (m_pCompanionWindow == NULL)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "SDL_CreateWindow - Window could not be created! SDL Error: %s\n", SDL_GetError());
      throw std::runtime_error(buf);
    }
    windowID = SDL_GetWindowID(m_pCompanionWindow);
    isOpen = true;
  }

  ~SDLWindow()
  {
    SDL_DestroyWindow(m_pCompanionWindow);
  }

  void Update()
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.window.windowID != windowID)
        return;
      if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
        isOpen = false;
    }
  }

  void SetWindowTitle(const char *title)
  {
    SDL_SetWindowTitle(m_pCompanionWindow, title);
  }
};