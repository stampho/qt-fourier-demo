#ifndef DFTGPU_H
#define DFTGPU_H

#include "ft.h"

#include <CL/cl.h>

class DFTGpu : public FT {
public:
    explicit DFTGpu(FImage *image, QObject *parent = 0);
    ~DFTGpu();

private:
    Complex *calculateFourier(float *input, bool inverse = false);

    bool initOpenCL();
    bool createKernel(const QString &, const QString &);
    void printCLInfo() const;

    cl_device_id m_clDevice;
    cl_context m_clContext;
    cl_command_queue m_clQueue;
    cl_kernel m_clKernel;
};

#endif // DFTGPU_H
