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

    m_gpu->setCommonKernelArg<cl_float2>(fourierBuffer, size, clFourier, "fft1DCol");
    m_gpu->setInputKernelArg<float>(&dir, "fft1DCol");
    m_gpu->setInputKernelArg<float>(&norm, "fft1DCol");

    globalWorkGroupSize[0] = (size_t)m_cols;
    clError |= clEnqueueNDRangeKernel(m_gpu->getCommandQueue(),
                                      m_gpu->getKernel("fft1DCol"),
                                      dim,
                                      0,
                                      globalWorkGroupSize,
                                      0, 0, 0, 0);
    clError |= clFinish(m_gpu->getCommandQueue());

    if (clError != CL_SUCCESS) {
        qWarning("[ERROR] Unable to execute OpenCL Kernel: %d", clError);
        return new Complex[size];
    }

    m_gpu->release();

    Complex *fourier = new Complex[size];
    for (unsigned i = 0; i < size; ++i)
        fourier[i] = Complex(fourierBuffer[i].s[0], fourierBuffer[i].s[1]);
    delete fourierBuffer;

    return fourier;
}
