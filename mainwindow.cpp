#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "fimage.h"
#include "dftgpu.h"
#include "dftcpu.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    FImage image = FImage::createFromFile(QStringLiteral(":/images/qt-logo-128.png"));
    //FImage image = FImage::rectangle(QSize(128, 128), QSize(16, 8));
    //FImage image = FImage::rectangle(QSize(256, 256), QSize(16, 8));
    //FImage image = FImage::rectangle(QSize(5, 4), QSize(3, 2));
    //qDebug() << image;

    QPixmap pixmap = QPixmap::fromImage(image);
    ui->originalImage->setPixmap(pixmap);
    ui->originalImage->setFixedSize(pixmap.size());

    //DFTCpu fourier(&image);
    DFTGpu fourier(&image);

    FImage magnitudeImage = fourier.magnitudeImage();
    QPixmap magnitudePixmap = QPixmap::fromImage(magnitudeImage);
    ui->magnitudeImage->setPixmap(magnitudePixmap);
    ui->magnitudeImage->setFixedSize(magnitudePixmap.size());

    FImage phaseImage = fourier.phaseImage();
    QPixmap phasePixmap = QPixmap::fromImage(phaseImage);
    ui->phaseImage->setPixmap(phasePixmap);
    ui->phaseImage->setFixedSize(phasePixmap.size());
}

MainWindow::~MainWindow()
{
    delete ui;
}
