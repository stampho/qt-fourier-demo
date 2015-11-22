#ifndef FIMAGE_H
#define FIMAGE_H

#include <QDebug>
#include <QImage>
#include <QVector>

class QSize;
class QString;

class FImage : public QImage {
public:
    static bool isRectCode(const QString &);

    static FImage createFromFile(const QString &);
    static FImage rectangle(const QString &);
    static FImage rectangle(const QSize &, const QSize &);
    static FImage rectangle(const QSize &, const QSize &, unsigned, unsigned);

    FImage();
    FImage(int, int);
    FImage(uchar *, int, int);
    QVector<uchar> data() const;

private:
    FImage(const QImage &);
    void convertToGrayscale();
    void fillData();
    void updateImage();

    QVector<uchar> m_data;
};

QDebug operator<<(QDebug, const FImage &);

#endif // FIMAGE_H
