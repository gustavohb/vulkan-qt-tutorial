#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H

#include <QVulkanWindow>

class Renderer;

class VulkanWindow : public QVulkanWindow {
    Q_OBJECT

public:
    VulkanWindow(QWindow *parentWindow = nullptr);
    QVulkanWindowRenderer *createRenderer() override;

private:
    QVulkanInstance m_instance;
    Renderer *m_renderer = nullptr;

private:
    void pickPhysicalDevice();
};

#endif // VULKANWINDOW_H
