#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "fimage.h"
#include "dftgpu.h"
#include "dftcpu.h"
#include "fftcpu.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    FImage image = FImage::createFromFile(QStringLiteral(":/images/qt-logo-128.png"));
    //FImage image = FImage::rectangle(QSize(128, 128), QSize(16, 8));
    //FImage image = FImage::rectangle(QSize(256, 256), QSize(16, 8));
    //FImage image = FImage::rectangle(QSize(4, 4), QSize(2, 2));
    //qDebug() << image;

    QPixmap pixmap = QPixmap::fromImage(image);
    ui->originalImageRef->setPixmap(pixmap);
    ui->originalImageRef->setFixedSize(pixmap.size());
    ui->originalImageMod->setPixmap(pixmap);
    ui->originalImageMod->setFixedSize(pixmap.size());

    //DFTCpu fourierRef(&image);
    DFTGpu fourierRef(&image);

    FImage magnitudeImageRef = fourierRef.magnitudeImage();
    QPixmap magnitudePixmapRef = QPixmap::fromImage(magnitudeImageRef);
    ui->magnitudeImageRef->setPixmap(magnitudePixmapRef);
    ui->magnitudeImageRef->setFixedSize(magnitudePixmapRef.size());

    FImage recMagnitudeImageRef = fourierRef.reconstructFromMagnitude();
    QPixmap recMagnitudePixmapRef = QPixmap::fromImage(recMagnitudeImageRef);
    ui->recMagnitudeImageRef->setPixmap(recMagnitudePixmapRef);
    ui->recMagnitudeImageRef->setFixedSize(recMagnitudePixmapRef.size());

    FImage phaseImageRef = fourierRef.phaseImage();
    QPixmap phasePixmapRef = QPixmap::fromImage(phaseImageRef);
    ui->phaseImageRef->setPixmap(phasePixmapRef);
    ui->phaseImageRef->setFixedSize(phasePixmapRef.size());

    FImage recPhaseImageRef = fourierRef.reconstructFromPhase();
    QPixmap recPhasePixmapRef = QPixmap::fromImage(recPhaseImageRef);
    ui->recPhaseImageRef->setPixmap(recPhasePixmapRef);
    ui->recPhaseImageRef->setFixedSize(recPhasePixmapRef.size());

    FFTCpu fourierMod(&image);

    FImage magnitudeImageMod = fourierMod.magnitudeImage();
    QPixmap magnitudePixmapMod = QPixmap::fromImage(magnitudeImageMod);
    ui->magnitudeImageMod->setPixmap(magnitudePixmapMod);
    ui->magnitudeImageMod->setFixedSize(magnitudePixmapMod.size());

    FImage recMagnitudeImageMod = fourierMod.reconstructFromMagnitude();
    QPixmap recMagnitudePixmapMod = QPixmap::fromImage(recMagnitudeImageMod);
    ui->recMagnitudeImageMod->setPixmap(recMagnitudePixmapMod);
    ui->recMagnitudeImageMod->setFixedSize(recMagnitudePixmapMod.size());

    FImage phaseImageMod = fourierMod.phaseImage();
    QPixmap phasePixmapMod = QPixmap::fromImage(phaseImageMod);
    ui->phaseImageMod->setPixmap(phasePixmapMod);
    ui->phaseImageMod->setFixedSize(phasePixmapMod.size());

    FImage recPhaseImageMod = fourierMod.reconstructFromPhase();
    QPixmap recPhasePixmapMod = QPixmap::fromImage(recPhaseImageMod);
    ui->recPhaseImageMod->setPixmap(recPhasePixmapMod);
    ui->recPhaseImageMod->setFixedSize(recPhasePixmapMod.size());
}

MainWindow::~MainWindow()
{
    delete ui;
}
