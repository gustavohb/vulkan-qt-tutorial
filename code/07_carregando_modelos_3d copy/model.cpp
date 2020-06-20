#include "model.h"

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#endif


void Model::readOBJFile(QString const &filePath) {

    tinyobj::attrib_t attribs;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    bool result = tinyobj::LoadObj(
        &attribs,
        &shapes,
        &materials,
        &warn,
        &err,
        filePath.toStdString().c_str()
    );

    if (!warn.empty()) {
        qDebug(warn.c_str());
    }

    if (!err.empty()) {
        qDebug(err.c_str());
    }

    if (!result) {
        qFatal("Could no open file: %s", filePath.toStdString().c_str());
    }

    const float fmin = std::numeric_limits<float>::lowest();
    const float fmax = std::numeric_limits<float>::max();

    QVector3D minDimension = QVector3D(fmax, fmax, fmax);
    QVector3D maxDimension = QVector3D(fmin, fmin, fmin);

    if (vertices.size()) {
        vertices.clear();
    }

    for (const tinyobj::shape_t &shape : shapes) {
        for (const tinyobj::index_t &index : shape.mesh.indices) {
            Vertex vertex = {};

            size_t indexTemp;

            indexTemp = index.vertex_index * 3;
            vertex.pos = {
                attribs.vertices[indexTemp],
                attribs.vertices[indexTemp + 1],
                attribs.vertices[indexTemp + 2]
            };

            indexTemp = index.texcoord_index * 2;
            vertex.texCoord = {
                attribs.texcoords[indexTemp + 0],
                1.0f - attribs.texcoords[indexTemp + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (vertex.pos.x() < minDimension.x()) {
                minDimension.setX(vertex.pos.x());
            }
            if (vertex.pos.x() > maxDimension.x()) {
                maxDimension.setX(vertex.pos.x());
            }

            if (vertex.pos.y() < minDimension.y()) {
                minDimension.setY(vertex.pos.y());
            }
            if (vertex.pos.y() > maxDimension.y()) {
                maxDimension.setY(vertex.pos.y());
            }

            if (vertex.pos.z() < minDimension.z()) {
                minDimension.setZ(vertex.pos.z());
            }
            if (vertex.pos.z() > maxDimension.z()) {
                maxDimension.setZ(vertex.pos.z());
            }




            vertices.push_back(vertex);
        }
    }

    float distance = qMax(
        maxDimension.x() - minDimension.x(),
        qMax(maxDimension.y() - minDimension.y(),
        maxDimension.z() - minDimension.z())
    );

    float sc = 1.0 / distance;
    QVector3D center = (maxDimension + minDimension) / 2;

    transformation.scale(sc);
    transformation.translate(-center);

    /*
    tinyobj::attrib_t attribs;

    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    bool result = tinyobj::LoadObj(&attribs, &shapes, &materials, &warn, &err, filePath.toStdString().c_str());

    if (!warn.empty()) {
        qDebug(warn.c_str());
    }

    if (!err.empty()) {
        qDebug(err.c_str());
    }

    if (!result) {
        qFatal("Could no open file: %s", filePath.toStdString().c_str());
    }


    const float fmin = std::numeric_limits<float>::lowest();
    const float fmax = std::numeric_limits<float>::max();

    QVector3D minDimension = QVector3D(fmax, fmax, fmax);
    QVector3D maxDimension = QVector3D(fmin, fmin, fmin);

    if (vertices.size()) {
        vertices.clear();
    }

    qDebug(" # of vertices  = %d\n", (int)(attribs.vertices.size()) / 3);
    qDebug(" # of normals   = %d\n", (int)(attribs.normals.size()) / 3);
    qDebug(" # of texcoords = %d\n", (int)(attribs.texcoords.size()) / 2);
    qDebug(" # of materials = %d\n", (int)materials.size());
    qDebug(" # of shapes    = %d\n", (int)shapes.size());

    if (vertices.size()) {
        vertices.clear();
    }


    for (const tinyobj::shape_t &shape : shapes) {
        for (const tinyobj::index_t &index : shape.mesh.indices) {
            Vertex vertex = {};

            size_t indexTemp;

            if (index.vertex_index > -1) {
                indexTemp = index.vertex_index * 3;
                vertex.pos = {
                    attribs.vertices[indexTemp],
                    attribs.vertices[indexTemp + 1],
                    attribs.vertices[indexTemp + 2]
                };

                if (vertex.pos.x() < minDimension.x()) {
                    minDimension.setX(vertex.pos.x());
                }
                if (vertex.pos.x() > maxDimension.x()) {
                    maxDimension.setX(vertex.pos.x());
                }

                if (vertex.pos.y() < minDimension.y()) {
                    minDimension.setY(vertex.pos.y());
                }
                if (vertex.pos.y() > maxDimension.y()) {
                    maxDimension.setY(vertex.pos.y());
                }

                if (vertex.pos.z() < minDimension.z()) {
                    minDimension.setZ(vertex.pos.z());
                }
                if (vertex.pos.z() > maxDimension.z()) {
                    maxDimension.setZ(vertex.pos.z());
                }
            }

            if (index.texcoord_index > -1) {
            //if (attribs.texcoords.size()) {
                indexTemp = index.texcoord_index * 2;
                vertex.texCoord = {
                    attribs.texcoords[indexTemp + 0],
                    1.0f - attribs.texcoords[indexTemp + 1]
                };
            } else {
                vertex.texCoord = {0.0, 0.0};
            }

            if (attribs.colors.size()) {
                indexTemp = index.vertex_index * 3;
                vertex.color = {
                    attribs.colors[indexTemp],
                    attribs.colors[indexTemp + 1],
                    attribs.colors[indexTemp + 2]
                };
            } else {
                vertex.color = {1.0f, 1.0f, 1.0f};
            }


            vertices.push_back(vertex);
        }
    }


    float distance = qMax(maxDimension.x() - minDimension.x(), qMax(maxDimension.y() - minDimension.y(), maxDimension.z() - minDimension.z()));
    float sc = 1.0 / distance;
    QVector3D center = (maxDimension + minDimension) / 2;

    transformation.scale(sc);
    transformation.translate(-center);
    */
    //if (attribs.normals.size() == 0) {
        //createNormals();
    //}
}



/*
void Model::readOBJFile(QString const &filePath) {
    tinyobj::attrib_t attribs;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    bool result = tinyobj::LoadObj(
        &attribs,
        &shapes,
        &materials,
        &warn,
        &err,
        filePath.toStdString().c_str()
    );

    if (!warn.empty()) {
        qDebug(warn.c_str());
    }

    if (!err.empty()) {
        qDebug(err.c_str());
    }

    if (!result) {
        qFatal("Could no open file: %s", filePath.toStdString().c_str());
    }

    const float fmin = std::numeric_limits<float>::lowest();
    const float fmax = std::numeric_limits<float>::max();

    QVector3D minDimension = QVector3D(fmax, fmax, fmax);
    QVector3D maxDimension = QVector3D(fmin, fmin, fmin);

    if (vertices.size()) {
        vertices.clear();
    }

    qDebug(" # of vertices  = %d\n", (int)(attribs.vertices.size()) / 3);
    qDebug(" # of normals   = %d\n", (int)(attribs.normals.size()) / 3);
    qDebug(" # of texcoords = %d\n", (int)(attribs.texcoords.size()) / 2);
    qDebug(" # of materials = %d\n", (int)materials.size());
    qDebug(" # of shapes    = %d\n", (int)shapes.size());

    for (const tinyobj::shape_t &shape : shapes) {
        for (const tinyobj::index_t &index : shape.mesh.indices) {
            Vertex vertex = {};

            size_t indexTemp;

            if (index.vertex_index > -1) {
                indexTemp = index.vertex_index * 3;
                vertex.pos = {
                    attribs.vertices[indexTemp],
                    attribs.vertices[indexTemp + 1],
                    attribs.vertices[indexTemp + 2]
                };

                if (vertex.pos.x() < minDimension.x()) {
                    minDimension.setX(vertex.pos.x());
                }
                if (vertex.pos.x() > maxDimension.x()) {
                    maxDimension.setX(vertex.pos.x());
                }

                if (vertex.pos.y() < minDimension.y()) {
                    minDimension.setY(vertex.pos.y());
                }
                if (vertex.pos.y() > maxDimension.y()) {
                    maxDimension.setY(vertex.pos.y());
                }

                if (vertex.pos.z() < minDimension.z()) {
                    minDimension.setZ(vertex.pos.z());
                }
                if (vertex.pos.z() > maxDimension.z()) {
                    maxDimension.setZ(vertex.pos.z());
                }
            }

            if (index.texcoord_index > -1) {
                indexTemp = index.texcoord_index * 2;
                vertex.texCoord = {
                    attribs.texcoords[indexTemp + 0],
                    1.0f - attribs.texcoords[indexTemp + 1]
                };
            }
            else {
                vertex.texCoord = {0.0, 0.0};
            }

            vertex.color = {1.0f, 1.0f, 1.0f};


            vertices.push_back(vertex);
        }
    }

    float distance = qMax(
        maxDimension.x() - minDimension.x(),
        qMax(maxDimension.y() - minDimension.y(),
        maxDimension.z() - minDimension.z())
    );

    float sc = 1.0 / distance;
    QVector3D center = (maxDimension + minDimension) / 2;

    transformation.scale(sc);
    transformation.translate(-center);

}
*/
