#ifndef GPU_H
#define GPU_H

#include <CL/cl.h>
#include <QDebug>
#include <QMap>
#include <QObject>
#include <QVector>

class GPU : public QObject {
    Q_OBJECT
public:
    explicit GPU(QObject *parent = 0);
    virtual ~GPU();

    void createKernel(const QString &, const QString &);
    bool hasError() const;

protected:
    cl_context getContext() const;
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
};

#endif // GPU_H
