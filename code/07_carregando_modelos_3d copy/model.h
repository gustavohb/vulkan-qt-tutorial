#ifndef MODEL_H
#define MODEL_H

#include <QVulkanFunctions>

#include <QVector2D>
#include <QVector3D>
#include <QVector>
#include <QMatrix4x4>
#include <array>

struct Vertex {
    QVector3D pos;
    QVector3D color;
    QVector2D texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};

        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.binding = 0;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

struct Model
{
    Model() {};

    bool isValid() const { return vertices.size(); }
    void readOBJFile(QString const &filePath);

    //QVector<Vertex> vertices;

    QVector<Vertex> vertices;
    /*
    QVector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}
    };
*/


    QMatrix4x4 transformation;

    /*
    QVector<Vertex> vertices = {
        { {-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
        { { 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
        { { 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },
        { { 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },
        { {-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { {-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} }
    };
    */
};

#endif // MODEL_H
