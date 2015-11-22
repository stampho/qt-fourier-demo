#include "gpu.h"

#include <QFile>
#include "clinfo.h"

GPU::GPU(QObject *parent)
    : QObject(parent)
    , m_clError(CL_SUCCESS)
    , m_clPlatform(0)
    , m_clDevice(0)
    , m_clContext(0)
    , m_clCommandQueue(0)
    , m_clProgram(0)
{
    initPlatforms();
    initDevices();
    initContext();
    initCommandQueue();
}

GPU::~GPU()
{
    clReleaseProgram(m_clProgram);
    Q_FOREACH (cl_kernel clKernel, m_clKernels.values())
        clReleaseKernel(clKernel);
    clReleaseCommandQueue(m_clCommandQueue);
    clReleaseContext(m_clContext);
}

void GPU::initPlatforms(unsigned platformId)
{
    cl_uint platformIdCount = 0;
    m_clError = clGetPlatformIDs(0, 0, &platformIdCount);
    CHECK_CL_ERROR("[ERROR] Unable to initialize OpenCL Platform");

    m_clPlatformList = QVector<cl_platform_id>(platformIdCount);
    m_clError = clGetPlatformIDs(platformIdCount, m_clPlatformList.data(), 0);
    CHECK_CL_ERROR("[ERROR] Unable to initialize OpenCL Platform");

    if (platformId >= platformIdCount) {
        qWarning("[WARNING] Invalid OpenCL Platform Id: %d. Fallback to 0.", platformId);
        platformId = 0;
    }

    m_clPlatform = m_clPlatformList[platformId];
}

void GPU::initDevices(unsigned deviceId)
{
    if (!m_clPlatform)
        return;

    cl_uint deviceIdCount = 0;
    m_clError = clGetDeviceIDs(m_clPlatform, CL_DEVICE_TYPE_ALL, 0, 0, &deviceIdCount);
    CHECK_CL_ERROR("[ERROR] Unable to initialize OpenCL Device");

    m_clDeviceList = QVector<cl_device_id>(deviceIdCount);
    m_clError = clGetDeviceIDs(m_clPlatform, CL_DEVICE_TYPE_ALL, deviceIdCount, m_clDeviceList.data(), 0);
    CHECK_CL_ERROR("[ERROR] Unable to initialize OpenCL Device");

    if (deviceId >= deviceIdCount) {
        qWarning("[WARNING] Invalid OpenCL Device Id: %d. Fallback to 0.", deviceId);
        deviceId = 0;
    }

    m_clDevice = m_clDeviceList[deviceId];
}

void GPU::initContext()
{
    if (!m_clDevice)
        return;

    m_clContext = clCreateContext(0, 1, &m_clDevice, 0, 0, &m_clError);
    CHECK_CL_ERROR("[ERROR] Unable to initialize OpenCL Context");
}

void GPU::initCommandQueue()
{
    if (!m_clDevice || !m_clContext)
        return;

    m_clCommandQueue = clCreateCommandQueue(m_clContext, m_clDevice, 0, &m_clError);
    CHECK_CL_ERROR("[ERROR] Unable to initialize OpenCL Command Queue");
}

void GPU::preferredWorkGroupSize(size_t size[3], int cols, int rows, int depth) const
{
    // TODO(pvarga): 3D work group is unsupported
    Q_UNUSED(depth);

    CLInfo kernelInfo(m_clKernels.first(), m_clDevice);
    size_t pref = kernelInfo[CL_KERNEL_WORK_GROUP_SIZE].toInt();
    size[0] = pref > (size_t)cols ? (size_t)cols : pref;
    size[1] = pref > (size_t)rows ? (size_t)rows : pref;
}

void GPU::addProgramMacro(const QString &macro)
{
    m_programMacros.append(macro);
}

void GPU::createKernel(QStringList kernelIds, const QString &kernelPath)
{
    if (hasError())
        return;

    createProgram(kernelPath);
    CHECK_CL_ERROR("[ERROR] Unable to create OpenCL Program");

    buildProgram();
    CHECK_CL_ERROR("[ERROR] Unable to build OpenCL Program");

    Q_FOREACH (QString kernelId, kernelIds) {
        cl_kernel clKernel = clCreateKernel(m_clProgram, kernelId.toLocal8Bit().data(), &m_clError);
        CHECK_CL_ERROR("[ERROR] Unable to create OpenCL Kernel");
        m_clKernels.insert(kernelId, clKernel);
        m_kernelArgCounter.insert(kernelId, 0);
        m_kernelInputArgs.insert(kernelId, QVector<cl_mem>());
        m_kernelOutputArgs.insert(kernelId, QVector<QPair<cl_mem, void *> >());
    }
}

