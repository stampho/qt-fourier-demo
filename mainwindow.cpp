#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <algorithm>
#include <QFileDialog>
#include <QFontDatabase>
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
    m_progress->setRange(0, 100);
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
        ui->benchFtCombo->insertItem(i, text);
    }

    ui->refFtCombo->setCurrentIndex(FT::FFTCPU);
    ui->refElapsedLabel->setStyleSheet("QLabel { color: red; }");
    ui->modFtCombo->setCurrentIndex(FT::FFTGPU);
    ui->modElapsedLabel->setStyleSheet("QLabel { color: red; }");
    ui->benchFtCombo->setCurrentIndex(FT::FFTGPU);

    ui->compareInputLine->setText(QStringLiteral(":/images/qt-logo-128.png"));
    ui->benchInputLine->setText(QStringLiteral("rect-128-128-32-16-50-200"));

    ui->benchResultView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    connect(ui->browseButton, SIGNAL(pressed()), this, SLOT(showImageBrowser()));
    connect(ui->compareRectButton, SIGNAL(pressed()), this, SLOT(showRectDialogForCompare()));
    connect(ui->benchRectButton, SIGNAL(pressed()), this, SLOT(showRectDialogForBench()));

    rangeMinSBPrevValue = ui->rangeMinSB->value();
    connect(ui->rangeMinSB, SIGNAL(valueChanged(int)), this, SLOT(roundSBToPowerOfTwo(int)));
    rangeMaxSBPrevValue = ui->rangeMaxSB->value();
    connect(ui->rangeMaxSB, SIGNAL(valueChanged(int)), this, SLOT(roundSBToPowerOfTwo(int)));

    connect(ui->startCompareButton, SIGNAL(pressed()), m_progress, SLOT(show()));
    connect(ui->startCompareButton, SIGNAL(pressed()), this, SLOT(startCompare()));

    connect(ui->startBenchButton, SIGNAL(pressed()), m_progress, SLOT(show()));
    connect(ui->startBenchButton, SIGNAL(pressed()), this, SLOT(startBench()));
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
        ui->compareInputLine->setText(dialog.selectedFiles().first());
}

void MainWindow::showRectDialogForCompare()
{
    RectDialog dialog(ui->compareInputLine->text(), this);
    if (dialog.exec())
        ui->compareInputLine->setText(dialog.getRectCode());
}

void MainWindow::showRectDialogForBench()
{
    RectDialog dialog(ui->benchInputLine->text(), this);
    if (dialog.exec())
        ui->benchInputLine->setText(dialog.getRectCode());
}

void MainWindow::roundSBToPowerOfTwo(int value)
{
    QSpinBox *sb = (QSpinBox *)sender();
    sb->blockSignals(true);

    int prevValue = (sb == ui->rangeMinSB) ? rangeMinSBPrevValue : rangeMaxSBPrevValue;
    int nextValue = value;

    if (prevValue < value)
        nextValue = qNextPowerOfTwo(value);
    else if(prevValue > value)
        nextValue = qNextPowerOfTwo(value) >> 1;

    if (sb == ui->rangeMinSB)
        rangeMinSBPrevValue = nextValue;
    else
        rangeMaxSBPrevValue = nextValue;

    sb->setValue(nextValue);
    sb->blockSignals(false);
}

