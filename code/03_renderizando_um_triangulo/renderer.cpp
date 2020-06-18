#include "renderer.h"

#include <QFile>

#include "vulkanwindow.h"

Renderer::Renderer(VulkanWindow *window) : m_window(window) {

}

void Renderer::initResources() {
    VkDevice device = m_window->device();
    m_deviceFunctions = m_window->vulkanInstance()->deviceFunctions(device);

    initPipeline();
}

void Renderer::startNextFrame() {
    m_window->frameReady();
    m_window->requestUpdate();
}

void Renderer::initPipeline() {
    QByteArray vertShaderCode = readFile(":shaders/shader.vert.spv");
    QByteArray fragShaderCode = readFile(":shaders/shader.frag.spv");
}

QByteArray Renderer::readFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        QDebug(QtFatalMsg) << QLatin1String("Failed to open file:") << fileName;
    QByteArray content = file.readAll();
    file.close();

    return content;
}
