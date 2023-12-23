#pragma once

#include "./VulkanInstance.cpp"
#include "./VulkanExtensions.cpp"
#include "./VulkanDevice.cpp"
#include "./OpenVRInstance.cpp"
#include "./SDLWindow.cpp"

class VulkanSwapchain
{
public:
  VkSurfaceKHR m_pSurface;
  uint32_t m_nSwapQueueImageCount;
  VkSwapchainKHR m_pSwapchain;
  std::vector<VkImage> m_swapchainImages;
  std::vector<VkImageView> m_pSwapchainImageViews;
  std::vector<VkFramebuffer> m_pSwapchainFramebuffers;
  std::vector<VkSemaphore> m_pSwapchainSemaphores;
  VkRenderPass m_pSwapchainRenderPass;
  VulkanSwapchain(VulkanDevice *vulkanDevice, SDLWindow *window)
  {
    VkResult nResult;

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window->m_pCompanionWindow, &wmInfo);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo = {};
    win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32SurfaceCreateInfo.pNext = NULL;
    win32SurfaceCreateInfo.flags = 0;
    win32SurfaceCreateInfo.hinstance = GetModuleHandle(NULL);
    win32SurfaceCreateInfo.hwnd = (HWND)wmInfo.info.win.window;
    nResult = vkCreateWin32SurfaceKHR(vulkanDevice->vulkanInstance->m_pInstance, &win32SurfaceCreateInfo, nullptr, &m_pSurface);
#else
    VkXlibSurfaceCreateInfoKHR xlibSurfaceCreateInfo = {VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR};
    xlibSurfaceCreateInfo.flags = 0;
    xlibSurfaceCreateInfo.dpy = wmInfo.info.x11.display;
    xlibSurfaceCreateInfo.window = wmInfo.info.x11.window;
    nResult = vkCreateXlibSurfaceKHR(m_pInstance, &xlibSurfaceCreateInfo, nullptr, &m_pSurface);
#endif
    if (nResult != VK_SUCCESS)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "Failed to create VkSurfaceKHR error %d.\n", nResult);
      throw std::runtime_error(buf);
    }

    VkBool32 bSupportsPresent = VK_FALSE;
    nResult = vkGetPhysicalDeviceSurfaceSupportKHR(vulkanDevice->m_pPhysicalDevice, vulkanDevice->m_nQueueFamilyIndex, m_pSurface, &bSupportsPresent);
    if (nResult != VK_SUCCESS || bSupportsPresent == VK_FALSE)
    {
      throw std::runtime_error("vkGetPhysicalDeviceSurfaceSupportKHR present not supported.\n");
    }

    // Query supported swapchain formats
    VkFormat nSwapChainFormat;
    uint32_t nFormatIndex = 0;
    uint32_t nNumSupportedSwapChainFormats = 0;
    VkColorSpaceKHR nColorSpace;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanDevice->m_pPhysicalDevice, m_pSurface, &nNumSupportedSwapChainFormats, NULL) != VK_SUCCESS)
    {
      throw std::runtime_error("Unable to query size of supported swapchain formats.\n");
    }
    VkSurfaceFormatKHR *pSupportedSurfaceFormats = new VkSurfaceFormatKHR[nNumSupportedSwapChainFormats];
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanDevice->m_pPhysicalDevice, m_pSurface, &nNumSupportedSwapChainFormats, pSupportedSurfaceFormats) != VK_SUCCESS)
    {
      throw std::runtime_error("Unable to query supported swapchain formats.\n");
    }
    if (nNumSupportedSwapChainFormats == 1 && pSupportedSurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
      nSwapChainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else
    {
      // Favor sRGB if it's available
      for (nFormatIndex = 0; nFormatIndex < nNumSupportedSwapChainFormats; nFormatIndex++)
      {
        if (pSupportedSurfaceFormats[nFormatIndex].format == VK_FORMAT_B8G8R8A8_SRGB ||
            pSupportedSurfaceFormats[nFormatIndex].format == VK_FORMAT_R8G8B8A8_SRGB)
        {
          break;
        }
      }
      if (nFormatIndex == nNumSupportedSwapChainFormats)
      {
        // Default to the first one if no sRGB
        nFormatIndex = 0;
      }
      nSwapChainFormat = pSupportedSurfaceFormats[nFormatIndex].format;
    }
    nColorSpace = pSupportedSurfaceFormats[nFormatIndex].colorSpace;
    delete[] pSupportedSurfaceFormats;

    // Check the surface properties and formats
    VkSurfaceCapabilitiesKHR surfaceCaps = {};
    nResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanDevice->m_pPhysicalDevice, m_pSurface, &surfaceCaps);
    if (nResult != VK_SUCCESS)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed with error %d\n", nResult);
      throw std::runtime_error(buf);
    }

    uint32_t nPresentModeCount = 0;
    nResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanDevice->m_pPhysicalDevice, m_pSurface, &nPresentModeCount, NULL);
    if (nResult != VK_SUCCESS)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "vkGetPhysicalDeviceSurfacePresentModesKHR failed with error %d\n", nResult);
      throw std::runtime_error(buf);
    }
    VkPresentModeKHR *pPresentModes = new VkPresentModeKHR[nPresentModeCount];
    nResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanDevice->m_pPhysicalDevice, m_pSurface, &nPresentModeCount, pPresentModes);
    if (nResult != VK_SUCCESS)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "vkGetPhysicalDeviceSurfacePresentModesKHR failed with error %d\n", nResult);
      throw std::runtime_error(buf);
    }

    // width and height are either both -1, or both not -1.
    VkExtent2D swapChainExtent;
    if (surfaceCaps.currentExtent.width == -1)
    {
      // If the surface size is undefined, the size is set to the size of the images requested.
      swapChainExtent.width = window->width;
      swapChainExtent.height = window->height;
    }
    else
    {
      // If the surface size is defined, the swap chain size must match
      swapChainExtent = surfaceCaps.currentExtent;
    }

    // VK_PRESENT_MODE_FIFO_KHR - equivalent of eglSwapInterval(1).  The presentation engine waits for the next vertical blanking period to update
    // the current image. Tearing cannot be observed. This mode must be supported by all implementations.
    VkPresentModeKHR swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < nPresentModeCount; i++)
    {
      // Order of preference for no vsync:
      // 1. VK_PRESENT_MODE_IMMEDIATE_KHR - The presentation engine does not wait for a vertical blanking period to update the current image,
      //                                    meaning this mode may result in visible tearing
      // 2. VK_PRESENT_MODE_MAILBOX_KHR - The presentation engine waits for the next vertical blanking period to update the current image. Tearing cannot be observed.
      //                                  An internal single-entry queue is used to hold pending presentation requests.
      // 3. VK_PRESENT_MODE_FIFO_RELAXED_KHR - equivalent of eglSwapInterval(-1).
      if (pPresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
      {
        // The presentation engine does not wait for a vertical blanking period to update the
        // current image, meaning this mode may result in visible tearing
        swapChainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        break;
      }
      else if (pPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
      {
        swapChainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
      }
      else if ((swapChainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) &&
               (pPresentModes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR))
      {
        // VK_PRESENT_MODE_FIFO_RELAXED_KHR - equivalent of eglSwapInterval(-1)
        swapChainPresentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
      }
    }

    // Have a swap queue depth of at least three frames
    m_nSwapQueueImageCount = surfaceCaps.minImageCount;
    if (m_nSwapQueueImageCount < 2)
    {
      m_nSwapQueueImageCount = 2;
    }
    if ((surfaceCaps.maxImageCount > 0) && (m_nSwapQueueImageCount > surfaceCaps.maxImageCount))
    {
      // Application must settle for fewer images than desired:
      m_nSwapQueueImageCount = surfaceCaps.maxImageCount;
    }

    VkSurfaceTransformFlagsKHR preTransform;
    if (surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
      preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
      preTransform = surfaceCaps.currentTransform;
    }

    VkImageUsageFlags nImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if ((surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
    {
      nImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    else
    {
      printf("Vulkan swapchain does not support VK_IMAGE_USAGE_TRANSFER_DST_BIT. Some operations may not be supported.\n");
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.pNext = NULL;
    swapChainCreateInfo.surface = m_pSurface;
    swapChainCreateInfo.minImageCount = m_nSwapQueueImageCount;
    swapChainCreateInfo.imageFormat = nSwapChainFormat;
    swapChainCreateInfo.imageColorSpace = nColorSpace;
    swapChainCreateInfo.imageExtent = swapChainExtent;
    swapChainCreateInfo.imageUsage = nImageUsageFlags;
    swapChainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.queueFamilyIndexCount = 0;
    swapChainCreateInfo.pQueueFamilyIndices = NULL;
    swapChainCreateInfo.presentMode = swapChainPresentMode;
    swapChainCreateInfo.clipped = VK_TRUE;
    if ((surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) != 0)
    {
      swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }
    else if ((surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) != 0)
    {
      swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }
    else
    {
      printf("Unexpected value for VkSurfaceCapabilitiesKHR.compositeAlpha: %x\n", surfaceCaps.supportedCompositeAlpha);
    }

    nResult = vkCreateSwapchainKHR(vulkanDevice->m_pDevice, &swapChainCreateInfo, NULL, &m_pSwapchain);
    if (nResult != VK_SUCCESS)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "vkCreateSwapchainKHR returned an error %d.\n", nResult);
      throw std::runtime_error(buf);
    }

    nResult = vkGetSwapchainImagesKHR(vulkanDevice->m_pDevice, m_pSwapchain, &m_nSwapQueueImageCount, NULL);
    if (nResult != VK_SUCCESS)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "vkGetSwapchainImagesKHR failed with error %d\n", nResult);
      throw std::runtime_error(buf);
    }
    m_swapchainImages.resize(m_nSwapQueueImageCount);
    vkGetSwapchainImagesKHR(vulkanDevice->m_pDevice, m_pSwapchain, &m_nSwapQueueImageCount, &m_swapchainImages[0]);
    if (nResult != VK_SUCCESS)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "vkGetSwapchainImagesKHR failed with error %d\n", nResult);
      throw std::runtime_error(buf);
    }

    // Create a renderpass
    uint32_t nTotalAttachments = 1;
    VkAttachmentDescription attachmentDesc;
    VkAttachmentReference attachmentReference;
    attachmentReference.attachment = 0;
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachmentDesc.format = nSwapChainFormat;
    attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachmentDesc.flags = 0;

    VkSubpassDescription subPassCreateInfo = {};
    subPassCreateInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPassCreateInfo.flags = 0;
    subPassCreateInfo.inputAttachmentCount = 0;
    subPassCreateInfo.pInputAttachments = NULL;
    subPassCreateInfo.colorAttachmentCount = 1;
    subPassCreateInfo.pColorAttachments = &attachmentReference;
    subPassCreateInfo.pResolveAttachments = NULL;
    subPassCreateInfo.pDepthStencilAttachment = NULL;
    subPassCreateInfo.preserveAttachmentCount = 0;
    subPassCreateInfo.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDesc;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subPassCreateInfo;
    renderPassCreateInfo.dependencyCount = 0;
    renderPassCreateInfo.pDependencies = NULL;

    nResult = vkCreateRenderPass(vulkanDevice->m_pDevice, &renderPassCreateInfo, NULL, &m_pSwapchainRenderPass);
    if (nResult != VK_SUCCESS)
    {
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "vkCreateRenderPass failed with error %d\n", nResult);
      throw std::runtime_error(buf);
    }

    // Create image views and framebuffers for each swapchain image
    for (size_t nImage = 0; nImage < m_swapchainImages.size(); nImage++)
    {
      VkImageViewCreateInfo imageViewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
      imageViewCreateInfo.flags = 0;
      imageViewCreateInfo.image = m_swapchainImages[nImage];
      imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      imageViewCreateInfo.format = nSwapChainFormat;
      imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
      imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
      imageViewCreateInfo.subresourceRange.levelCount = 1;
      imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
      imageViewCreateInfo.subresourceRange.layerCount = 1;
      VkImageView pImageView = VK_NULL_HANDLE;
      vkCreateImageView(vulkanDevice->m_pDevice, &imageViewCreateInfo, nullptr, &pImageView);
      m_pSwapchainImageViews.push_back(pImageView);

      VkImageView attachments[1] = {pImageView};
      VkFramebufferCreateInfo framebufferCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
      framebufferCreateInfo.renderPass = m_pSwapchainRenderPass;
      framebufferCreateInfo.attachmentCount = 1;
      framebufferCreateInfo.pAttachments = &attachments[0];
      framebufferCreateInfo.width = window->width;
      framebufferCreateInfo.height = window->height;
      framebufferCreateInfo.layers = 1;
      VkFramebuffer pFramebuffer;
      nResult = vkCreateFramebuffer(vulkanDevice->m_pDevice, &framebufferCreateInfo, NULL, &pFramebuffer);
      if (nResult != VK_SUCCESS)
      {
        char buf[1024];
        sprintf_s(buf, sizeof(buf), "vkCreateFramebuffer failed with error %d\n", nResult);
        throw std::runtime_error(buf);
      }
      m_pSwapchainFramebuffers.push_back(pFramebuffer);

      VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
      VkSemaphore pSemaphore = VK_NULL_HANDLE;
      vkCreateSemaphore(vulkanDevice->m_pDevice, &semaphoreCreateInfo, nullptr, &pSemaphore);
      m_pSwapchainSemaphores.push_back(pSemaphore);
    }

    delete[] pPresentModes;
  }
};