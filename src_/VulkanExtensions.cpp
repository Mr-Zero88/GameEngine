#pragma once

#include <vulkan.h>
#include <string>
#include <vector>
#include <openvr.h>

void GetVulkanInstanceExtensionsRequired(std::vector<std::string> &outInstanceExtensionList)
{
  outInstanceExtensionList.clear();
  uint32_t nBufferSize = vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(nullptr, 0);
  if (nBufferSize > 0)
  {
    // Allocate memory for the space separated list and query for it
    char *pExtensionStr = new char[nBufferSize];
    pExtensionStr[0] = 0;
    vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(pExtensionStr, nBufferSize);

    // Break up the space separated list into entries on the CUtlStringList
    std::string curExtStr;
    uint32_t nIndex = 0;
    while (pExtensionStr[nIndex] != 0 && (nIndex < nBufferSize))
    {
      if (pExtensionStr[nIndex] == ' ')
      {
        outInstanceExtensionList.push_back(curExtStr);
        curExtStr.clear();
      }
      else
      {
        curExtStr += pExtensionStr[nIndex];
      }
      nIndex++;
    }
    if (curExtStr.size() > 0)
    {
      outInstanceExtensionList.push_back(curExtStr);
    }

    delete[] pExtensionStr;
  }
}

void GetVulkanDeviceExtensionsRequired(VkPhysicalDevice pPhysicalDevice, std::vector<std::string> &outDeviceExtensionList)
{
  outDeviceExtensionList.clear();
  uint32_t nBufferSize = vr::VRCompositor()->GetVulkanDeviceExtensionsRequired((VkPhysicalDevice_T *)pPhysicalDevice, nullptr, 0);
  if (nBufferSize > 0)
  {
    // Allocate memory for the space separated list and query for it
    char *pExtensionStr = new char[nBufferSize];
    pExtensionStr[0] = 0;
    vr::VRCompositor()->GetVulkanDeviceExtensionsRequired((VkPhysicalDevice_T *)pPhysicalDevice, pExtensionStr, nBufferSize);

    // Break up the space separated list into entries on the CUtlStringList
    std::string curExtStr;
    uint32_t nIndex = 0;
    while (pExtensionStr[nIndex] != 0 && (nIndex < nBufferSize))
    {
      if (pExtensionStr[nIndex] == ' ')
      {
        outDeviceExtensionList.push_back(curExtStr);
        curExtStr.clear();
      }
      else
      {
        curExtStr += pExtensionStr[nIndex];
      }
      nIndex++;
    }
    if (curExtStr.size() > 0)
    {
      outDeviceExtensionList.push_back(curExtStr);
    }

    delete[] pExtensionStr;
  }
}