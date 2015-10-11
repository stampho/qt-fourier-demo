#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "fimage.h"
#include "ft.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //FImage image = FImage::createFromFile(QStringLiteral(":/images/qt-logo-128.png"));
    FImage image = FImage::rectangle(QSize(64, 64), QSize(16, 8));

    QPixmap pixmap = QPixmap::fromImage(image);
    ui->originalImage->setPixmap(pixmap);
    ui->originalImage->setFixedSize(pixmap.size());

    FT fourier(&image);

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
