#include "dftgpu.h"

#include <QTime>

DFTGpu::DFTGpu(FImage *image, QObject *parent)
    : FT(image, parent)
    , GPU(parent)
{
    createKernel(QStringLiteral("dft"), QStringLiteral(":/kernels/dft.cl"));
    if (hasError())
        return;

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
Complex *DFTGpu::calculateFourier(float *input, bool inverse) const
{
    Complex *fourier = new Complex[m_rows * m_cols];
    cl_int error;

    cl_mem clInput = clCreateBuffer(getContext(),
                                    CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    sizeof(float) * m_rows * m_cols,
                                    input,
                                    &error);
    if (error != CL_SUCCESS) {
        qWarning("Error while creating OpenCL Buffer: %d", error);
        return fourier;
    }

    cl_mem clOutReal = clCreateBuffer(getContext(),
                                      CL_MEM_WRITE_ONLY,
                                      sizeof(float) * m_rows * m_cols,
                                      NULL,
                                      &error);
    if (error != CL_SUCCESS) {
        qWarning("Error while creating OpenCL Buffer: %d", error);
        return fourier;
    }

    cl_mem clOutImag = clCreateBuffer(getContext(),
                                      CL_MEM_WRITE_ONLY,
                                      sizeof(float) * m_rows * m_cols,
                                      NULL,
                                      &error);

    if (error != CL_SUCCESS) {
        qWarning("Error while creating OpenCL Buffer: %d", error);
        return fourier;
    }

    int inv = (int)inverse;
    error |= clSetKernelArg(getKernel(), 0, sizeof(cl_mem), (void *) &clInput);
    error |= clSetKernelArg(getKernel(), 1, sizeof(int), (void *) &inv);
    error |= clSetKernelArg(getKernel(), 2, sizeof(cl_mem), (void *) &clOutReal);
    error |= clSetKernelArg(getKernel(), 3, sizeof(cl_mem), (void *) &clOutImag);

    if (error != CL_SUCCESS) {
        qWarning("Error while setting OpenCL Kernel Arguments: %d", error);
        return fourier;
    }

    cl_uint dim = 2;
    size_t globalWorkGroupSize[] = { (size_t)m_cols, (size_t)m_rows, 0 };
    error = clEnqueueNDRangeKernel(getCommandQueue(), getKernel(), dim, 0, globalWorkGroupSize, 0, 0, 0, 0);
    clFinish(getCommandQueue());

    if (error != CL_SUCCESS) {
        qWarning("Error while executing OpenCL Kernel: %d", error);
        return fourier;
    }

    float *real = new float[m_rows * m_cols];
    float *imag = new float[m_rows * m_cols];
    clEnqueueReadBuffer(getCommandQueue(), clOutReal, CL_TRUE, 0, sizeof(float) * m_rows * m_cols, real, 0, 0, 0);
    clEnqueueReadBuffer(getCommandQueue(), clOutImag, CL_TRUE, 0, sizeof(float) * m_rows * m_cols, imag, 0, 0, 0);

    for (int i = 0; i < m_rows * m_cols; ++i)
        fourier[i] = Complex(real[i], imag[i]);

    clReleaseMemObject(clInput);
    clReleaseMemObject(clOutReal);
    clReleaseMemObject(clOutImag);

    return fourier;
}

