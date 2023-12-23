#pragma once
#pragma warning(push)
#pragma warning(disable : 4996)
#pragma warning(disable : 4267)

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define SDL_VIDEO_DRIVER_X11
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <inttypes.h>
#include <openvr.h>
#include <deque>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#include "lodepng.cpp"
#include "matrices.cpp"
#include "pathtools.cpp"

#if defined(POSIX)
#include "unistd.h"
#endif

class SDLWindow
{
private:
  SDL_Window *m_pCompanionWindow;

public:
  SDLWindow(const char *title, int x, int y, int w, int h)
  {
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
  }

  void SetWindowTitle(const char *title)
  {
    SDL_SetWindowTitle(m_pCompanionWindow, title);
  }
};