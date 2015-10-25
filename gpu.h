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


class GPU : public QObject {
    Q_OBJECT
public:
    explicit GPU(QObject *parent = 0);
    virtual ~GPU();

    void preferredWorkGroupSize(size_t size[3], int, int, int) const;
    void createKernel(const QString &, const QString &);

    template<typename T>
    void setInputKernelArg(const T *input)
    {
        if (!m_clContext || !m_clKernel)
            return;

        m_clError = clSetKernelArg(m_clKernel, m_argCounter++, sizeof(T), (void *) input);
        CHECK_CL_ERROR("[ERROR] Unable to set OpenCL Kernel argument");
        m_inputArgs.append(0);
    }

    template<typename T>
    void setInputKernelArg(T *input, unsigned size)
    {
        if (!m_clContext || !m_clKernel || !size)
            return;

        cl_mem clInput = 0;

        clInput = clCreateBuffer(m_clContext,
                                 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(T) * size,
                                 input,
                                 &m_clError);
        CHECK_CL_ERROR("[ERROR] Unable to create OpenCL Input Buffer");

        m_clError = clSetKernelArg(m_clKernel, m_argCounter++, sizeof(cl_mem), (void *) &clInput);
        CHECK_CL_ERROR("[ERROR] Unable to set OpenCL Kernel argument");

        m_inputArgs.append(clInput);
    }

    template<typename T>
    void setOutputKernelArg(T *output, unsigned size)
    {
        if (!m_clContext || !m_clKernel || !size)
            return;

        cl_mem clOutput = clCreateBuffer(m_clContext,
                                         CL_MEM_WRITE_ONLY,
                                         sizeof(T) * size,
                                         0,
                                         &m_clError);
        CHECK_CL_ERROR("[ERROR] Unable to create OpenCL Output Buffer");

        m_clError = clSetKernelArg(getKernel(), m_argCounter++, sizeof(cl_mem), (void *) &clOutput);
        CHECK_CL_ERROR("[ERROR] Unable to set OpenCL Kernel argument");

        m_outputArgs.append(QPair<cl_mem, void *>(clOutput, output));
    }

    void release();

    bool hasError() const;

    cl_device_id getDevice() const;
    cl_command_queue getCommandQueue() const;
    cl_kernel getKernel() const;

private:
    void initPlatforms(unsigned platformId = 0);
    void initDevices(unsigned deviceId = 0);
    void initContext();
    void initCommandQueue();

    void createProgram(const QString &);
    void buildProgram();

    cl_int m_clError;

    QVector<cl_platform_id> m_clPlatformList;
    cl_platform_id m_clPlatform;
    QVector<cl_device_id> m_clDeviceList;
    cl_device_id m_clDevice;

    cl_context m_clContext;
    cl_command_queue m_clCommandQueue;
    cl_program m_clProgram;
    cl_kernel m_clKernel;

    unsigned m_argCounter;
    QVector<cl_mem> m_inputArgs;
    QVector<QPair<cl_mem, void * > > m_outputArgs;
};

#endif // GPU_H