void MainWindow::startCompare()
{
    int elapsed;
    const float progressStep = 100.0 / 130.0;
    m_progress->setValue(0);

    QString input = ui->compareInputLine->text();
    FImage image;

    m_progress->setValue(2 * progressStep);

    if (FImage::isRectCode(input))
        image = FImage::rectangle(input);
    else
        image = FImage::createFromFile(input);

    m_progress->setValue(6 * progressStep);

    QPixmap pixmap = QPixmap::fromImage(image);
    ui->originalImageRef->setPixmap(pixmap);
    ui->originalImageRef->setFixedSize(pixmap.size());
    ui->originalImageMod->setPixmap(pixmap);
    ui->originalImageMod->setFixedSize(pixmap.size());

    m_progress->setValue(10 * progressStep);
    FT *fourierRef = FT::createFT((FT::FTType)ui->refFtCombo->currentIndex(), &image);
    elapsed = fourierRef->init();
    ui->refElapsedLabel->setText(QString("%1 ms").arg(QString::number(elapsed)));
    m_progress->setValue(20 * progressStep);

    FImage magnitudeImageRef = fourierRef->magnitudeImage();
    QPixmap magnitudePixmapRef = QPixmap::fromImage(magnitudeImageRef);
    ui->magnitudeImageRef->setPixmap(magnitudePixmapRef);
    ui->magnitudeImageRef->setFixedSize(magnitudePixmapRef.size());
    m_progress->setValue(30 * progressStep);

    FImage recMagnitudeImageRef = fourierRef->reconstructFromMagnitude();
    QPixmap recMagnitudePixmapRef = QPixmap::fromImage(recMagnitudeImageRef);
    ui->recMagnitudeImageRef->setPixmap(recMagnitudePixmapRef);
    ui->recMagnitudeImageRef->setFixedSize(recMagnitudePixmapRef.size());
    m_progress->setValue(40 * progressStep);

    FImage phaseImageRef = fourierRef->phaseImage();
    QPixmap phasePixmapRef = QPixmap::fromImage(phaseImageRef);
    ui->phaseImageRef->setPixmap(phasePixmapRef);
    ui->phaseImageRef->setFixedSize(phasePixmapRef.size());
    m_progress->setValue(50 * progressStep);

    FImage recPhaseImageRef = fourierRef->reconstructFromPhase();
    QPixmap recPhasePixmapRef = QPixmap::fromImage(recPhaseImageRef);
    ui->recPhaseImageRef->setPixmap(recPhasePixmapRef);
    ui->recPhaseImageRef->setFixedSize(recPhasePixmapRef.size());
    m_progress->setValue(60 * progressStep);

    FImage recOriginalImageRef = fourierRef->reconstructOriginalImage();
    QPixmap recOriginalPixmapRef = QPixmap::fromImage(recOriginalImageRef);
    ui->recOriginalImageRef->setPixmap(recOriginalPixmapRef);
    ui->recOriginalImageRef->setFixedSize(recOriginalPixmapRef.size());
    m_progress->setValue(70 * progressStep);

    delete fourierRef;


    FT *fourierMod = FT::createFT((FT::FTType)ui->modFtCombo->currentIndex(), &image);
    elapsed = fourierMod->init();
    ui->modElapsedLabel->setText(QString("%1 ms").arg(QString::number(elapsed)));
    m_progress->setValue(80 * progressStep);

    FImage magnitudeImageMod = fourierMod->magnitudeImage();
    QPixmap magnitudePixmapMod = QPixmap::fromImage(magnitudeImageMod);
    ui->magnitudeImageMod->setPixmap(magnitudePixmapMod);
    ui->magnitudeImageMod->setFixedSize(magnitudePixmapMod.size());
    m_progress->setValue(90 * progressStep);

    FImage recMagnitudeImageMod = fourierMod->reconstructFromMagnitude();
    QPixmap recMagnitudePixmapMod = QPixmap::fromImage(recMagnitudeImageMod);
    ui->recMagnitudeImageMod->setPixmap(recMagnitudePixmapMod);
    ui->recMagnitudeImageMod->setFixedSize(recMagnitudePixmapMod.size());
    m_progress->setValue(100 * progressStep);

    FImage phaseImageMod = fourierMod->phaseImage();
    QPixmap phasePixmapMod = QPixmap::fromImage(phaseImageMod);
    ui->phaseImageMod->setPixmap(phasePixmapMod);
    ui->phaseImageMod->setFixedSize(phasePixmapMod.size());
    m_progress->setValue(110 * progressStep);

    FImage recPhaseImageMod = fourierMod->reconstructFromPhase();
    QPixmap recPhasePixmapMod = QPixmap::fromImage(recPhaseImageMod);
    ui->recPhaseImageMod->setPixmap(recPhasePixmapMod);
    ui->recPhaseImageMod->setFixedSize(recPhasePixmapMod.size());
    m_progress->setValue(120 * progressStep);

    FImage recOriginalImageMod = fourierMod->reconstructOriginalImage();
    QPixmap recOriginalPixmapMod = QPixmap::fromImage(recOriginalImageMod);
    ui->recOriginalImageMod->setPixmap(recOriginalPixmapMod);
    ui->recOriginalImageMod->setFixedSize(recOriginalPixmapMod.size());
    m_progress->setValue(130 * progressStep);

    delete fourierMod;
}

void MainWindow::startBench()
{
    int rangeMin = ui->rangeMinSB->value();
    int rangeMax = ui->rangeMaxSB->value();

    Q_ASSERT(IS_POWER_OF_TWO(rangeMin));
    Q_ASSERT(IS_POWER_OF_TWO(rangeMax));

    int sizeCount = (int)(log2(rangeMax) - log2(rangeMin) + 1);
    if (sizeCount <= 0)
        return;

    int iterations = ui->benchIterRB->value();
    float fourierCount = iterations * (sizeCount + 2);
    float progressStep = 100.0 / fourierCount;
    float progressCounter;

    FT::FTType algorithm = (FT::FTType)ui->benchFtCombo->currentIndex();

    QString input = ui->benchInputLine->text();
    Q_ASSERT(FImage::isRectCode(input));

    ui->benchResultView->clear();
    progressCounter = 0.0;
    m_progress->setValue(progressCounter);

    for (int size = rangeMin; size <= rangeMax; size = qNextPowerOfTwo(size)) {
        QVector<int> results;
        FImage rectangle = FImage::rectangle(input, QSize(size, size));

        FT *fourierWarmUp = FT::createFT(algorithm, &rectangle);
        fourierWarmUp->bench();
        delete fourierWarmUp;
        progressCounter += progressStep;
        m_progress->setValue(progressCounter);

        for (int i = 0; i < iterations; ++i) {
            FT *fourier = FT::createFT(algorithm, &rectangle);
            results.append(fourier->bench());
            delete fourier;

            progressCounter += progressStep;
            m_progress->setValue(progressCounter);
        }

        int result = 0;
        if (ui->benchMinRB->isChecked())
            result = *std::min_element(results.begin(), results.end());
        else if (ui->benchMaxRB->isChecked())
            result = *std::max_element(results.begin(), results.end());
        else if (ui->benchMeanRB->isChecked()) {
            float sum = 0.0;
            Q_FOREACH (int r, results)
                sum += (float)r;

            result = qRound(sum / (float)results.count());
        }

        QStringList benchSum;
        benchSum.append(QStringLiteral("%1").arg(rectangle.id()).leftJustified(28, ' '));
        benchSum.append(QString::number(size).rightJustified(4, ' '));
        benchSum.append(QStringLiteral("%1 ms").arg(QString::number(result).rightJustified(4, ' ')));

        QStringList resultList;
        Q_FOREACH (int r, results)
            resultList.append(QString::number(r).rightJustified(4, ' '));

        ui->benchResultView->append(QStringLiteral("%1\t%2").arg(benchSum.join(" ")).arg(resultList.join(" ")));

        progressCounter += progressStep;
        m_progress->setValue(progressCounter);
    }

    m_progress->setValue(100);
}
