#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "vulkanwindow.h"
#include "model.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    m_vulkanWindow = new VulkanWindow();
    QWidget *vulkanWrapper = QWidget::createWindowContainer(m_vulkanWindow);
    QGridLayout *vulkanGrid = new QGridLayout(ui->vulkanFrame);
    vulkanGrid->addWidget(vulkanWrapper);

    connect(
        ui->loadModelButton,
        SIGNAL(clicked()),
        this,
        SLOT(loadModel())
    );

    connect(
        ui->loadTextureButton,
        SIGNAL(clicked()),
        this,
        SLOT(loadTexture())
    );

    ui->loadTextureButton->setEnabled(false);
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::loadModel() {
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open 3D Model"),
        QDir::homePath(),
        tr("3D Model Files (*.obj *.OBJ)")
    );

    if (!fileName.isEmpty()) {
        QSharedPointer<Model> model =
            QSharedPointer<Model>::create();
        model->readOBJFile(fileName);
        m_vulkanWindow->renderer()->addObject(model);

        ui->loadTextureButton->setEnabled(true);
    }
}

void MainWindow::loadTexture() {
    const QString fileName =QFileDialog::getOpenFileName(
        this,
        tr("Open Image"),
        QDir::homePath(),
        tr("Image Files (*.png *.jpg *.bmp)")
    );

    if (!fileName.isEmpty()) {
        m_vulkanWindow->renderer()->addTextureImage(fileName);
    }
}
