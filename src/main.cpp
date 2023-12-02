#include <iostream>
#include <openvr.h>

int main()
{
  vr::IVRSystem *vrSystem;
  vr::EVRInitError error = vr::VRInitError_None;
  vrSystem = vr::VR_Init(&error, vr::VRApplication_Scene);

  if (error != vr::VRInitError_None)
  {
    return 1;
  }

  return 0;
}