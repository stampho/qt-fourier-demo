#include "dftcpu.h"

#include <QTime>

DFTCpu::DFTCpu(FImage *image, QObject *parent)
    : FT(image, parent)
{
    QTime timer;
    timer.start();
    qDebug() << "[CPU] Working on Fourier Transformation...";

    m_fourier = calculateFourier(m_imageData);
    m_magnitude = calculateMagnitude(m_fourier);
    m_phase = calculatePhase(m_fourier);

    qDebug() << "BOOM! Done.";
    qDebug() << "It took" << timer.elapsed() << "msecs";
}

DFTCpu::~DFTCpu()
{
}

// TODO(pvarga): Complex array as an input is not supported yet
Complex *DFTCpu::calculateFourier(float *input, bool inverse)
{
    const float dir = inverse ? 1.0 : -1.0;
    Complex *fourier = new Complex[m_rows * m_cols];

    for (int v = 0; v < m_rows; ++v) {
        for (int u = 0; u < m_cols; ++u) {
            float sumReal = 0.0;
            float sumImag = 0.0;

            for (int y = 0; y < m_rows; ++y) {
                for (int x = 0; x < m_cols; ++x) {
                    float f = (float)input[x + y * m_cols];
                    float a = (float)u * (float)x / (float)m_cols;
                    float b = (float)v * (float)y / (float)m_rows;
                    float angle = dir * 2.0 * M_PI * (a + b);

                    sumReal += (float)f * cos(angle);
                    sumImag += (float)f * sin(angle);
                }
            }

            int index = u + v * m_cols;
            fourier[index].real = sumReal;
            fourier[index].imag = sumImag;
        }
    }

    return fourier;
}

