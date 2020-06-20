#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "vulkanwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    m_vulkanWindow = new VulkanWindow();
    QWidget *vulkanWrapper = QWidget::createWindowContainer(m_vulkanWindow);
    QGridLayout *vulkanGrid = new QGridLayout(ui->vulkanFrame);
    vulkanGrid->addWidget(vulkanWrapper);
}

MainWindow::~MainWindow() {
    delete ui;
}
