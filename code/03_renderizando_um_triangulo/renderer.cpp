#include "renderer.h"

#include <QFile>
#include <QVulkanFunctions>

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

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
}

QByteArray Renderer::readFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        QDebug(QtFatalMsg) << QLatin1String("Failed to open file:") << fileName;
    QByteArray content = file.readAll();
    file.close();

    return content;
}

VkShaderModule Renderer::createShaderModule(const QByteArray &code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size_t(code.size());
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.constData());

    VkShaderModule shaderModule;
    VkDevice device = m_window->device();
    VkResult result = m_deviceFunctions->vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS)
        QDebug(QtFatalMsg) << QLatin1String("Failed to create shader module:") << result;

    return shaderModule;
}
