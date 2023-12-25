#pragma once

#include "./VulkanInstance.cpp"
#include "./VulkanRenderer.cpp"

enum RendererType
{
  Vulakn = 0
};

class Renderer
{
private:
  RendererType rendererType;
  VulkanRenderer *vulkanRenderer;

public:
  Renderer(VulkanInstance *vulkanInstance)
  {
    rendererType = RendererType::Vulakn;
    vulkanRenderer = new VulkanRenderer(vulkanInstance);
  }
  void Render(RenderTarget *renderTarget)
  {
    switch (rendererType)
    {
    case RendererType::Vulakn:
      vulkanRenderer->Render(renderTarget);
      break;
    }
  }
};