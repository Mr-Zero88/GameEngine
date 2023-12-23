#pragma once
#include <string>
#include <SDL.h>
#include <iostream>

#include "./OpenVRInstance.cpp"
#include "./SDLWindow.cpp"
#include "./VulkanInstance.cpp"
#include "./Utility.cpp"

int main(int argc, char *argv[])
{
  try
  {
    OpenVRInstance *vrInstance = new OpenVRInstance();
    SDLWindow *window = new SDLWindow(std::string("Hello World - " + vrInstance->Driver + " " + vrInstance->Display).c_str(), 700, 100, 640, 320);
    VulkanInstance *vulkanInstance = new VulkanInstance("Hello World");
  }
  catch (std::runtime_error error)
  {
    std::cout << "Runtime Error: " + std::string(error.what());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", formatError(error).c_str(), NULL);
    return 1;
  }
  return 0;
}