#ifndef RENDERER_H
#define RENDERER_H

#include <QVulkanWindowRenderer>
#include <QVulkanDeviceFunctions>

class VulkanWindow;

class Renderer : public QVulkanWindowRenderer {
public:
    Renderer(VulkanWindow *window);

    void initResources() override;
    void releaseResources() override;
    void startNextFrame() override;

private:
    VulkanWindow *m_window = nullptr;
    QVulkanDeviceFunctions *m_deviceFunctions;
    VkPipelineLayout m_pipelineLayout = nullptr;
    VkPipeline m_graphicsPipeline = nullptr;

private:
    void initPipeline();

    static QByteArray readFile(const QString &fileName);
    VkShaderModule createShaderModule(const QByteArray &code);
};

#endif // RENDERER_H
