#include "dftgpu.h"

#include "clinfo.h"
#include "gpu.h"

DFTGpu::DFTGpu(FImage *image, QObject *parent)
    : FT(image, parent)
    , m_gpu(new GPU(parent))
{
    m_gpu->createKernel(QStringList() << "dft", QStringLiteral(":/kernels/dft.cl"));
    if (m_gpu->hasError())
        return;

    //qDebug() << CLInfo(m_gpu->getKernel(), m_gpu->getDevice());
}

DFTGpu::~DFTGpu()
{
}

Complex *DFTGpu::calculateFourier(Complex *input, bool inverse)
{
    const unsigned size = m_cols * m_rows;
    const float dir = inverse ? 1.0 : -1.0;
    const float norm = inverse ? 1.0 / size : 1.0;

    cl_float2 *fInput = new cl_float2[size];
    for (unsigned i = 0; i < size; ++i) {
        fInput[i].s[0] = input[i].real;
        fInput[i].s[1] = input[i].imag;
    }

    const unsigned cols = m_cols;
    const unsigned rows = m_rows;

    m_gpu->setInputKernelArg<cl_float2>(fInput, size);
    m_gpu->setInputKernelArg<unsigned>(&cols);
    m_gpu->setInputKernelArg<unsigned>(&rows);
    m_gpu->setInputKernelArg<float>(&dir);
    m_gpu->setInputKernelArg<float>(&norm);

    cl_float2 *output = new cl_float2[size];
    m_gpu->setOutputKernelArg<cl_float2>(output, size);

    cl_uint dim = 2;
    size_t globalWorkGroupSize[] = { 0, 0, 0 };
    m_gpu->preferredWorkGroupSize(globalWorkGroupSize, m_cols, m_rows, 0);
    size_t prefWidth = globalWorkGroupSize[0];
    size_t prefHeight = globalWorkGroupSize[1];

    cl_int clError = 0;
    unsigned iRuns = qCeil((float)m_cols / (float)prefWidth);
    unsigned jRuns = qCeil((float)m_rows / (float)prefHeight);

    for (unsigned i = 0; i < iRuns; ++i) {
        for (unsigned j = 0; j < jRuns; ++j) {
            //qDebug("[%u, %u] %u, %u", i, j, i * prefWidth, j * prefHeight);
            size_t offset[] = { i * prefWidth, j * prefHeight, 0 };
            clError |= clEnqueueNDRangeKernel(m_gpu->getCommandQueue(),
                                              m_gpu->getKernel(),
                                              dim,
                                              offset,
                                              globalWorkGroupSize,
                                              0, 0, 0, 0);
        }
    }
    clError |= clFinish(m_gpu->getCommandQueue());

    if (clError != CL_SUCCESS) {
        qWarning("[ERROR] Unable to execute OpenCL Kernel: %d", clError);
        return new Complex[size];
    }

    m_gpu->release();
    delete fInput;

    Complex *result = new Complex[size];
    for (unsigned i = 0; i < size; ++i)
        result[i] = Complex(output[i].s[0], output[i].s[1]);

    return result;
}

