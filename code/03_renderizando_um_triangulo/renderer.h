#ifndef RENDERER_H
#define RENDERER_H

#include <QVulkanWindowRenderer>
#include <QVulkanDeviceFunctions>

class VulkanWindow;

class Renderer : public QVulkanWindowRenderer {
public:
    Renderer(VulkanWindow *window);

    void initResources() override;
    void startNextFrame() override;

private:

    VulkanWindow *m_window = nullptr;
    QVulkanDeviceFunctions *m_deviceFunctions;

private:
    void initPipeline();

    static QByteArray readFile(const QString &fileName);
};

#endif // RENDERER_H
