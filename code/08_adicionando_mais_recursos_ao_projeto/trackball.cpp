#include "trackball.h"

#include <cmath>
#include <qmath.h>

Trackball::Trackball()
{

}

Trackball::Trackball(float angularVelocity, const QVector3D& axis)
    : m_axis(axis)
    , m_angularVelocity(angularVelocity)
    , m_pressed(false) {
    m_rotation = QQuaternion();
    m_lastTime = QTime::currentTime();
}

void Trackball::move(const QPointF& p) {
    if (!m_pressed)
        return;

    QTime currentTime = QTime::currentTime();
    int msecs = m_lastTime.msecsTo(currentTime);
    if (msecs <= 20)
        return;

    QVector3D lastPos3D =
        QVector3D(m_lastPos.x(), m_lastPos.y(), 0.0f);
    float sqrZ = 1 - QVector3D::dotProduct(lastPos3D, lastPos3D);
    if (sqrZ > 0)
        lastPos3D.setZ(std::sqrt(sqrZ));
    else
        lastPos3D.normalize();

    QVector3D currentPos3D = QVector3D(p.x(), p.y(), 0.0f);
    sqrZ = 1 - QVector3D::dotProduct(currentPos3D, currentPos3D);
    if (sqrZ > 0)
        currentPos3D.setZ(std::sqrt(sqrZ));
    else
        currentPos3D.normalize();

    m_axis = QVector3D::crossProduct(lastPos3D, currentPos3D);
    float angle = qRadiansToDegrees(std::acos(QVector3D::dotProduct(lastPos3D.normalized(), currentPos3D.normalized())));

    m_rotation = QQuaternion::fromAxisAndAngle(m_axis, angle) * m_rotation;

    m_angularVelocity = angle / msecs;
    m_lastPos = p;
    m_lastTime = currentTime;
}

QQuaternion Trackball::rotation() const {
    if (m_pressed)
        return m_rotation;

    QTime currentTime = QTime::currentTime();
    float angle = m_angularVelocity *
        m_lastTime.msecsTo(currentTime);
    return QQuaternion::fromAxisAndAngle(m_axis, angle) *
        m_rotation;
}

void Trackball::push(const QPointF& p) {
    m_rotation = rotation();
    m_pressed = true;
    m_lastTime = QTime::currentTime();
    m_lastPos = p;
    m_angularVelocity = 0.0f;
}

void Trackball::release(const QPointF& p) {
    move(p);
    m_pressed = false;
}
