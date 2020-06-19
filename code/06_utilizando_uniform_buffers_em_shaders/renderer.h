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

    VkImage textureImage = VK_NULL_HANDLE;
    VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
    VkImageView textureImageView = VK_NULL_HANDLE;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    QSharedPointer<Model> model;
};

class VulkanWindow;

class Renderer : public QVulkanWindowRenderer {
public:
    Renderer(VulkanWindow *window);

    void initResources() override;
    void releaseResources() override;
    void startNextFrame() override;
    void addTextureImage(QString texturePath);

private:
    VulkanWindow *m_window = nullptr;
    QVulkanDeviceFunctions *m_deviceFunctions;
    VkDescriptorSetLayout m_descriptorSetLayout = nullptr;
    VkPipelineLayout m_pipelineLayout = nullptr;
    VkPipeline m_graphicsPipeline = nullptr;
    VkSampler m_textureSampler = nullptr;

    Object3D* m_object = nullptr;

private:
    void initPipeline();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createTextureSampler();
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();
    void initObject();
    void createTextureImageView();
    void createObjectVertexBuffer();
    void releaseObjectResources();

    static QByteArray readFile(const QString &fileName);
    VkShaderModule createShaderModule(const QByteArray &code);
};

#endif // RENDERER_H
