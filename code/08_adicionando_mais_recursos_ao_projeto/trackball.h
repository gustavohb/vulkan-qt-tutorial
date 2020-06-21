#ifndef TRACKBALL_H
#define TRACKBALL_H

#include <QVector3D>
#include <QQuaternion>
#include <QTime>

class Trackball
{

public:
    Trackball();
    Trackball(float angularVelocity, const QVector3D& axis);

    void move(const QPointF& p);
    void push(const QPointF& p);
    void release(const QPointF& p);
    QQuaternion rotation() const;

private:
    QQuaternion m_rotation;
    QVector3D m_axis;
    float m_angularVelocity;
    QPointF m_lastPos;
    QTime m_lastTime;
    bool m_pressed;
};

#endif // TRACKBALL_H
