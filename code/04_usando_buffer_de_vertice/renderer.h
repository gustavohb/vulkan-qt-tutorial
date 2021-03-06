#ifndef RENDERER_H
#define RENDERER_H

#include <QVulkanWindowRenderer>
#include <QVulkanDeviceFunctions>
#include <QSharedPointer>

struct Model;

struct Object3D
{
    Object3D(QSharedPointer<Model> model);

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

    QSharedPointer<Model> model;
};

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

    Object3D* m_object = nullptr;

private:
    void initPipeline();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void initObject();
    void createObjectVertexBuffer();
    void releaseObjectResources();

    static QByteArray readFile(const QString &fileName);
    VkShaderModule createShaderModule(const QByteArray &code);
};

#endif // RENDERER_H
