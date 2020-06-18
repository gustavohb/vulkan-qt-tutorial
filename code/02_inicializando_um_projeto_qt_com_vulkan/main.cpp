#include "mainwindow.h"
#include <QApplication>
#include <QVulkanInstance>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QVulkanInstance inst;
    if (!inst.create())
        qFatal("Failed to create Vulkan instance: %d", inst.errorCode());
    MainWindow w;
    w.show();
    return a.exec();
}
