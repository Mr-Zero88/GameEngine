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

#include "./VulkanExtensions.cpp"

// VK_EXT_debug_report callback
static VkBool32 VKAPI_PTR VKDebugMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
                                                 size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage, void *pUserData)
{
  char buf[4096] = {0};
  switch (flags)
  {
  case VK_DEBUG_REPORT_ERROR_BIT_EXT:
    sprintf(buf, "VK ERROR %s %" PRIu64 ":%d: %s\n", pLayerPrefix, uint64_t(location), messageCode, pMessage);
    break;
  case VK_DEBUG_REPORT_WARNING_BIT_EXT:
    sprintf(buf, "VK WARNING %s %" PRIu64 ":%d: %s\n", pLayerPrefix, uint64_t(location), messageCode, pMessage);
    break;
  case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
    sprintf(buf, "VK PERF %s %" PRIu64 ":%d: %s\n", pLayerPrefix, uint64_t(location), messageCode, pMessage);
    break;
  case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
    sprintf(buf, "VK INFO %s %" PRIu64 ":%d: %s\n", pLayerPrefix, uint64_t(location), messageCode, pMessage);
    break;
  case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
    sprintf(buf, "VK DEBUG %s %" PRIu64 ":%d: %s\n", pLayerPrefix, uint64_t(location), messageCode, pMessage);
    break;
  default:
    break;
  }

  printf("%s\n", buf);

  return VK_FALSE;
}

class VulkanInstance
{
public:
  VkInstance m_pInstance;
  VkDebugReportCallbackEXT m_pDebugReportCallback;
  VulkanInstance(const char *ApplicationName)
  {
    std::vector<std::string> requiredInstanceExtensions;
    GetVulkanInstanceExtensionsRequired(requiredInstanceExtensions);

    // Additional required instance extensions
    requiredInstanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(_WIN32)
    requiredInstanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
    requiredInstanceExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

    uint32_t nEnabledLayerCount = 0;
    VkLayerProperties *pLayerProperties = nullptr;
    char **ppEnabledLayerNames = nullptr;

    char const *pInstanceValidationLayers[] =
        {
            "VK_LAYER_GOOGLE_threading",
            "VK_LAYER_LUNARG_parameter_validation",
            "VK_LAYER_LUNARG_object_tracker",
            "VK_LAYER_LUNARG_image",
            "VK_LAYER_LUNARG_core_validation",
            "VK_LAYER_LUNARG_swapchain"};

    uint32_t nInstanceLayerCount = 0;
    VkResult nResult = vkEnumerateInstanceLayerProperties(&nInstanceLayerCount, nullptr);
    if (nResult == VK_SUCCESS && nInstanceLayerCount > 0)
    {
      pLayerProperties = new VkLayerProperties[nInstanceLayerCount];
      ppEnabledLayerNames = new char *[nInstanceLayerCount];
      nResult = vkEnumerateInstanceLayerProperties(&nInstanceLayerCount, pLayerProperties);
      if (nResult != VK_SUCCESS)
      {
        char buf[1024];
        sprintf_s(buf, sizeof(buf), "Error vkEnumerateInstanceLayerProperties in %d\n", nResult);
        throw std::runtime_error(buf);
      }

      uint32_t nLayerIndex = 0;
      for (nLayerIndex = 0; nLayerIndex < nInstanceLayerCount; nLayerIndex++)
      {
        for (uint32_t nLayer = 0; nLayer < _countof(pInstanceValidationLayers); nLayer++)
        {
          if (strstr(pLayerProperties[nLayerIndex].layerName, pInstanceValidationLayers[nLayer]) != NULL)
          {
            ppEnabledLayerNames[nEnabledLayerCount++] = pLayerProperties[nLayerIndex].layerName;
          }
        }
      }
      requiredInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    uint32_t nInstanceExtensionCount = 0;
    nResult = vkEnumerateInstanceExtensionProperties(NULL, &nInstanceExtensionCount, NULL);
    if (nResult != VK_SUCCESS)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "vkEnumerateInstanceExtensionProperties failed with error %d\n", nResult);
      throw std::runtime_error(buf);
    }

    // Allocate enough ExtensionProperties to support all extensions being enabled
    char **ppEnableInstanceExtensionNames = new char *[requiredInstanceExtensions.size()];
    int32_t nEnableInstanceExtensionNamesCount = 0;
    VkExtensionProperties *pExtensionProperties = new VkExtensionProperties[nInstanceExtensionCount];
    if (nInstanceExtensionCount > 0)
    {
      nResult = vkEnumerateInstanceExtensionProperties(NULL, &nInstanceExtensionCount, pExtensionProperties);
      if (nResult != VK_SUCCESS)
      {
        char buf[1024];
        sprintf_s(buf, sizeof(buf), "vkEnumerateInstanceExtensionProperties failed with error %d\n", nResult);
        throw std::runtime_error(buf);
      }

      for (size_t nExt = 0; nExt < requiredInstanceExtensions.size(); nExt++)
      {
        bool bFound = false;
        uint32_t nExtIndex = 0;
        for (nExtIndex = 0; nExtIndex < nInstanceExtensionCount; nExtIndex++)
        {
          if (strcmp(requiredInstanceExtensions[nExt].c_str(), pExtensionProperties[nExtIndex].extensionName) == 0)
          {
            bFound = true;
            ppEnableInstanceExtensionNames[nEnableInstanceExtensionNamesCount++] = pExtensionProperties[nExtIndex].extensionName;
            break;
          }
        }

        if (!bFound)
        {
          char buf[1024];
          sprintf_s(buf, sizeof(buf), "Vulkan missing requested extension '%s'.\n", requiredInstanceExtensions[nExt].c_str());
          throw std::runtime_error(buf);
        }
      }
    }

    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.pApplicationName = ApplicationName;
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = nullptr;
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    // Create the instance
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = nEnableInstanceExtensionNamesCount;
    instanceCreateInfo.ppEnabledExtensionNames = ppEnableInstanceExtensionNames;
    instanceCreateInfo.enabledLayerCount = nEnabledLayerCount;
    instanceCreateInfo.ppEnabledLayerNames = ppEnabledLayerNames;

    nResult = vkCreateInstance(&instanceCreateInfo, nullptr, &m_pInstance);
    if (nResult != VK_SUCCESS)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "vkCreateInstance failed with error %d\n", nResult);
      throw std::runtime_error(buf);
    }

    // Enable debug report extension
    PFN_vkCreateDebugReportCallbackEXT g_pVkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_pInstance, "vkCreateDebugReportCallbackEXT");
    PFN_vkDestroyDebugReportCallbackEXT g_pVkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_pInstance, "vkDestroyDebugReportCallbackEXT");

    VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo = {};
    debugReportCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    debugReportCreateInfo.pNext = NULL;
    debugReportCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT;
    debugReportCreateInfo.pfnCallback = VKDebugMessageCallback;
    debugReportCreateInfo.pUserData = NULL;
    g_pVkCreateDebugReportCallbackEXT(m_pInstance, &debugReportCreateInfo, NULL, &m_pDebugReportCallback);

    delete[] ppEnableInstanceExtensionNames;
    delete[] ppEnabledLayerNames;
    delete[] pLayerProperties;
    delete[] pExtensionProperties;
  }
};