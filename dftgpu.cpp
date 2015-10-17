#include "dftgpu.h"

#include <QFile>
#include <QTime>

DFTGpu::DFTGpu(FImage *image, QObject *parent)
    : FT(image, parent)
    , m_clDevice(0)
    , m_clContext(0)
    , m_clQueue(0)
    , m_clKernel(0)
{
    bool success = false;

    success = initOpenCL();
    if (!success) {
        qWarning("OpenCL initialization has failed!");
        return;
    }

    success = createKernel(QStringLiteral("dft"), QStringLiteral(":/kernels/dft.cl"));
    if (!success) {
        qWarning("Creating OpenCL Kernel has failed!");
        return;
    }

    //printCLInfo();

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
    Q_UNUSED(inverse);

    Complex *fourier = new Complex[m_rows * m_cols];
    cl_int error;

    cl_mem clInput = clCreateBuffer(m_clContext,
                                    CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    sizeof(float) * m_rows * m_cols,
                                    input,
                                    &error);
    if (error != CL_SUCCESS) {
        qWarning("Error while creating OpenCL Buffer: %d", error);
        return fourier;
    }

    cl_mem clOutReal = clCreateBuffer(m_clContext,
                                      CL_MEM_WRITE_ONLY,
                                      sizeof(float) * m_rows * m_cols,
                                      NULL,
                                      &error);
    if (error != CL_SUCCESS) {
        qWarning("Error while creating OpenCL Buffer: %d", error);
        return fourier;
    }

    cl_mem clOutImag = clCreateBuffer(m_clContext,
                                      CL_MEM_WRITE_ONLY,
                                      sizeof(float) * m_rows * m_cols,
                                      NULL,
                                      &error);

    if (error != CL_SUCCESS) {
        qWarning("Error while creating OpenCL Buffer: %d", error);
        return fourier;
    }

    error = clEnqueueWriteBuffer(m_clQueue, clInput, CL_TRUE, 0, sizeof(float) * m_rows * m_cols, input, 0, 0, 0);
    if (error != CL_SUCCESS) {
        qWarning("Error while writing OpenCL Buffer: %d", error);
        return fourier;
    }

    error |= clSetKernelArg(m_clKernel, 0, sizeof(cl_mem), (void *) &clInput);
    error |= clSetKernelArg(m_clKernel, 1, sizeof(cl_mem), (void *) &clOutReal);
    error |= clSetKernelArg(m_clKernel, 2, sizeof(cl_mem), (void *) &clOutImag);

    if (error != CL_SUCCESS) {
        qWarning("Error while setting OpenCL Kernel Arguments: %d", error);
        return fourier;
    }

    cl_uint dim = 2;
    size_t globalWorkGroupSize[] = { (size_t)m_cols, (size_t)m_rows, 0 };
    error = clEnqueueNDRangeKernel(m_clQueue, m_clKernel, dim, 0, globalWorkGroupSize, 0, 0, 0, 0);
    clFinish(m_clQueue);

    if (error != CL_SUCCESS) {
        qWarning("Error while executing OpenCL Kernel: %d", error);
        return fourier;
    }

    float *real = new float[m_rows * m_cols];
    float *imag = new float[m_rows * m_cols];
    clEnqueueReadBuffer(m_clQueue, clOutReal, CL_TRUE, 0, sizeof(float) * m_rows * m_cols, real, 0, 0, 0);
    clEnqueueReadBuffer(m_clQueue, clOutImag, CL_TRUE, 0, sizeof(float) * m_rows * m_cols, imag, 0, 0, 0);

    for (int i = 0; i < m_rows * m_cols; ++i)
        fourier[i] = Complex(real[i], imag[i]);

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
    error = clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_GPU, 0, 0, &deviceIdCount);
    if (error != CL_SUCCESS) {
        qWarning("Error while initializing OpenCL Device: %d", error);
        return false;
    }

    QVector<cl_device_id> deviceIds(deviceIdCount);
    error = clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_GPU, deviceIdCount, deviceIds.data(), 0);
    if (error != CL_SUCCESS) {
        qWarning("Error while initializing OpenCL Device: %d", error);
        return false;
    }

    m_clDevice = deviceIds[0];

    m_clContext = clCreateContext(0, 1, &m_clDevice, 0, 0, &error);
    if (error != CL_SUCCESS) {
        qWarning("Error while initializing OpenCL Context: %d", error);
        return false;
    }

    m_clQueue = clCreateCommandQueue(m_clContext, m_clDevice, 0, &error);
    if (error != CL_SUCCESS) {
        qWarning("Error while initializing OpenCL Command Queue: %d", error);
        return false;
    }

    return true;
}

