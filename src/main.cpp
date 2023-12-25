#pragma once
#include <string>
#include <SDL.h>
#include <iostream>

#include "./Utility.cpp"
#include "./VulkanInstance.cpp"
#include "./Renderer.cpp"
#include "./OpenVRInstance.cpp"
#include "./SDLWindow.cpp"

int main(int argc, char *argv[])
{
  try
  {
    VulkanInstance *vulkanInstance = new VulkanInstance("Hello World");
    Renderer *renderer = new Renderer(vulkanInstance);
    OpenVRInstance *vrInstance = new OpenVRInstance();
    SDLWindow *window = new SDLWindow(std::string("Hello World - " + vrInstance->Driver + " " + vrInstance->Display).c_str(), 700, 100, 640, 320);

    while (window->isOpen && vrInstance->isOpen)
    {
      window->Update();
      vrInstance->Update();
      renderer->Render(window->renderTarget);
      renderer->Render(vrInstance->renderTarget);
    }

    delete (window);
    delete (vrInstance);
    delete (renderer);
    delete (vulkanInstance);
  }
  catch (std::runtime_error error)
  {
    std::cout << "Runtime Error: " + std::string(error.what());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", formatError(error).c_str(), NULL);
    return 1;
  }
  return 0;
}