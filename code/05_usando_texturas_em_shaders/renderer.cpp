#include "renderer.h"

#include <QFile>
#include <QVulkanFunctions>
#include <array>

#include "vulkanwindow.h"

#include "model.h"

Object3D::Object3D(QSharedPointer<Model> model)
    : model(model) {}

Renderer::Renderer(VulkanWindow *window) : m_window(window) {

}

void Renderer::initResources() {
    VkDevice device = m_window->device();
    m_deviceFunctions = m_window->vulkanInstance()->deviceFunctions(device);

    createDescriptorSetLayout();
    initPipeline();
    createTextureSampler();
    initObject();
}

void Renderer::createBuffer(VkDeviceSize size,
                            VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags properties,
                            VkBuffer& buffer,
                            VkDeviceMemory& bufferMemory) {

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0;
    bufferInfo.pQueueFamilyIndices = nullptr;

    VkDevice device = m_window->device();
    VkResult result = m_deviceFunctions->vkCreateBuffer(
            device,
            &bufferInfo,
            nullptr,
            &buffer
        );
    if (result != VK_SUCCESS) {
        qFatal("Failed to create vertex buffer: %d", result);
    }

    VkMemoryRequirements memRequirements;
    m_deviceFunctions->vkGetBufferMemoryRequirements(
            device,
            buffer,
            &memRequirements
        );

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    result = m_deviceFunctions->vkAllocateMemory(
            device,
            &allocInfo,
            nullptr,
            &bufferMemory
        );
    if ( result != VK_SUCCESS) {
        qFatal("Failed to allocate vertex buffer memory!");
    }

    m_deviceFunctions->vkBindBufferMemory(
            device,
            buffer,
            bufferMemory,
            0
        );

}

uint32_t Renderer::findMemoryType(
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties) {

    VkPhysicalDeviceMemoryProperties memProperties;
    QVulkanInstance *inst = m_window->vulkanInstance();
    QVulkanFunctions *f = inst->functions();

    f->vkGetPhysicalDeviceMemoryProperties(
            m_window->physicalDevice(),
            &memProperties
        );

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
         if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    qFatal("Failed to find suitable memory type!");

}

void Renderer::initObject() {
    QSharedPointer<Model> model = QSharedPointer<Model>::create(Model());
    m_object = new Object3D(model);

    createObjectVertexBuffer();

    addTextureImage(":/textures/texture.png");
}

void Renderer::createTextureImageView() {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;
    viewInfo.flags = 0;
    viewInfo.image = m_object->textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkDevice device = m_window->device();
    if (m_object->textureImageView) {
        m_deviceFunctions->vkDestroyImageView(
            device,
            m_object->textureImageView,
            nullptr
        );
    }

    VkResult result = m_deviceFunctions->vkCreateImageView(
        device,
        &viewInfo,
        nullptr,
        &m_object->textureImageView
    );
    if (result != VK_SUCCESS) {
        qFatal("Failed to create texture image view: %d", result);
    }
}


void Renderer::createObjectVertexBuffer() {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize bufferSize = sizeof(m_object->model->vertices[0]) * m_object->model->vertices.size();

    createBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void* data;
    VkDevice device = m_window->device();
    m_deviceFunctions->vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_object->model->vertices.data(), (size_t) bufferSize);
    m_deviceFunctions->vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_object->vertexBuffer,
        m_object->vertexBufferMemory
    );

    copyBuffer(stagingBuffer, m_object->vertexBuffer, bufferSize);

    m_deviceFunctions->vkDestroyBuffer(
        device,
        stagingBuffer,
        nullptr
    );
    m_deviceFunctions->vkFreeMemory(
        device,
        stagingBufferMemory,
        nullptr
    );
}

void Renderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    m_deviceFunctions->vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void Renderer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerLayoutBinding;

    VkDevice device = m_window->device();
    VkResult result = m_deviceFunctions->vkCreateDescriptorSetLayout(
        device,
        &layoutInfo,
        nullptr,
        &m_descriptorSetLayout
    );
    if (result != VK_SUCCESS) {
        qFatal("Failed to create descriptor set layout: %d", result);
    }
}

