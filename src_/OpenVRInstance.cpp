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

class OpenVRInstance
{
public:
  std::string Driver;
  std::string Display;
  vr::IVRSystem *m_pHMD;
  OpenVRInstance()
  {
    vr::EVRInitError eError = vr::VRInitError_None;
    m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
    if (eError != vr::VRInitError_None)
    {
      m_pHMD = NULL;
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "VR_Init - Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
      throw std::runtime_error(buf);
    }
    Driver = GetTrackedDeviceString(m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
    Display = GetTrackedDeviceString(m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

    if (!vr::VRCompositor())
    {
      throw std::runtime_error("VRCompositor - Compositor initialization failed. See log file for details\n");
    }
  }

  std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
  {
    uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
    if (unRequiredBufferLen == 0)
      return "";

    char *pchBuffer = new char[unRequiredBufferLen];
    unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
    std::string sResult = pchBuffer;
    delete[] pchBuffer;
    return sResult;
  }
};