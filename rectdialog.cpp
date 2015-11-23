#include "rectdialog.h"
#include "ui_rectdialog.h"

#include "fimage.h"

RectDialog::RectDialog(const QString &rectCode, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RectDialog)
    , m_ignoreSignals(false)
{
    ui->setupUi(this);
    if (FImage::isRectCode(rectCode)) {
        ui->rectCodeLine->setText(rectCode);
        applyRectCode();
    } else
        updateRectCode();

    connect(ui->bgWidthSpin, SIGNAL(valueChanged(int)), this, SLOT(updateRectCode()));
    connect(ui->bgHeightSpin, SIGNAL(valueChanged(int)), this, SLOT(updateRectCode()));
    connect(ui->contentWidthSpin, SIGNAL(valueChanged(int)), this, SLOT(updateRectCode()));
    connect(ui->contentHeightSpin, SIGNAL(valueChanged(int)), this, SLOT(updateRectCode()));
    connect(ui->bgColorSlider, SIGNAL(valueChanged(int)), this, SLOT(updateRectCode()));
    connect(ui->contentColorSlider, SIGNAL(valueChanged(int)), this, SLOT(updateRectCode()));

    connect(ui->applyButton, SIGNAL(pressed()), this, SLOT(applyRectCode()));
}

RectDialog::~RectDialog()
{
    delete ui;
}

QString RectDialog::getRectCode() const
{
    return m_rectCode;
}

void RectDialog::updateRectCode()
{
    if (m_ignoreSignals)
        return;

    QStringList buffer;

    buffer.append("rect");
    buffer.append(QString::number(ui->bgWidthSpin->value()));
    buffer.append(QString::number(ui->bgHeightSpin->value()));
    buffer.append(QString::number(ui->contentWidthSpin->value()));
    buffer.append(QString::number(ui->contentHeightSpin->value()));
    buffer.append(QString::number(ui->bgColorSlider->value()));
    buffer.append(QString::number(ui->contentColorSlider->value()));

    m_rectCode = buffer.join("-");
    ui->rectCodeLine->setText(m_rectCode);
    updatePreview();
}

void RectDialog::applyRectCode()
{
    m_ignoreSignals = true;

    m_rectCode = ui->rectCodeLine->text();
    QStringList values = m_rectCode.split("-");
    ui->bgWidthSpin->setValue(values[1].toInt());
    ui->bgHeightSpin->setValue(values[2].toInt());
    ui->contentWidthSpin->setValue(values[3].toInt());
    ui->contentHeightSpin->setValue(values[4].toInt());
    ui->bgColorSlider->setValue(values[5].toInt());
    ui->contentColorSlider->setValue(values[6].toInt());
    updatePreview();

    m_ignoreSignals = false;
}

void RectDialog::updatePreview()
{
    FImage image = FImage::rectangle(m_rectCode);
    QPixmap pixmap = QPixmap::fromImage(image);
    ui->previewImage->setPixmap(pixmap);
    ui->previewImage->setFixedSize(pixmap.size());
}
