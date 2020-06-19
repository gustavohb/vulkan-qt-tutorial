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

    initPipeline();
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


}

void Renderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

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

    m_deviceFunctions->vkCmdDraw(commandBuffer, 3, 1, 0, 0);
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
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

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

void Renderer::releaseResources() {
    VkDevice device = m_window->device();

    m_deviceFunctions->vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
    m_deviceFunctions->vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
}
