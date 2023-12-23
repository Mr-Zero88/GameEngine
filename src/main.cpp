#pragma once
#include <string>
#include <SDL.h>
#include <iostream>

#include "./OpenVRInstance.cpp"
#include "./SDLWindow.cpp"
#include "./VulkanInstance.cpp"

bool replace(std::string &str, const std::string &from, const std::string &to)
{
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

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
    auto errorstr = std::string(error.what());
    replace(errorstr, ": ", "\n");
    replace(errorstr, ": ", "\n");
    replace(errorstr, ": ", "\n");
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", errorstr.c_str(), NULL);
    return 1;
  }
  return 0;
}