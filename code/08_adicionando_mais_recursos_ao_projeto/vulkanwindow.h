#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H

#include <QVulkanWindow>

#include "renderer.h"
#include "trackball.h"

class VulkanWindow : public QVulkanWindow {
    Q_OBJECT

public:
    VulkanWindow(QWindow *parentWindow = nullptr);
    QVulkanWindowRenderer *createRenderer() override;

    Renderer *renderer() {
        return m_renderer;
    }

    QQuaternion getTrackballRotation() {
        return m_trackball.rotation();
    }

    float getZoom() {
        return m_zoom;
    }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QVulkanInstance m_instance;
    Renderer *m_renderer = nullptr;
    Trackball m_trackball;
    float m_zoom = 0;

private:
    void pickPhysicalDevice();
    QPointF pixelPosToViewPos(const QPointF& p);
};

#endif // VULKANWINDOW_H