void GPU::createProgram(const QString &kernelPath)
{
    if (!m_clContext)
        return;

    QFile kernelFile(kernelPath);
    if (!kernelFile.open(QFile::ReadOnly | QFile::Text)) {
        m_clError = -100;
        qWarning("[ERROR] Failed to load kernel source file: %s", kernelFile.fileName().toLocal8Bit().data());
        return;
    }
    QTextStream in(&kernelFile);
    QString kernelSource = in.readAll();
    kernelFile.close();

    size_t length = kernelSource.length();
    char src[length];
    qstrcpy(src, kernelSource.toLocal8Bit().data());

    size_t lengths[1] = { length };
    const char *sources[1] = { src };
    m_clProgram = clCreateProgramWithSource(m_clContext, 1, sources, lengths, &m_clError);
}

void GPU::buildProgram()
{
    if (!m_clDevice || !m_clProgram)
        return;

    QString options;
    for (int i = 0; i < m_programMacros.size(); ++i)
        options = QString("%1 -D%2").arg(options).arg(m_programMacros[i]);

    m_clError = clBuildProgram(m_clProgram, 0, 0, options.toLocal8Bit().data(), 0, 0);
    if (m_clError != CL_SUCCESS) {
        size_t len;
        clGetProgramBuildInfo(m_clProgram, m_clDevice, CL_PROGRAM_BUILD_LOG, 0, 0, &len);

        char buffer[len];
        clGetProgramBuildInfo(m_clProgram, m_clDevice, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, 0);
        qDebug() << buffer;
    }
}

void GPU::releaseInputArgs(const QString &kernelId)
{
    QVector<cl_mem> &inputArgs = m_kernelInputArgs[kernelId];

    for (int i = 0; i < inputArgs.size(); ++i) {
        cl_mem clInput = inputArgs[i];
        if (clInput)
            clReleaseMemObject(clInput);
    }

    inputArgs.clear();
}

void GPU::releaseOutputArgs(const QString &kernelId)
{
    QVector<QPair<cl_mem, void *> > &outputArgs = m_kernelOutputArgs[kernelId];

    for (int i = 0; i < outputArgs.size(); ++i) {
        cl_mem clOutput = outputArgs[i].first;
        void *output = outputArgs[i].second;

        size_t size = 0;
        m_clError = clGetMemObjectInfo(clOutput, CL_MEM_SIZE, sizeof(size), &size, 0);
        CHECK_CL_ERROR("[ERROR] Unable to acquire OpenCL Output Buffer size");

        m_clError = clEnqueueReadBuffer(m_clCommandQueue, clOutput, CL_TRUE, 0, size, output, 0, 0, 0);
        CHECK_CL_ERROR("[ERROR] Unable to read from OpenCL Output Buffer");

        clReleaseMemObject(clOutput);
    }

    outputArgs.clear();
}

void GPU::release(const QString &kernelId)
{
    if (!kernelId.isNull() && !kernelId.isEmpty()) {
        releaseInputArgs(kernelId);
        releaseOutputArgs(kernelId);
        m_kernelArgCounter[kernelId] = 0;
        return;
    }

    Q_FOREACH (QString id, m_clKernels.keys()) {
        releaseInputArgs(id);
        releaseOutputArgs(id);
        m_kernelArgCounter[id] = 0;
    }
}

bool GPU::hasError() const
{
    return m_clError != CL_SUCCESS;
}

cl_device_id GPU::getDevice() const
{
    return m_clDevice;
}

cl_context GPU::getContext() const
{
    return m_clContext;
}

cl_command_queue GPU::getCommandQueue() const
{
    return m_clCommandQueue;
}

cl_kernel GPU::getKernel(const QString &kernelId) const
{
    if (kernelId.isNull() || kernelId.isEmpty())
        return m_clKernels.first();

    return m_clKernels[kernelId];
}

