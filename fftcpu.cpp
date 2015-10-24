#include "fftcpu.h"

#include <QTime>

FFTCpu::FFTCpu(FImage *image, QObject *parent)
    : FT(image, parent)
{
    QTime timer;
    timer.start();
    qDebug() << "[CPU] Working on Fast Fourier Transformation...";

    m_fourier = calculateFourier(m_imageData);
    m_magnitude = calculateMagnitude(m_fourier);
    m_phase = calculatePhase(m_fourier);

    qDebug() << "BOOM! Done.";
    qDebug() << "It took" << timer.elapsed() << "msecs";
}

FFTCpu::~FFTCpu()
{
}

// TODO(pvarga): Complex array as an input is not supported yet
Complex *FFTCpu::calculateFourier(float *input, bool inverse)
{
    const int size = m_rows * m_cols;
    const float norm = inverse ? 1.0 / size : 1.0;
    Complex *fourier = new Complex[size];

    if (!isPowerOfTwo(m_rows) || !isPowerOfTwo(m_cols)) {
        qWarning("Image width or height is not power of 2! (%dx%d)", m_cols, m_rows);
        return fourier;
    }

    for (int i = 0; i < size; ++i) {
        fourier[i].real = input[i];
        fourier[i].imag = 0.0;
    }

    for (int i = 0; i < size; i += m_cols)
        fft1D(&fourier[i], (unsigned)m_cols, inverse);

    Complex column[m_rows];
    for (int x = 0; x < m_cols; ++x) {
        for (int y = 0; y < m_rows; ++y) {
            int index = x + y * m_cols;
            column[y] = fourier[index];
        }

        fft1D(&column[0], (unsigned)m_rows, inverse);

        for (int y = 0; y < m_rows; ++y) {
            int index = x + y * m_cols;
            fourier[index] = column[y];
            fourier[index].real *= norm;
            fourier[index].imag *= norm;
        }
    }

    return fourier;
}

void FFTCpu::fft1D(Complex *vector, unsigned n, bool inverse) const
{
    const float dir = inverse ? 1.0 : -1.0;

    revbinPermute(vector, n);
    unsigned ldn = log2(n);

    for (unsigned ldm = 1; ldm <= ldn; ++ldm) {
        unsigned m = pow(2, ldm);
        unsigned mh = m / 2;

        for (unsigned r = 0; r < n; r += m) {
            for (unsigned j = 0; j < mh; ++j) {
                float angle = dir * 2.0 * M_PI * (float)j / (float)m;

                float ereal = cos(angle);
                float eimag = sin(angle);

                Complex v = vector[r + j + mh];
                float vreal = v.real;
                float vimag = v.imag;
                v.real = vreal * ereal - vimag * eimag;
                v.imag = vimag * ereal + vreal * eimag;

                Complex u = vector[r + j];
                vector[r + j].real = u.real + v.real;
                vector[r + j].imag = u.imag + v.imag;
                vector[r + j + mh].real = u.real - v.real;
                vector[r + j + mh].imag = u.imag - v.imag;
            }
        }
    }
}

void FFTCpu::revbinPermute(Complex *vector, unsigned n) const
{
    if (n <= 2)
        return;

    unsigned ldn = log2(n);
    for (unsigned x = 0; x < n - 1; ++x) {
        unsigned r = revbin(x, ldn);
        if (r > x) {
            Complex tmp = vector[x];
            vector[x] = vector[r];
            vector[r] = tmp;
        }
    }
}

inline int FFTCpu::revbin(unsigned x, unsigned ldn) const
{
    unsigned r = 0;
    for (; ldn > 0; --ldn) {
        r = r << 1;
        r = r + (x & 1);
        x = x >> 1;
    }
    return r;
}

inline bool FFTCpu::isPowerOfTwo(unsigned x) const
{
    return ((x != 0) && !(x & (x - 1)));
}