void Renderer::createDescriptorPool() {
    VkDescriptorPoolSize poolSize = {};
    poolSize.type =  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    VkDevice device = m_window->device();

    if (m_object->descriptorPool) {
        m_deviceFunctions->vkDestroyDescriptorPool(
            device,
            m_object->descriptorPool,
            nullptr
        );
        m_object->descriptorPool = VK_NULL_HANDLE;
    }

    VkResult result = m_deviceFunctions->vkCreateDescriptorPool(
        device,
        &poolInfo,
        nullptr,
        &m_object->descriptorPool
    );
    if (result != VK_SUCCESS) {
        qFatal("Failed to create descriptor pool: %d", result);
    }
}

VkCommandBuffer Renderer::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_window->graphicsCommandPool();
    allocInfo.commandBufferCount = 1;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer commandBuffer;
    VkDevice device = m_window->device();
    m_deviceFunctions->vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    m_deviceFunctions->vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    m_deviceFunctions->vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkQueue graphicsQueue = m_window->graphicsQueue();
    m_deviceFunctions->vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    m_deviceFunctions->vkQueueWaitIdle(graphicsQueue);

    VkDevice device = m_window->device();
    VkCommandPool commandPool = m_window->graphicsCommandPool();
    m_deviceFunctions->vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void Renderer::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
        && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        qFatal("Unsupported layout transition!");
    }

    m_deviceFunctions->vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,
        destinationStage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier
    );

    endSingleTimeCommands(commandBuffer);
}

void Renderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    m_deviceFunctions->vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(commandBuffer);
}

void Renderer::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = 0;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    VkDevice device = m_window->device();
    VkResult result = m_deviceFunctions->vkCreateSampler(
        device,
        &samplerInfo,
        nullptr,
        &m_textureSampler
    );
    if (result != VK_SUCCESS) {
        qFatal("Failed to create texture sampler: %d", result);
    }
}