bool DFTGpu::createKernel(const QString &kernelId, const QString &kernelPath)
{
    QFile kernelFile(kernelPath);
    if (!kernelFile.open(QFile::ReadOnly | QFile::Text)) {
        qWarning("Failed to load kernel source file: %s", kernelFile.fileName().toLocal8Bit().data());
        return false;
    }
    QTextStream in(&kernelFile);
    QString kernelSource = in.readAll();

    cl_int clError;

    const char *sources[1] = { kernelSource.toLocal8Bit().data() };
    size_t lengths[1] = { (size_t)kernelSource.length() };
    cl_program program = clCreateProgramWithSource(m_clContext, 1, sources, lengths, &clError);
    if (clError != CL_SUCCESS || !program) {
        qWarning("Error while creating OpenCL Program: %d", clError);
        return false;
    }

    clError = clBuildProgram(program, 0, 0, 0, 0, 0);
    if (clError != CL_SUCCESS) {
        size_t len;
        char buffer[2048];

        clGetProgramBuildInfo(program, m_clDevice, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        qDebug() << buffer;

        qWarning("Error while building OpenCL Program: %d", clError);
        return false;
    }

    m_clKernel = clCreateKernel(program, kernelId.toLocal8Bit().data(), &clError);
    if (clError != CL_SUCCESS) {
        qWarning("Error while creating OpenCL Kernel: %d", clError);
        return false;
    }

    return true;
}

/* TODO(pvarga): This method should be changed to collect OpenCL information (as much as possible),
 * convert them into QString and store strings in a QMap. It is necessary to show information with
 * GUI. This funcion is currently used for debugging only. Error handling is not implemented.
 */
void DFTGpu::printCLInfo() const
{
    if (!m_clDevice)
        return;

    char buffer[1024];
    clGetDeviceInfo(m_clDevice, CL_DEVICE_NAME, sizeof(buffer), buffer, NULL);
    qDebug() << "[Device] Name:" << buffer;
    clGetDeviceInfo(m_clDevice, CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL);
    qDebug() << "[Device] Vendor:" << buffer;
    clGetDeviceInfo(m_clDevice, CL_DEVICE_VERSION, sizeof(buffer), buffer, NULL);
    qDebug() << "[Device] Device Version:" << buffer;
    clGetDeviceInfo(m_clDevice, CL_DRIVER_VERSION, sizeof(buffer), buffer, NULL);
    qDebug() << "[Device] Driver Version:" << buffer;

    cl_uint buf_uint;
    cl_ulong buf_ulong;
    clGetDeviceInfo(m_clDevice, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(buf_uint), &buf_uint, NULL);
    qDebug() << "[Device] Compute Units:" << buf_uint;
    clGetDeviceInfo(m_clDevice, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(buf_uint), &buf_uint, NULL);
    qDebug() << "[Device] Clock Frequency:" << buf_uint;
    clGetDeviceInfo(m_clDevice, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(buf_ulong), &buf_ulong, NULL);
    qDebug() << "[Device] Global Memory:" << buf_ulong;

    // FIXME(pvarga): Number of the supported dimensions may vary.
    size_t workItemSizes[3];
    clGetDeviceInfo(m_clDevice, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(workItemSizes), workItemSizes, NULL);
    qDebug("[Device] Max work items: %d %d %d", workItemSizes[0], workItemSizes[1], workItemSizes[2]);

    if (!m_clKernel)
        return;

    size_t kernelWorkGroupSize;
    clGetKernelWorkGroupInfo(m_clKernel, m_clDevice, CL_KERNEL_WORK_GROUP_SIZE, sizeof(kernelWorkGroupSize), &kernelWorkGroupSize, NULL);
    qDebug("[Kernel] Max Work Group Size: %d", kernelWorkGroupSize);
}
