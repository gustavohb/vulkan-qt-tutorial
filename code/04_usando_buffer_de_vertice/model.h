#ifndef MODEL_H
#define MODEL_H

#include <QVector2D>
#include <QVector3D>
#include <QVector>

struct Vertex {
    QVector2D pos;
    QVector3D color;
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
