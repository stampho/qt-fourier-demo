#ifndef GPU_H
#define GPU_H

#include <CL/cl.h>
#include <QDebug>
#include <QMap>
#include <QObject>
#include <QVector>

#define CHECK_CL_ERROR(message) \
    if (m_clError != CL_SUCCESS) { \
        qWarning("%s: %d", message, m_clError); \
        return; \
    }

#define CHECK_CL_ERROR_RET(message) \
    if (m_clError != CL_SUCCESS) { \
        qWarning("%s: %d", message, m_clError); \
        return 0; \
    }


class GPU : public QObject {
    Q_OBJECT
public:
    explicit GPU(QObject *parent = 0);
    virtual ~GPU();

    void preferredWorkGroupSize(size_t size[3], int, int, int) const;
    void addProgramMacro(const QString &);
    void createKernel(QStringList, const QString &);

    template<typename T>
    void setInputKernelArg(const T *input, QString kernelId = QString())
    {
        if (kernelId.isNull() || kernelId.isEmpty())
            kernelId = m_clKernels.firstKey();

        if (!m_kernelInputArgs.contains(kernelId))
            return;

        cl_kernel clKernel = m_clKernels[kernelId];
        if (!m_clContext || !clKernel)
            return;

        m_clError = clSetKernelArg(clKernel,
                                   m_kernelArgCounter[kernelId]++,
                                   sizeof(T),
                                   (void *) input);
        CHECK_CL_ERROR("[ERROR] Unable to set OpenCL Kernel argument");

        m_kernelInputArgs[kernelId].append(0);
    }

    template<typename T>
    void setInputKernelArg(T *input, unsigned size, QString kernelId = QString())
    {
        if (kernelId.isNull() || kernelId.isEmpty())
            kernelId = m_clKernels.firstKey();

        if (!m_kernelInputArgs.contains(kernelId))
            return;

        cl_kernel clKernel = m_clKernels[kernelId];
        if (!m_clContext || !clKernel || !size)
            return;

        cl_mem clInput = clCreateBuffer(m_clContext,
                                        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                        sizeof(T) * size,
                                        input,
                                        &m_clError);
        CHECK_CL_ERROR("[ERROR] Unable to create OpenCL Input Buffer");

        m_clError = clSetKernelArg(clKernel,
                                   m_kernelArgCounter[kernelId]++,
                                   sizeof(cl_mem),
                                   (void *) &clInput);
        CHECK_CL_ERROR("[ERROR] Unable to set OpenCL Kernel argument");

        m_kernelInputArgs[kernelId].append(clInput);
    }

    template<typename T>
    void setOutputKernelArg(T *output, unsigned size, QString kernelId = QString())
    {
        if (kernelId.isNull() || kernelId.isEmpty())
            kernelId = m_clKernels.firstKey();

        if (!m_kernelOutputArgs.contains(kernelId))
            return;

        cl_kernel clKernel = m_clKernels[kernelId];
        if (!m_clContext || !clKernel || !size)
            return;

        cl_mem clOutput = clCreateBuffer(m_clContext,
                                         CL_MEM_WRITE_ONLY,
                                         sizeof(T) * size,
                                         0,
                                         &m_clError);
        CHECK_CL_ERROR("[ERROR] Unable to create OpenCL Output Buffer");

        m_clError = clSetKernelArg(clKernel,
                                   m_kernelArgCounter[kernelId]++,
                                   sizeof(cl_mem),
                                   (void *) &clOutput);
        CHECK_CL_ERROR("[ERROR] Unable to set OpenCL Kernel argument");

        m_kernelOutputArgs[kernelId].append(QPair<cl_mem, void *>(clOutput, output));
    }

    template<typename T>
    cl_mem setCommonKernelArg(T *buffer, unsigned size, cl_mem clCommon = 0, QString kernelId = QString())
    {
        if (kernelId.isNull() || kernelId.isEmpty())
            kernelId = m_clKernels.firstKey();

        if (!m_kernelOutputArgs.contains(kernelId))
            return 0;

        cl_kernel clKernel = m_clKernels[kernelId];
        if (!m_clContext || !clKernel || !size)
            return 0;

        bool first = !clCommon;
        if (first) {
            clCommon = clCreateBuffer(m_clContext,
                                      CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      sizeof(T) * size,
                                      buffer,
                                      &m_clError);
            CHECK_CL_ERROR_RET("[ERROR] Unable to create OpenCL Output Buffer");
        }

        m_clError = clSetKernelArg(clKernel,
                                   m_kernelArgCounter[kernelId]++,
                                   sizeof(cl_mem),
                                   (void *) &clCommon);
        CHECK_CL_ERROR_RET("[ERROR] Unable to set OpenCL Kernel argument");

        // Guarantees that the clCommon is deallocated only once
        if (first)
            m_kernelOutputArgs[kernelId].append(QPair<cl_mem, void *>(clCommon, buffer));
        else
            m_kernelInputArgs[kernelId].append(0);

        return clCommon;
    }

    void release(const QString &kernelId = QString());

    bool hasError() const;

    cl_device_id getDevice() const;
    cl_context getContext() const;
    cl_command_queue getCommandQueue() const;
    cl_kernel getKernel(const QString &kernelId = QString()) const;

private:
    void initPlatforms(unsigned platformId = 0);
    void initDevices(unsigned deviceId = 0);
    void initContext();
    void initCommandQueue();

    void createProgram(const QString &);
    void buildProgram();

    void releaseInputArgs(const QString &);
    void releaseOutputArgs(const QString &);

    cl_int m_clError;

    QVector<cl_platform_id> m_clPlatformList;
    cl_platform_id m_clPlatform;
    QVector<cl_device_id> m_clDeviceList;
    cl_device_id m_clDevice;

    cl_context m_clContext;
    cl_command_queue m_clCommandQueue;

    cl_program m_clProgram;
    QStringList m_programMacros;

    QMap<QString, cl_kernel> m_clKernels;
    QMap<QString, unsigned> m_kernelArgCounter;
    QMap<QString, QVector<cl_mem> > m_kernelInputArgs;
    QMap<QString, QVector<QPair<cl_mem, void *> > > m_kernelOutputArgs;
};

#endif // GPU_H
