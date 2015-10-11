#include "dftgpu.h"

#include <CL/cl.h>
#include <QTime>

DFTGpu::DFTGpu(FImage *image, QObject *parent)
    : FT(image, parent)
{
    bool success = initOpenCL();
    if (!success) {
        qWarning("OpenCL initialization has failed!");
        return;
    }

    // TODO(pvarga): copy imageData into GPU memory

    QTime timer;
    timer.start();
    qDebug() << "[GPU] Working on Fourier Transformation...";

    m_fourier = calculateFourier(m_imageData);
    m_magnitude = calculateMagnitude(m_fourier);
    m_phase = calculatePhase(m_fourier);

    qDebug() << "BOOM! Done.";
    qDebug() << "It took" << timer.elapsed() << "msecs";
}

DFTGpu::~DFTGpu()
{
}

Complex *DFTGpu::calculateFourier(float *input, bool inverse)
{
    Q_UNUSED(input);
    Q_UNUSED(inverse);

    Complex *fourier = new Complex[m_rows * m_cols];

    // TODO(pvarga): Implement OpenCL Kernel!
    for (int i = 0; i < m_rows * m_cols; ++i)
        fourier[i] = Complex(0, 0);

    return fourier;

}

bool DFTGpu::initOpenCL()
{
    cl_int error;

    cl_uint platformIdCount = 0;
    error = clGetPlatformIDs(0, 0, &platformIdCount);
    if (error != CL_SUCCESS) {
        qWarning("Error while initializing OpenCL Platform: %d", error);
        return false;
    }

    QVector<cl_platform_id> platformIds(platformIdCount);
    error = clGetPlatformIDs(platformIdCount, platformIds.data(), 0);
    if (error != CL_SUCCESS) {
        qWarning("Error while initializing OpenCL Platform: %d", error);
        return false;
    }

    cl_uint deviceIdCount = 0;
    error = clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_ALL, 0, 0, &deviceIdCount);
    if (error != CL_SUCCESS) {
        qWarning("Error while initializing OpenCL Device: %d", error);
        return false;
    }

    QVector<cl_device_id> deviceIds(deviceIdCount);
    error = clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_ALL, deviceIdCount, deviceIds.data(), 0);
    if (error != CL_SUCCESS) {
        qWarning("Error while initializing OpenCL Device: %d", error);
        return false;
    }

    const cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM,
                                                        reinterpret_cast<cl_context_properties>(platformIds[0]),
                                                        0, 0 };
    cl_context context = clCreateContext(contextProperties, deviceIdCount, deviceIds.data(), 0, 0, &error);
    if (error != CL_SUCCESS) {
        qWarning("Error while initializing OpenCL Context: %d", error);
        return false;
    }

    return true;
}
