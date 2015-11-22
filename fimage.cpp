#include "fimage.h"

#include <QDebug>
#include <QPainter>

bool FImage::isRectCode(const QString &rectCode)
{
    QStringList values = rectCode.split("-");

    if (values.length() != 7)
        return false;

    if (QString::compare(QStringLiteral("rect"), values[0]) != 0)
        return false;

    return true;
}

FImage FImage::createFromFile(const QString &fileName)
{
    QImage image = QImage(fileName);
    image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    return FImage(image);
}

FImage FImage::rectangle(const QString &rectCode)
{
    if (!FImage::isRectCode(rectCode))
        return FImage();

    QStringList values = rectCode.split("-");
    int bgWidth = values[1].toInt();
    int bgHeight = values[2].toInt();
    int contentWidth = values[3].toInt();
    int contentHeight = values[4].toInt();
    int bgColor = values[5].toInt();
    int fgColor = values[6].toInt();

    return FImage::rectangle(QSize(bgWidth, bgHeight), QSize(contentWidth, contentHeight), bgColor, fgColor);
}

FImage FImage::rectangle(const QSize &imageSize, const QSize &rectangleSize)
{
    return FImage::rectangle(imageSize, rectangleSize, 0, 255);
}

FImage FImage::rectangle(const QSize &imageSize, const QSize &rectangleSize, unsigned bgColor, unsigned fgColor)
{
    int topLeftX = imageSize.width() / 2 - rectangleSize.width() / 2;
    int topLeftY = imageSize.height() / 2 - rectangleSize.height() / 2;
    QPoint topLeft(topLeftX, topLeftY);

    QRect rect(topLeft, rectangleSize);

    QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(bgColor, bgColor, bgColor));

    QPainterPath path;
    path.addRect(rect);

    QPainter paint(&image);
    QBrush brush = QBrush(QColor(fgColor, fgColor, fgColor));
    paint.fillPath(path, brush);

    return FImage(image);
}

FImage::FImage()
    : QImage()
{
}

FImage::FImage(int width, int height)
    : QImage(width, height, QImage::Format_ARGB32_Premultiplied)
{
    fill(Qt::black);
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
