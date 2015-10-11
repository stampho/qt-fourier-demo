#include "fimage.h"

#include <QDebug>
#include <QPainter>

FImage FImage::createFromFile(const QString &fileName)
{
    QImage image = QImage(fileName);
    image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    return FImage(image);
}

FImage FImage::rectangle(const QSize &imageSize, const QSize &rectangleSize)
{
    int topLeftX = imageSize.width() / 2 - rectangleSize.width() / 2;
    int topLeftY = imageSize.height() / 2 - rectangleSize.height() / 2;
    QPoint topLeft(topLeftX, topLeftY);

    QRect rect(topLeft, rectangleSize);

    QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::black);

    QPainterPath path;
    path.addRect(rect);

    QPainter paint(&image);
    paint.fillPath(path, Qt::white);

    return FImage(image);
}

FImage::FImage(const QImage &image)
    : QImage(image)
{
    if (!isGrayscale())
        convertToGrayscale();

    fillData();
}

FImage::FImage(uchar *data, int width, int height)
    : QImage(width, height, QImage::Format_ARGB32_Premultiplied)
{
    int size = width * height;
    m_data = QVector<uchar>(size);
    memcpy(m_data.data(), data, size);

    fill(Qt::white);
    updateImage();
}

QVector<uchar> FImage::data() const
{
    return m_data;
}


void FImage::convertToGrayscale()
{
    for (int y = 0; y < height(); ++y) {
        QRgb *line = (QRgb*)scanLine(y);

        for (int x = 0; x < width(); ++x) {
            const int value = qGray(line[x]);
            line[x] = qRgba(value, value, value, qAlpha(line[x]));
        }
    }
}

void FImage::fillData()
{
    const int d = depth() / 8;
    m_data = QVector<uchar>(width() * height());

    for (int y = 0; y < height(); ++y) {
        uchar *line = scanLine(y);

        for (int x = 0; x < width(); ++x) {
            const uchar value = *(line + x * d);
            m_data[x + y * width()] = value;
        }
    }
}

void FImage::updateImage()
{
    for (int y = 0; y < height(); ++y) {
        QRgb *line = (QRgb*)scanLine(y);

        for (int x = 0; x < width(); ++x) {
            const int value = m_data[x + y * width()];
            line[x] = qRgba(value, value, value, qAlpha(line[x]));
        }
    }
}

QDebug operator<<(QDebug debug, const FImage &image)
{
    QDebug verbose = debug.nospace().noquote();
    QVector<uchar> data = image.data();

    QString formattedValue;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            int index = x + y * image.width();
            formattedValue.sprintf("%3d ", data[index]);
            verbose << formattedValue;
        }
        verbose << "\n";
    }

    return verbose;
}
