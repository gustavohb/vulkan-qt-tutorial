#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

#include "vulkanwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    VulkanWindow *m_vulkanWindow;
};

#endif // MAINWINDOW_H
