#ifndef FT_H
#define FT_H

#include <QDebug>
#include <QObject>

class FImage;

struct Complex {
    float real;
    float imag;

    inline float magnitude() const;
    inline float phase() const;
};

QDebug operator<<(QDebug, const Complex &);

class FT : public QObject {
    Q_OBJECT
public:
    explicit FT(FImage *image, QObject *parent = 0);
    ~FT();

    FImage magnitudeImage() const;
    FImage phaseImage() const;

private:
    Complex *calculateFourier(float *input, bool inverse = false);
    float *calculateMagnitude(Complex *);
    float *calculatePhase(Complex *);
    float *fftshift(float *input, bool inverse = false);

    template <typename T> T *fftshift(const T *input, bool inverse = false) const;

    int m_rows;
    int m_cols;
    float *m_values;

    Complex *m_fourier;
    float *m_magnitude;
    float *m_phase;
};

#endif // FT_H
