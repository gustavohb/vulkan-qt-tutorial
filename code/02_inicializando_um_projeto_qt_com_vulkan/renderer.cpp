#include "renderer.h"

#include "vulkanwindow.h"

Renderer::Renderer(VulkanWindow *window) : m_window(window) {

}

void Renderer::startNextFrame() {
    m_window->frameReady();
    m_window->requestUpdate();
}
