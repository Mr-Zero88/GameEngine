#pragma once
#include <string>
#include <SDL.h>
#include <iostream>

#include "./OpenVRInstance.cpp"
#include "./SDLWindow.cpp"
#include "./VulkanInstance.cpp"
#include "./VulkanDevice.cpp"
#include "./VulkanSwapchain.cpp"
#include "./VulkanCommandPool.cpp"
#include "./VulkanRenderer.cpp"
#include "./Utility.cpp"

int main(int argc, char *argv[])
{
  try
  {
    OpenVRInstance *vrInstance = new OpenVRInstance();
    SDLWindow *window = new SDLWindow(std::string("Hello World - " + vrInstance->Driver + " " + vrInstance->Display).c_str(), 700, 100, 640, 320);
    VulkanInstance *vulkanInstance = new VulkanInstance("Hello World");
    VulkanDevice *vulkanDevice = new VulkanDevice(vulkanInstance, vrInstance);
    VulkanSwapchain *vulkanSwapchain = new VulkanSwapchain(vulkanDevice, window);
    VulkanCommandPool *vulkanCommandPool = new VulkanCommandPool(vulkanDevice);
    VulkanRenderer *vulkanRenderer = new VulkanRenderer(vrInstance, vulkanCommandPool, vulkanDevice, vulkanSwapchain);

    SDL_StartTextInput();
    SDL_ShowCursor(SDL_DISABLE);
    while (true)
    {
      vulkanRenderer->RenderFrame();
    }
    SDL_StopTextInput();
  }
  catch (std::runtime_error error)
  {
    std::cout << "Runtime Error: " + std::string(error.what());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", formatError(error).c_str(), NULL);
    return 1;
  }
  return 0;
}