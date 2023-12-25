#pragma once

#include "./VulkanInstance.cpp"
#include "./VulkanExtensions.cpp"
#include "./OpenVRInstance.cpp"

class VulkanDevice
{
public:
  VulkanInstance *vulkanInstance;
  OpenVRInstance *vrInstance;
  VkPhysicalDevice m_pPhysicalDevice;
  VkPhysicalDeviceProperties m_physicalDeviceProperties;
  VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;
  VkPhysicalDeviceFeatures m_physicalDeviceFeatures;
  uint32_t m_nQueueFamilyIndex;
  VkQueue m_pQueue;
  VkDevice m_pDevice;
  VulkanDevice(VulkanInstance *vulkanInstance, OpenVRInstance *vrInstance)
  {
    this->vulkanInstance = vulkanInstance;
    this->vrInstance = vrInstance;
    uint32_t nDeviceCount = 0;
    VkResult nResult = vkEnumeratePhysicalDevices(vulkanInstance->m_pInstance, &nDeviceCount, NULL);
    if (nResult != VK_SUCCESS || nDeviceCount == 0)
    {
      throw std::runtime_error("vkEnumeratePhysicalDevices failed, unable to init and enumerate GPUs with Vulkan.\n");
    }

    VkPhysicalDevice *pPhysicalDevices = new VkPhysicalDevice[nDeviceCount];
    nResult = vkEnumeratePhysicalDevices(vulkanInstance->m_pInstance, &nDeviceCount, pPhysicalDevices);
    if (nResult != VK_SUCCESS || nDeviceCount == 0)
    {
      throw std::runtime_error("vkEnumeratePhysicalDevices failed, unable to init and enumerate GPUs with Vulkan.\n");
    }

    // Query OpenVR for the physical device to use
    uint64_t pHMDPhysicalDevice = 0;
    vrInstance->m_pHMD->GetOutputDevice(&pHMDPhysicalDevice, vr::TextureType_Vulkan, (VkInstance_T *)vulkanInstance->m_pInstance);

    // Select the HMD physical device
    m_pPhysicalDevice = VK_NULL_HANDLE;
    for (uint32_t nPhysicalDevice = 0; nPhysicalDevice < nDeviceCount; nPhysicalDevice++)
    {
      if (((VkPhysicalDevice)pHMDPhysicalDevice) == pPhysicalDevices[nPhysicalDevice])
      {
        m_pPhysicalDevice = (VkPhysicalDevice)pHMDPhysicalDevice;
        break;
      }
    }
    if (m_pPhysicalDevice == VK_NULL_HANDLE)
    {
      // Fallback: Grab the first physical device
      printf("Failed to find GetOutputDevice VkPhysicalDevice, falling back to choosing first device.\n");
      m_pPhysicalDevice = pPhysicalDevices[0];
    }
    delete[] pPhysicalDevices;

    vkGetPhysicalDeviceProperties(m_pPhysicalDevice, &m_physicalDeviceProperties);
    vkGetPhysicalDeviceMemoryProperties(m_pPhysicalDevice, &m_physicalDeviceMemoryProperties);
    vkGetPhysicalDeviceFeatures(m_pPhysicalDevice, &m_physicalDeviceFeatures);

    //--------------------//
    // VkDevice creation  //
    //--------------------//
    // Query OpenVR for the required device extensions for this physical device
    std::vector<std::string> requiredDeviceExtensions;
    GetVulkanDeviceExtensionsRequired(m_pPhysicalDevice, requiredDeviceExtensions);
    // Add additional required extensions
    requiredDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Find the first graphics queue
    uint32_t nQueueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_pPhysicalDevice, &nQueueCount, 0);
    VkQueueFamilyProperties *pQueueFamilyProperties = new VkQueueFamilyProperties[nQueueCount];
    vkGetPhysicalDeviceQueueFamilyProperties(m_pPhysicalDevice, &nQueueCount, pQueueFamilyProperties);
    if (nQueueCount == 0)
    {
      throw std::runtime_error("Failed to get queue properties.\n");
    }
    uint32_t nGraphicsQueueIndex = 0;
    for (nGraphicsQueueIndex = 0; nGraphicsQueueIndex < nQueueCount; nGraphicsQueueIndex++)
    {
      if (pQueueFamilyProperties[nGraphicsQueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
        break;
      }
    }
    if (nGraphicsQueueIndex >= nQueueCount)
    {
      throw std::runtime_error("No graphics queue found.\n");
    }
    m_nQueueFamilyIndex = nGraphicsQueueIndex;
    delete[] pQueueFamilyProperties;

    uint32_t nDeviceExtensionCount = 0;
    nResult = vkEnumerateDeviceExtensionProperties(m_pPhysicalDevice, NULL, &nDeviceExtensionCount, NULL);
    if (nResult != VK_SUCCESS)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "vkEnumerateDeviceExtensionProperties failed with error %d\n", nResult);
      throw std::runtime_error(buf);
    }

    // Allocate enough ExtensionProperties to support all extensions being enabled
    const char **ppDeviceExtensionNames = new const char *[nDeviceExtensionCount];
    uint32_t nEnabledDeviceExtensionCount = 0;

    // Enable required device extensions
    VkExtensionProperties *pDeviceExtProperties = new VkExtensionProperties[nDeviceExtensionCount];
    memset(pDeviceExtProperties, 0, sizeof(VkExtensionProperties) * nDeviceExtensionCount);
    if (nDeviceExtensionCount > 0)
    {
      nResult = vkEnumerateDeviceExtensionProperties(m_pPhysicalDevice, NULL, &nDeviceExtensionCount, pDeviceExtProperties);
      if (nResult != VK_SUCCESS)
      {
        char buf[1024];
        sprintf_s(buf, sizeof(buf), "vkEnumerateDeviceExtensionProperties failed with error %d\n", nResult);
        throw std::runtime_error(buf);
      }

      for (size_t nRequiredDeviceExt = 0; nRequiredDeviceExt < requiredDeviceExtensions.size(); nRequiredDeviceExt++)
      {
        bool bExtFound = false;
        for (uint32_t nDeviceExt = 0; nDeviceExt < nDeviceExtensionCount; nDeviceExt++)
        {
          if (stricmp(requiredDeviceExtensions[nRequiredDeviceExt].c_str(), pDeviceExtProperties[nDeviceExt].extensionName) == 0)
          {
            bExtFound = true;
            break;
          }
        }

        if (bExtFound)
        {
          ppDeviceExtensionNames[nEnabledDeviceExtensionCount] = requiredDeviceExtensions[nRequiredDeviceExt].c_str();
          nEnabledDeviceExtensionCount++;
        }
      }
    }

    // Create the device
    VkDeviceQueueCreateInfo deviceQueueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    deviceQueueCreateInfo.queueFamilyIndex = m_nQueueFamilyIndex;
    deviceQueueCreateInfo.queueCount = 1;
    float fQueuePriority = 1.0f;
    deviceQueueCreateInfo.pQueuePriorities = &fQueuePriority;

    VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = nEnabledDeviceExtensionCount;
    deviceCreateInfo.ppEnabledExtensionNames = ppDeviceExtensionNames;
    deviceCreateInfo.pEnabledFeatures = &m_physicalDeviceFeatures;

    nResult = vkCreateDevice(m_pPhysicalDevice, &deviceCreateInfo, nullptr, &m_pDevice);
    if (nResult != VK_SUCCESS)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "vkEnumevkCreateDevice failed with error %d\n", nResult);
      throw std::runtime_error(buf);
    }

    // Get the device queue
    vkGetDeviceQueue(m_pDevice, m_nQueueFamilyIndex, 0, &m_pQueue);
  }
};