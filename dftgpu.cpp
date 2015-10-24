#include "dftgpu.h"

#include "clinfo.h"
#include "gpu.h"
#include <QTime>

DFTGpu::DFTGpu(FImage *image, QObject *parent)
    : FT(image, parent)
    , m_gpu(new GPU(parent))
{
    m_gpu->createKernel(QStringLiteral("dft"), QStringLiteral(":/kernels/dft.cl"));
    if (m_gpu->hasError())
        return;

    //qDebug() << CLInfo(m_gpu->getDevice());
    //qDebug() << CLInfo(m_gpu->getKernel(), m_gpu->getDevice());

    QTime timer;
    timer.start();
    qDebug() << "[GPU] Working on Discrete Fourier Transformation...";

    m_fourier = calculateFourier(m_imageData);
    m_magnitude = calculateMagnitude(m_fourier);
    m_phase = calculatePhase(m_fourier);

    qDebug() << "BOOM! Done.";
    qDebug() << "It took" << timer.elapsed() << "msecs";
}

DFTGpu::~DFTGpu()
{
}

// TODO(pvarga): Complex array as an input is not supported yet
Complex *DFTGpu::calculateFourier(float *input, bool inverse)
{
    const unsigned size = m_rows * m_cols;
    const float dir = inverse ? 1.0 : -1.0;
    const float norm = inverse ? 1.0 / size : 1.0;

    m_gpu->setInputKernelArg<float>(input, size);
    m_gpu->setInputKernelArg<float>(&dir);
    m_gpu->setInputKernelArg<float>(&norm);

    cl_float2 *output = new cl_float2[size];
    m_gpu->setOutputKernelArg<cl_float2>(output, size);

    cl_uint dim = 2;
    size_t globalWorkGroupSize[] = { (size_t)m_cols, (size_t)m_rows, 0 };

    cl_int clError = 0;
    clError |= clEnqueueNDRangeKernel(m_gpu->getCommandQueue(),
                                      m_gpu->getKernel(),
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

    Complex *result = new Complex[size];
    for (unsigned i = 0; i < size; ++i)
        result[i] = Complex(output[i].s[0], output[i].s[1]);

    return result;
}