void Renderer::addTextureImage(QString texturePath) {
    QImage image(texturePath);

    if (image.isNull()) {
        qFatal("Failed to load texture image!");
    }

    image = image.convertToFormat(QImage::Format_RGBA8888);
    VkDeviceSize imageSize = image.sizeInBytes();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );

    void *data;
    VkDevice device = m_window->device();
    m_deviceFunctions->vkMapMemory(
        device,
        stagingBufferMemory,
        0,
        imageSize,
        0,
        &data
    );
    memcpy(data, image.constBits(), static_cast<size_t>(imageSize));
    m_deviceFunctions->vkUnmapMemory(device, stagingBufferMemory);

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.extent.width = image.width();
    imageInfo.extent.height = image.height();
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (m_object->textureImage) {
       m_deviceFunctions->vkDestroyImage(device, m_object->textureImage, nullptr);
        m_object->textureImage = VK_NULL_HANDLE;
    }

    VkResult result = m_deviceFunctions->vkCreateImage(device, &imageInfo, nullptr, &m_object->textureImage);
    if (result != VK_SUCCESS) {
       qFatal("Failed to create image: %d", result);
    }

    VkMemoryRequirements memRequirements;
    m_deviceFunctions->vkGetImageMemoryRequirements(
        device,
        m_object->textureImage,
        &memRequirements
    );

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    if (m_object->textureImageMemory != VK_NULL_HANDLE) {
        m_deviceFunctions->vkFreeMemory(
            device,
            m_object->textureImageMemory,
            nullptr
        );
        m_object->textureImageMemory = VK_NULL_HANDLE;
    }

    result = m_deviceFunctions->vkAllocateMemory(
        device,
        &allocInfo,
        nullptr,
        &m_object->textureImageMemory
    );
    if (result != VK_SUCCESS) {
        qFatal("Failed to allocate image memory: %d", result);
    }

    m_deviceFunctions->vkBindImageMemory(
        device,
        m_object->textureImage,
        m_object->textureImageMemory,
        0
    );

    transitionImageLayout(
        m_object->textureImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    copyBufferToImage(
        stagingBuffer, m_object->textureImage,
        static_cast<uint32_t>(image.width()),
        static_cast<uint32_t>(image.height())
    );

    transitionImageLayout(
        m_object->textureImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    m_deviceFunctions->vkDestroyBuffer(
        device, stagingBuffer,
        nullptr
    );
    m_deviceFunctions->vkFreeMemory(
        device,
        stagingBufferMemory,
        nullptr
    );

    createTextureImageView();
    createDescriptorPool();
}

void Renderer::startNextFrame() {
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_window->defaultRenderPass();
    renderPassInfo.framebuffer = m_window->currentFramebuffer();
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    const QSize swapChainImageSize = m_window->swapChainImageSize();
    renderPassInfo.renderArea.extent.width = swapChainImageSize.width();
    renderPassInfo.renderArea.extent.height = swapChainImageSize.height();

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    VkCommandBuffer commandBuffer = m_window->currentCommandBuffer();
    m_deviceFunctions->vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = swapChainImageSize.width();
    viewport.height = swapChainImageSize.height();
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    m_deviceFunctions->vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = viewport.width;
    scissor.extent.height = viewport.height;
    m_deviceFunctions->vkCmdSetScissor(
        commandBuffer,
        0,
        1,
        &scissor
    );

    m_deviceFunctions->vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_graphicsPipeline
    );

    VkBuffer vertexBuffers[] = {m_object->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    m_deviceFunctions->vkCmdBindVertexBuffers(
        commandBuffer,
        0,
        1,
        vertexBuffers,
        offsets
    );

    m_deviceFunctions->vkCmdDraw(
        commandBuffer,
        static_cast<uint32_t>(m_object->model->vertices.size()),
        1,
        0,
        0
    );
    m_deviceFunctions->vkCmdEndRenderPass(commandBuffer);

    m_window->frameReady();
    m_window->requestUpdate();
}

void Renderer::initPipeline() {
    VkDevice device = m_window->device();
    QByteArray vertShaderCode = readFile(":shaders/shader.vert.spv");
    QByteArray fragShaderCode = readFile(":shaders/shader.frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicInfo = {};
    dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicInfo.dynamicStateCount = 2;
    dynamicInfo.pDynamicStates = dynamicStates;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.depthClampEnable = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationInfo.depthBiasEnable = VK_FALSE;
    rasterizationInfo.depthBiasConstantFactor = 0.0f;
    rasterizationInfo.depthBiasClamp = 0.0f;
    rasterizationInfo.depthBiasSlopeFactor = 0.0f;
    rasterizationInfo.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

    VkResult result = m_deviceFunctions->vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    if (result != VK_SUCCESS)
        qFatal("Failed to create pipeline layout: %d", result);

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pDynamicState = &dynamicInfo;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_window->defaultRenderPass();
    pipelineInfo.subpass = 0;

    result = m_deviceFunctions->vkCreateGraphicsPipelines(
            device,
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &m_graphicsPipeline
        );

    if (result != VK_SUCCESS)
        qFatal("Failed to graphics pipeline: %d", result);

    m_deviceFunctions->vkDestroyShaderModule(device, fragShaderModule, nullptr);
    m_deviceFunctions->vkDestroyShaderModule(device, vertShaderModule, nullptr);
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

void Renderer::releaseObjectResources() {
    VkDevice device = m_window->device();

    if (m_object->vertexBuffer) {
        m_deviceFunctions->vkDestroyBuffer(
            device,
            m_object->vertexBuffer,
            nullptr
        );
        m_object->vertexBuffer = VK_NULL_HANDLE;
    }

    if(m_object->vertexBufferMemory) {
        m_deviceFunctions->vkFreeMemory(
            device,
            m_object->vertexBufferMemory,
            nullptr
        );
        m_object->vertexBufferMemory = VK_NULL_HANDLE;
    }

    if (m_object->textureImageView) {
        m_deviceFunctions->vkDestroyImageView(
            device,
            m_object->textureImageView,
            nullptr
        );
    }

    if (m_object->textureImage) {
        m_deviceFunctions->vkDestroyImage(
            device, m_object->textureImage,
            nullptr
        );
        m_object->textureImage = VK_NULL_HANDLE;
    }

    if (m_object->textureImageMemory) {
        m_deviceFunctions->vkFreeMemory(
            device,
            m_object->textureImageMemory,
            nullptr
        );
        m_object->textureImageMemory = VK_NULL_HANDLE;
    }

    if (m_object->descriptorPool) {
        m_deviceFunctions->vkDestroyDescriptorPool(
            device,
            m_object->descriptorPool,
            nullptr
        );
        m_object->descriptorPool = VK_NULL_HANDLE;
    }
}

void Renderer::releaseResources() {
    VkDevice device = m_window->device();

    m_deviceFunctions->vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
    m_deviceFunctions->vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);

    m_deviceFunctions->vkDestroySampler(
            device,
            m_textureSampler,
            nullptr
        );

    m_deviceFunctions->vkDestroyDescriptorSetLayout(
            device,
            m_descriptorSetLayout,
            nullptr
        );
}
