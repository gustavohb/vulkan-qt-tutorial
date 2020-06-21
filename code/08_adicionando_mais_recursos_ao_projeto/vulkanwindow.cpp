#include "vulkanwindow.h"

#include "renderer.h"

#include <QMouseEvent>

VulkanWindow::VulkanWindow(QWindow *parentWindow) : QVulkanWindow(parentWindow) {
    if (!m_instance.create())
        qFatal("Failed to create Vulkan instance: %d", m_instance.errorCode());
    setVulkanInstance(&m_instance);
    pickPhysicalDevice();

    m_trackball = Trackball(-0.05f, QVector3D(0, 1, 0));
}

void VulkanWindow::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        m_trackball.push(pixelPosToViewPos(event->localPos()));
    }
}

void VulkanWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        m_trackball.move(pixelPosToViewPos(event->localPos()));
    } else {
        m_trackball.release(pixelPosToViewPos(event->localPos()));
    }
}

void VulkanWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_trackball.release(pixelPosToViewPos(event->localPos()));
    }
}

void VulkanWindow::wheelEvent(QWheelEvent *event) {
    m_zoom += 0.001 * event->delta();
}

void VulkanWindow::pickPhysicalDevice() {
    QVector<VkPhysicalDeviceProperties> devices = availablePhysicalDevices();
    int discreteGPUIndex = -1;
    int integratedGPUIndex = -1;
    for (int i = 0; i < devices.length(); ++i) {
        const VkPhysicalDeviceProperties &dev = devices.at(i);
        if (dev.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            discreteGPUIndex = i;
        else if (dev.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            integratedGPUIndex = i;
    }

    if (discreteGPUIndex != -1)
        setPhysicalDeviceIndex(discreteGPUIndex);
    else if (integratedGPUIndex != -1)
        setPhysicalDeviceIndex(integratedGPUIndex);
    else
        qFatal("No Vulkan capable GPU found.");
}

QPointF VulkanWindow::pixelPosToViewPos(const QPointF& p) {
    float x = ((float) p.x()) / (width() / 2);
    float y = ((float)p.y()) / (height() / 2);
    x = x - 1;
    y = 1 - y;

    return QPointF(x, y);
}

QVulkanWindowRenderer *VulkanWindow::createRenderer() {
    m_renderer = new Renderer(this);
    return m_renderer;
}


