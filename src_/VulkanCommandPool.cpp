#pragma once

#include "./VulkanInstance.cpp"
#include "./VulkanExtensions.cpp"
#include "./VulkanDevice.cpp"
#include "./VulkanSwapchain.cpp"
#include "./OpenVRInstance.cpp"
#include "./SDLWindow.cpp"

class VulkanCommandPool
{
public:
  VulkanDevice *vulkanDevice;
  VkCommandPool m_pCommandPool;
  VkDescriptorPool m_pDescriptorPool;

  struct VulkanCommandBuffer_t
  {
    VkCommandBuffer m_pCommandBuffer;
    VkFence m_pFence;
  };
  std::deque<VulkanCommandBuffer_t> m_commandBuffers;
  VulkanCommandBuffer_t m_currentCommandBuffer;
  VulkanCommandPool(VulkanDevice *vulkanDevice)
  {
    this->vulkanDevice = vulkanDevice;
    VkResult nResult;

    // Create the command pool
    {
      VkCommandPoolCreateInfo commandPoolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
      commandPoolCreateInfo.queueFamilyIndex = vulkanDevice->m_nQueueFamilyIndex;
      commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
      nResult = vkCreateCommandPool(vulkanDevice->m_pDevice, &commandPoolCreateInfo, nullptr, &m_pCommandPool);
      if (nResult != VK_SUCCESS)
      {

        char buf[1024];
        sprintf_s(buf, sizeof(buf), "vkCreateCommandPool returned error %d", nResult);
        throw std::runtime_error(buf);
      }
    }

    // Command buffer used during resource loading
    m_currentCommandBuffer = GetCommandBuffer();
    VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(m_currentCommandBuffer.m_pCommandBuffer, &commandBufferBeginInfo);

    // SetupTexturemaps();
    // SetupScene();
    // SetupCameras();
    // SetupStereoRenderTargets();
    // SetupCompanionWindow();

    // if (!CreateAllShaders())
    //   return false;

    // CreateAllDescriptorSets();
    // SetupRenderModels();

    // Submit the command buffer used during loading
    vkEndCommandBuffer(m_currentCommandBuffer.m_pCommandBuffer);
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_currentCommandBuffer.m_pCommandBuffer;
    vkQueueSubmit(vulkanDevice->m_pQueue, 1, &submitInfo, m_currentCommandBuffer.m_pFence);
    m_commandBuffers.push_front(m_currentCommandBuffer);

    m_currentCommandBuffer.m_pCommandBuffer = VK_NULL_HANDLE;
    m_currentCommandBuffer.m_pFence = VK_NULL_HANDLE;

    // Wait for the GPU before proceeding
    vkQueueWaitIdle(vulkanDevice->m_pQueue);
  }

  VulkanCommandBuffer_t GetCommandBuffer()
  {
    VulkanCommandBuffer_t commandBuffer;
    if (m_commandBuffers.size() > 0)
    {
      // If the fence associated with the command buffer has finished, reset it and return it
      if (vkGetFenceStatus(vulkanDevice->m_pDevice, m_commandBuffers.back().m_pFence) == VK_SUCCESS)
      {
        VulkanCommandBuffer_t *pCmdBuffer = &m_commandBuffers.back();
        commandBuffer.m_pCommandBuffer = pCmdBuffer->m_pCommandBuffer;
        commandBuffer.m_pFence = pCmdBuffer->m_pFence;

        vkResetCommandBuffer(commandBuffer.m_pCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        vkResetFences(vulkanDevice->m_pDevice, 1, &commandBuffer.m_pFence);
        m_commandBuffers.pop_back();
        return commandBuffer;
      }
    }

    // Create a new command buffer and associated fence
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.commandPool = m_pCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(vulkanDevice->m_pDevice, &commandBufferAllocateInfo, &commandBuffer.m_pCommandBuffer);

    VkFenceCreateInfo fenceCreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    vkCreateFence(vulkanDevice->m_pDevice, &fenceCreateInfo, nullptr, &commandBuffer.m_pFence);
    return commandBuffer;
  }
};