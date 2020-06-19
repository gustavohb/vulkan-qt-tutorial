#ifndef MODEL_H
#define MODEL_H

#include <QVector2D>
#include <QVector3D>
#include <QVector>
#include <array>

struct Vertex {
    QVector2D pos;
    QVector3D color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};

        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.binding = 0;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

struct Model
{
    Model() {};

    QVector<Vertex> vertices = {
           { {0.0f, -0.5f}, {1.0f, 0.0f, 0.0f} },
           { {0.5f,  0.5f}, {0.0f, 1.0f, 0.0f} },
           { {-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} }
    };
};

#endif // MODEL_H
