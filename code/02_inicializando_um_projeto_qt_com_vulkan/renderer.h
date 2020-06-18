#ifndef RENDERER_H
#define RENDERER_H

#include <QVulkanWindowRenderer>

class VulkanWindow;

class Renderer : public QVulkanWindowRenderer {
public:
    Renderer(VulkanWindow *window);

    void startNextFrame() override;

private:

    VulkanWindow *m_window = nullptr;
};

#endif // RENDERER_H
