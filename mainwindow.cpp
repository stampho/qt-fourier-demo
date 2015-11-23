#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QProgressDialog>

#include "fimage.h"
#include "ft.h"
#include "rectdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_progress(new QProgressDialog(this))

{
    ui->setupUi(this);
    m_progress->setWindowModality(Qt::WindowModal);
    m_progress->setRange(0, 130);
    m_progress->setCancelButton(0);
    m_progress->setLabelText("Working on Fourier Transformation...");
    m_progress->cancel();

    for (int i = 0; i < FT::FTTYPECOUNT; ++i) {
        QString text;

        switch (i) {
        case FT::DFTCPU:
            text = QStringLiteral("DFT CPU");
            break;
        case FT::DFTGPU:
            text = QStringLiteral("DFT GPU");
            break;
        case FT::FFTCPU:
            text = QStringLiteral("FFT CPU");
            break;
        case FT::FFTGPU:
            text = QStringLiteral("FFT GPU");
            break;
        default:
            text = QStringLiteral("Unknown");
        }

        ui->refFtCombo->insertItem(i, text);
        ui->modFtCombo->insertItem(i, text);
    }

    ui->refFtCombo->setCurrentIndex(FT::FFTCPU);
    ui->modFtCombo->setCurrentIndex(FT::FFTGPU);

    ui->imageLine->setText(QStringLiteral(":/images/qt-logo-128.png"));
    connect(ui->browseButton, SIGNAL(pressed()), this, SLOT(showImageBrowser()));
    connect(ui->cRectButton, SIGNAL(pressed()), this, SLOT(showRectDialog()));
    connect(ui->startCompareButton, SIGNAL(pressed()), m_progress, SLOT(show()));
    connect(ui->startCompareButton, SIGNAL(pressed()), this, SLOT(startCompare()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showImageBrowser()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Images (*.bmp *.jpg *.png)");
    if (dialog.exec())
        ui->imageLine->setText(dialog.selectedFiles().first());
}

void MainWindow::showRectDialog()
{
    RectDialog dialog(this);
    if (dialog.exec())
        ui->imageLine->setText(dialog.getRectCode());
}

void MainWindow::startCompare()
{
    m_progress->setValue(0);

    QString input = ui->imageLine->text();
    FImage image;

    if (FImage::isRectCode(input))
        image = FImage::rectangle(input);
    else
        image = FImage::createFromFile(input);

    QPixmap pixmap = QPixmap::fromImage(image);
    ui->originalImageRef->setPixmap(pixmap);
    ui->originalImageRef->setFixedSize(pixmap.size());
    ui->originalImageMod->setPixmap(pixmap);
    ui->originalImageMod->setFixedSize(pixmap.size());

    m_progress->setValue(10);
    FT *fourierRef = FT::createFT((FT::FTType)ui->refFtCombo->currentIndex(), &image);
    m_progress->setValue(20);

    FImage magnitudeImageRef = fourierRef->magnitudeImage();
    QPixmap magnitudePixmapRef = QPixmap::fromImage(magnitudeImageRef);
    ui->magnitudeImageRef->setPixmap(magnitudePixmapRef);
    ui->magnitudeImageRef->setFixedSize(magnitudePixmapRef.size());
    m_progress->setValue(30);

    FImage recMagnitudeImageRef = fourierRef->reconstructFromMagnitude();
    QPixmap recMagnitudePixmapRef = QPixmap::fromImage(recMagnitudeImageRef);
    ui->recMagnitudeImageRef->setPixmap(recMagnitudePixmapRef);
    ui->recMagnitudeImageRef->setFixedSize(recMagnitudePixmapRef.size());
    m_progress->setValue(40);

    FImage phaseImageRef = fourierRef->phaseImage();
    QPixmap phasePixmapRef = QPixmap::fromImage(phaseImageRef);
    ui->phaseImageRef->setPixmap(phasePixmapRef);
    ui->phaseImageRef->setFixedSize(phasePixmapRef.size());
    m_progress->setValue(50);

    FImage recPhaseImageRef = fourierRef->reconstructFromPhase();
    QPixmap recPhasePixmapRef = QPixmap::fromImage(recPhaseImageRef);
    ui->recPhaseImageRef->setPixmap(recPhasePixmapRef);
    ui->recPhaseImageRef->setFixedSize(recPhasePixmapRef.size());
    m_progress->setValue(60);

    FImage recOriginalImageRef = fourierRef->reconstructOriginalImage();
    QPixmap recOriginalPixmapRef = QPixmap::fromImage(recOriginalImageRef);
    ui->recOriginalImageRef->setPixmap(recOriginalPixmapRef);
    ui->recOriginalImageRef->setFixedSize(recOriginalPixmapRef.size());
    m_progress->setValue(70);

    delete fourierRef;


    FT *fourierMod = FT::createFT((FT::FTType)ui->modFtCombo->currentIndex(), &image);
    m_progress->setValue(80);

    FImage magnitudeImageMod = fourierMod->magnitudeImage();
    QPixmap magnitudePixmapMod = QPixmap::fromImage(magnitudeImageMod);
    ui->magnitudeImageMod->setPixmap(magnitudePixmapMod);
    ui->magnitudeImageMod->setFixedSize(magnitudePixmapMod.size());
    m_progress->setValue(90);

    FImage recMagnitudeImageMod = fourierMod->reconstructFromMagnitude();
    QPixmap recMagnitudePixmapMod = QPixmap::fromImage(recMagnitudeImageMod);
    ui->recMagnitudeImageMod->setPixmap(recMagnitudePixmapMod);
    ui->recMagnitudeImageMod->setFixedSize(recMagnitudePixmapMod.size());
    m_progress->setValue(100);

    FImage phaseImageMod = fourierMod->phaseImage();
    QPixmap phasePixmapMod = QPixmap::fromImage(phaseImageMod);
    ui->phaseImageMod->setPixmap(phasePixmapMod);
    ui->phaseImageMod->setFixedSize(phasePixmapMod.size());
    m_progress->setValue(110);

    FImage recPhaseImageMod = fourierMod->reconstructFromPhase();
    QPixmap recPhasePixmapMod = QPixmap::fromImage(recPhaseImageMod);
    ui->recPhaseImageMod->setPixmap(recPhasePixmapMod);
    ui->recPhaseImageMod->setFixedSize(recPhasePixmapMod.size());
    m_progress->setValue(120);

    FImage recOriginalImageMod = fourierMod->reconstructOriginalImage();
    QPixmap recOriginalPixmapMod = QPixmap::fromImage(recOriginalImageMod);
    ui->recOriginalImageMod->setPixmap(recOriginalPixmapMod);
    ui->recOriginalImageMod->setFixedSize(recOriginalPixmapMod.size());
    m_progress->setValue(130);

    delete fourierMod;
}
