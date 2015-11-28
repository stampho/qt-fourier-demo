#include "fftcpu.h"

FFTCpu::FFTCpu(FImage *image, QObject *parent)
    : FT(image, parent)
{
}

FFTCpu::~FFTCpu()
{
}

Complex *FFTCpu::calculateFourier(Complex *input, bool inverse)
{
    const int size = m_rows * m_cols;
    const float norm = inverse ? 1.0 / size : 1.0;
    Complex *fourier = new Complex[size];

    if (!IS_POWER_OF_TWO(m_rows) || !IS_POWER_OF_TWO(m_cols)) {
        qWarning("Image width or height is not power of 2! (%dx%d)", m_cols, m_rows);
        return fourier;
    }

    memcpy(fourier, input, size * sizeof(Complex));

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
        unsigned m = qPow(2, ldm);
        unsigned mh = m / 2;

        for (unsigned r = 0; r < n; r += m) {
            for (unsigned j = 0; j < mh; ++j) {
                float angle = dir * 2.0 * (float)M_PI * (float)j / (float)m;

                float ereal = qCos(angle);
                float eimag = qSin(angle);

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

    while (ldn-- > 0) {
        r = r << 1;
        r = r + (x & 1);
        x = x >> 1;
    }

    return r;
}
