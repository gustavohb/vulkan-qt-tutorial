#include "renderer.h"

#include "vulkanwindow.h"

Renderer::Renderer(VulkanWindow *window) : m_window(window) {

}

void Renderer::initResources() {
    VkDevice device = m_window->device();
    m_deviceFunctions = m_window->vulkanInstance()->deviceFunctions(device);
}

void Renderer::startNextFrame() {
    m_window->frameReady();
    m_window->requestUpdate();
}


