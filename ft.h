#ifndef FT_H
#define FT_H

#include <QDebug>
#include <QObject>

class FImage;

struct Complex {
    float real;
    float imag;

    Complex();
    Complex(float, float);

    inline float magnitude() const;
    inline float phase() const;
};

QDebug operator<<(QDebug, const Complex &);

class FT : public QObject {
    Q_OBJECT
public:
    explicit FT(FImage *image, QObject *parent = 0);
    virtual ~FT();

    FImage magnitudeImage() const;
    FImage reconstructFromMagnitude();
    FImage phaseImage() const;
    FImage reconstructFromPhase();
    FImage reconstructOriginalImage();

protected:
    virtual Complex *calculateFourier(Complex *input, bool inverse) = 0;
    float *calculateMagnitude(Complex *) const;
    float *calculatePhase(Complex *) const;

    template <typename T> T *fftshift(const T *input, bool inverse = false) const;

    int m_rows;
    int m_cols;
    Complex *m_imageData;

    Complex *m_fourier;
    float *m_magnitude;
    float *m_phase;
};

#endif // FT_H
