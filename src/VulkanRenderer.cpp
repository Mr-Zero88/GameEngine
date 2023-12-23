#pragma once

#include "./VulkanInstance.cpp"
#include "./VulkanExtensions.cpp"
#include "./VulkanDevice.cpp"
#include "./VulkanSwapchain.cpp"
#include "./OpenVRInstance.cpp"
#include "./VulkanCommandPool.cpp"
#include "./SDLWindow.cpp"

class VulkanRenderer
{
public:
  OpenVRInstance *vrInstance;
  VulkanCommandPool *vulkanCommandPool;
  VulkanDevice *vulkanDevice;
  VulkanSwapchain *vulkanSwapchain;
  uint32_t m_nFrameIndex;
  VulkanRenderer(OpenVRInstance *vrInstance, VulkanCommandPool *vulkanCommandPool, VulkanDevice *vulkanDevice, VulkanSwapchain *vulkanSwapchain)
  {
    this->vrInstance = vrInstance;
    this->vulkanCommandPool = vulkanCommandPool;
    this->vulkanDevice = vulkanDevice;
    this->vulkanSwapchain = vulkanSwapchain;
  }

  void RenderFrame()
  {
    if (vrInstance->m_pHMD)
    {
      // vulkanCommandPool->m_currentCommandBuffer = vulkanCommandPool->GetCommandBuffer();

      // Start the command buffer
      VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
      commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      vkBeginCommandBuffer(vulkanCommandPool->m_currentCommandBuffer.m_pCommandBuffer, &commandBufferBeginInfo);

      // UpdateControllerAxes();
      // RenderStereoTargets();
      // RenderCompanionWindow();

      // End the command buffer
      vkEndCommandBuffer(vulkanCommandPool->m_currentCommandBuffer.m_pCommandBuffer);

      // Submit the command buffer
      VkPipelineStageFlags nWaitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &vulkanCommandPool->m_currentCommandBuffer.m_pCommandBuffer;
      submitInfo.waitSemaphoreCount = 1;
      submitInfo.pWaitSemaphores = &vulkanSwapchain->m_pSwapchainSemaphores[m_nFrameIndex];
      submitInfo.pWaitDstStageMask = &nWaitDstStageMask;
      vkQueueSubmit(vulkanDevice->m_pQueue, 1, &submitInfo, vulkanCommandPool->m_currentCommandBuffer.m_pFence);

      // Add the command buffer back for later recycling
      vulkanCommandPool->m_commandBuffers.push_front(vulkanCommandPool->m_currentCommandBuffer);

      vulkanCommandPool->m_currentCommandBuffer.m_pCommandBuffer = VK_NULL_HANDLE;
      vulkanCommandPool->m_currentCommandBuffer.m_pFence = VK_NULL_HANDLE;

      // Submit to SteamVR
      vr::VRTextureBounds_t bounds;
      bounds.uMin = 0.0f;
      bounds.uMax = 1.0f;
      bounds.vMin = 0.0f;
      bounds.vMax = 1.0f;

      vr::VRVulkanTextureData_t vulkanData;
      vulkanData.m_nImage = (uint64_t)m_leftEyeDesc.m_pImage;
      vulkanData.m_pDevice = (VkDevice_T *)m_pDevice;
      vulkanData.m_pPhysicalDevice = (VkPhysicalDevice_T *)m_pPhysicalDevice;
      vulkanData.m_pInstance = (VkInstance_T *)m_pInstance;
      vulkanData.m_pQueue = (VkQueue_T *)m_pQueue;
      vulkanData.m_nQueueFamilyIndex = m_nQueueFamilyIndex;

      vulkanData.m_nWidth = m_nRenderWidth;
      vulkanData.m_nHeight = m_nRenderHeight;
      vulkanData.m_nFormat = VK_FORMAT_R8G8B8A8_SRGB;
      vulkanData.m_nSampleCount = m_nMSAASampleCount;

      vr::Texture_t texture = {&vulkanData, vr::TextureType_Vulkan, vr::ColorSpace_Auto};
      vr::VRCompositor()->Submit(vr::Eye_Left, &texture, &bounds);

      vulkanData.m_nImage = (uint64_t)m_rightEyeDesc.m_pImage;
      vr::VRCompositor()->Submit(vr::Eye_Right, &texture, &bounds);
    }

    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_pSwapchain;
    presentInfo.pImageIndices = &m_nCurrentSwapchainImage;
    vkQueuePresentKHR(m_pQueue, &presentInfo);

    // Spew out the controller and pose count whenever they change.
    if (m_iTrackedControllerCount != m_iTrackedControllerCount_Last || m_iValidPoseCount != m_iValidPoseCount_Last)
    {
      m_iValidPoseCount_Last = m_iValidPoseCount;
      m_iTrackedControllerCount_Last = m_iTrackedControllerCount;

      dprintf("PoseCount:%d(%s) Controllers:%d\n", m_iValidPoseCount, m_strPoseClasses.c_str(), m_iTrackedControllerCount);
    }

    UpdateHMDMatrixPose();

    m_nFrameIndex = (m_nFrameIndex + 1) % m_swapchainImages.size();
  }
}