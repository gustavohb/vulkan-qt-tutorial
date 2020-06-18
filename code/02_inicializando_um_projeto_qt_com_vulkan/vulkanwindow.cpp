#include "vulkanwindow.h"

#include "renderer.h"

VulkanWindow::VulkanWindow(QWindow *parentWindow) : QVulkanWindow(parentWindow) {
  if (!m_instance.create())
      qFatal("Failed to create Vulkan instance: %d", m_instance.errorCode());
  setVulkanInstance(&m_instance);
}

QVulkanWindowRenderer *VulkanWindow::createRenderer() {
    m_renderer = new Renderer(this);
    return m_renderer;
}
