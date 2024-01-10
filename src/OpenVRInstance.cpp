#pragma once
#pragma warning(push)
#pragma warning(disable : 4996)
#pragma warning(disable : 4267)

#include <openvr.h>

#include "./RenderTarget.cpp"

class OpenVRInstance
{
private:
  vr::IVRSystem *m_pHMD;

public:
  std::string Driver;
  std::string Display;
  RenderTarget *renderTarget;
  bool isOpen;
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
    isOpen = true;
  }

  void Update()
  {
    vr::VREvent_t event;
    while (m_pHMD->PollNextEvent(&event, sizeof(event)))
    {
      switch (event.eventType)
      {
      case vr::VREvent_Quit:
        isOpen = false;
        break;
      }
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