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
    static FImage rectangle(const QString &, const QSize &);
    static FImage rectangle(const QSize &, const QSize &);
    static FImage rectangle(const QSize &, const QSize &, unsigned, unsigned);

    FImage();
    FImage(int, int);
    FImage(uchar *, int, int, const QString &id = QString());
    QVector<uchar> data() const;
    QString id() const;

private:
    FImage(const QImage &, const QString &id = QString());
    void convertToGrayscale();
    void fillData();
    void updateImage();

    QVector<uchar> m_data;
    QString m_id;
};

QDebug operator<<(QDebug, const FImage &);

#endif // FIMAGE_H
