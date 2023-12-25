#pragma once

#include "./VulkanInstance.cpp"
#include "./VulkanExtensions.cpp"
#include "./VulkanDevice.cpp"
#include "./VulkanSwapchain.cpp"
#include "./OpenVRInstance.cpp"
#include "./VulkanCommandPool.cpp"
#include "./SDLWindow.cpp"

struct FramebufferDesc
{
  VkImage m_pImage;
  VkImageLayout m_nImageLayout;
  VkDeviceMemory m_pDeviceMemory;
  VkImageView m_pImageView;
  VkImage m_pDepthStencilImage;
  VkImageLayout m_nDepthStencilImageLayout;
  VkDeviceMemory m_pDepthStencilDeviceMemory;
  VkImageView m_pDepthStencilImageView;
  VkRenderPass m_pRenderPass;
  VkFramebuffer m_pFramebuffer;
};

class VulkanRenderer
{
public:
  OpenVRInstance *vrInstance;
  VulkanCommandPool *vulkanCommandPool;
  VulkanDevice *vulkanDevice;
  VulkanSwapchain *vulkanSwapchain;
  uint32_t m_nFrameIndex;
  uint32_t m_nRenderWidth;
  uint32_t m_nRenderHeight;
  int m_nMSAASampleCount;
  float m_flSuperSampleScale;
  FramebufferDesc m_leftEyeDesc;
  FramebufferDesc m_rightEyeDesc;
  VulkanRenderer(OpenVRInstance *vrInstance, VulkanCommandPool *vulkanCommandPool, VulkanDevice *vulkanDevice, VulkanSwapchain *vulkanSwapchain)
  {
    this->vrInstance = vrInstance;
    this->vulkanCommandPool = vulkanCommandPool;
    this->vulkanDevice = vulkanDevice;
    this->vulkanSwapchain = vulkanSwapchain;
    m_nMSAASampleCount = 4;
    m_flSuperSampleScale = 1;
    vrInstance->m_pHMD->GetRecommendedRenderTargetSize(&m_nRenderWidth, &m_nRenderHeight);
    m_nRenderWidth = (uint32_t)(m_flSuperSampleScale * (float)m_nRenderWidth);
    m_nRenderHeight = (uint32_t)(m_flSuperSampleScale * (float)m_nRenderHeight);
    VkResult nResult;
    VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = m_nRenderWidth;
    imageCreateInfo.extent.height = m_nRenderHeight;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.samples = (VkSampleCountFlagBits)m_nMSAASampleCount;
    imageCreateInfo.usage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    imageCreateInfo.flags = 0;
    nResult = vkCreateImage(vulkanDevice->m_pDevice, &imageCreateInfo, nullptr, &m_leftEyeDesc.m_pImage);
    nResult = vkCreateImage(vulkanDevice->m_pDevice, &imageCreateInfo, nullptr, &m_rightEyeDesc.m_pImage);
  }

  void RenderFrame()
  {
    std::cout << "Test1\n";
    if (vrInstance->m_pHMD)
    {
      // vulkanCommandPool->m_currentCommandBuffer = vulkanCommandPool->GetCommandBuffer();

      // Start the command buffer
      VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
      commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      vkBeginCommandBuffer(vulkanCommandPool->m_currentCommandBuffer.m_pCommandBuffer, &commandBufferBeginInfo);
      std::cout << "Test2\n";

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
      vulkanData.m_pDevice = (VkDevice_T *)vulkanDevice->m_pDevice;
      vulkanData.m_pPhysicalDevice = (VkPhysicalDevice_T *)vulkanDevice->m_pPhysicalDevice;
      vulkanData.m_pInstance = (VkInstance_T *)vulkanDevice->vulkanInstance->m_pInstance;
      vulkanData.m_pQueue = (VkQueue_T *)vulkanDevice->m_pQueue;
      vulkanData.m_nQueueFamilyIndex = vulkanDevice->m_nQueueFamilyIndex;

      vulkanData.m_nWidth = m_nRenderWidth;
      vulkanData.m_nHeight = m_nRenderHeight;
      vulkanData.m_nFormat = VK_FORMAT_R8G8B8A8_SRGB;
      vulkanData.m_nSampleCount = m_nMSAASampleCount;

      vr::Texture_t texture = {&vulkanData, vr::TextureType_Vulkan, vr::ColorSpace_Auto};

      vulkanData.m_nImage = (uint64_t)m_leftEyeDesc.m_pImage;
      vr::VRCompositor()->Submit(vr::Eye_Left, &texture, &bounds);

      vulkanData.m_nImage = (uint64_t)m_rightEyeDesc.m_pImage;
      vr::VRCompositor()->Submit(vr::Eye_Right, &texture, &bounds);
    }
    std::cout << "Test3\n";

    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vulkanSwapchain->m_pSwapchain;
    presentInfo.pImageIndices = &vulkanSwapchain->m_nCurrentSwapchainImage;
    vkQueuePresentKHR(vulkanDevice->m_pQueue, &presentInfo);

    // // Spew out the controller and pose count whenever they change.
    // if (m_iTrackedControllerCount != m_iTrackedControllerCount_Last || m_iValidPoseCount != m_iValidPoseCount_Last)
    // {
    //   m_iValidPoseCount_Last = m_iValidPoseCount;
    //   m_iTrackedControllerCount_Last = m_iTrackedControllerCount;

    //   dprintf("PoseCount:%d(%s) Controllers:%d\n", m_iValidPoseCount, m_strPoseClasses.c_str(), m_iTrackedControllerCount);
    // }

    // UpdateHMDMatrixPose();

    m_nFrameIndex = (m_nFrameIndex + 1) % vulkanSwapchain->m_swapchainImages.size();
    std::cout << "Test4\n";
  }
};