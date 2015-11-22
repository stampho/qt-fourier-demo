#include "fftgpu.h"

#include "clinfo.h"
#include "gpu.h"
#include <QtMath>
#include <QTime>

FFTGpu::FFTGpu(FImage *image, QObject *parent)
    : FT(image, parent)
    , m_gpu(new GPU(parent))
{
    m_gpu->addProgramMacro(QString("WIDTH=%1").arg(QString::number(m_cols)));
    m_gpu->addProgramMacro(QString("LDWIDTH=%1").arg(QString::number(log2(m_cols))));
    m_gpu->addProgramMacro(QString("HEIGHT=%1").arg(QString::number(m_rows)));
    m_gpu->addProgramMacro(QString("LDHEIGHT=%1").arg(QString::number(log2(m_rows))));
    m_gpu->createKernel(QStringList() << "fft1DRow" << "fft1DCol", QStringLiteral(":/kernels/fft.cl"));
    if (m_gpu->hasError())
        return;

    //qDebug() << CLInfo(m_gpu->getDevice());
    //qDebug() << CLInfo(m_gpu->getKernel(), m_gpu->getDevice());

    QTime timer;
    timer.start();
    qDebug() << "[GPU] Working on Fast Fourier Transformation...";

    m_fourier = calculateFourier(m_imageData);
    m_magnitude = calculateMagnitude(m_fourier);
    m_phase = calculatePhase(m_fourier);

    qDebug() << "BOOM! Done.";
    qDebug() << "It took" << timer.elapsed() << "msecs";
}

FFTGpu::~FFTGpu()
{
}

Complex *FFTGpu::calculateFourier(Complex *input, bool inverse)
{
    const unsigned size = m_cols * m_rows;
    const float dir = inverse ? 1.0 : -1.0;
    const float norm = inverse ? 1.0 / size : 1.0;
    cl_int clError = 0;

    if (!isPowerOfTwo(m_rows) || !isPowerOfTwo(m_cols)) {
        qWarning("Image width or height is not power of 2! (%dx%d)", m_cols, m_rows);
        return new Complex[size];
    }

    cl_float2 *fourierBuffer = new cl_float2[size];
    for (unsigned i = 0; i < size; ++i) {
        fourierBuffer[i].s[0] = input[i].real;
        fourierBuffer[i].s[1] = input[i].imag;
    }

    cl_mem clFourier = m_gpu->setCommonKernelArg<cl_float2>(fourierBuffer, size, 0, "fft1DRow");
    m_gpu->setInputKernelArg<float>(&dir, "fft1DRow");

    cl_uint dim = 1;
    size_t globalWorkGroupSize[] = { (size_t)m_rows, 0, 0 };

    clError |= clEnqueueNDRangeKernel(m_gpu->getCommandQueue(),
                                      m_gpu->getKernel("fft1DRow"),
                                      dim,
                                      0,
                                      globalWorkGroupSize,
                                      0, 0, 0, 0);

    clError |= clFinish(m_gpu->getCommandQueue());

    if (clError != CL_SUCCESS) {
        qWarning("[ERROR] Unable to execute OpenCL Kernel: %d", clError);
        return new Complex[size];
    }

    m_gpu->release("fft1DRow");
    Complex *fourier = new Complex[size];
    for (unsigned i = 0; i < size; ++i)
        fourier[i] = Complex(fourierBuffer[i].s[0], fourierBuffer[i].s[1]);

    delete fourierBuffer;

    // TODO: Replace this with fft1DCol kernel!
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

void FFTGpu::fft1D(Complex *vector, unsigned n, bool inverse) const
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

void FFTGpu::revbinPermute(Complex *vector, unsigned n) const
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

inline int FFTGpu::revbin(unsigned x, unsigned ldn) const
{
    unsigned r = 0;
    for (; ldn > 0; --ldn) {
        r = r << 1;
        r = r + (x & 1);
        x = x >> 1;
    }
    return r;
}

inline bool FFTGpu::isPowerOfTwo(unsigned x) const
{
    return ((x != 0) && !(x & (x - 1)));
}
