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
    return FImage(image, fileName);
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

FImage FImage::rectangle(const QString &rectCode, const QSize &bgSize)
{
    if (!FImage::isRectCode(rectCode))
        return FImage();

    QStringList values = rectCode.split("-");
    int contentWidth = values[3].toInt();
    int contentHeight = values[4].toInt();
    int bgColor = values[5].toInt();
    int fgColor = values[6].toInt();

    return FImage::rectangle(bgSize, QSize(contentWidth, contentHeight), bgColor, fgColor);
}

FImage FImage::rectangle(const QSize &bgSize, const QSize &contentSize)
{
    return FImage::rectangle(bgSize, contentSize, 0, 255);
}

FImage FImage::rectangle(const QSize &bgSize, const QSize &contentSize, unsigned bgColor, unsigned fgColor)
{
    int topLeftX = bgSize.width() / 2 - contentSize.width() / 2;
    int topLeftY = bgSize.height() / 2 - contentSize.height() / 2;
    QPoint topLeft(topLeftX, topLeftY);

    QRect rect(topLeft, contentSize);

    QImage image(bgSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(bgColor, bgColor, bgColor));

    QPainterPath path;
    path.addRect(rect);

    QPainter paint(&image);
    QBrush brush = QBrush(QColor(fgColor, fgColor, fgColor));
    paint.fillPath(path, brush);

    QStringList code;

    code.append("rect");
    code.append(QString::number(bgSize.width()));
    code.append(QString::number(bgSize.height()));
    code.append(QString::number(contentSize.width()));
    code.append(QString::number(contentSize.height()));
    code.append(QString::number(bgColor));
    code.append(QString::number(fgColor));

    return FImage(image, code.join("-"));
}

FImage::FImage()
    : QImage()
    , m_id(QStringLiteral("rect-0-0-0-0-0-0"))
{
}

FImage::FImage(int width, int height)
    : QImage(width, height, QImage::Format_ARGB32_Premultiplied)
    , m_id(QStringLiteral("rect-%1-%2-0-0-0-0").arg(width).arg(height))
{
    fill(Qt::black);
}

FImage::FImage(const QImage &image, const QString &id)
    : QImage(image)
    , m_id(id)
{
    if (!isGrayscale())
        convertToGrayscale();

    fillData();
}

FImage::FImage(uchar *data, int width, int height, const QString &id)
    : QImage(width, height, QImage::Format_ARGB32_Premultiplied)
    , m_id(id)
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

QString FImage::id() const
{
    return m_id;
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
