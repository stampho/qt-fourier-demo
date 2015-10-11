#ifndef DFTGPU_H
#define DFTGPU_H

#include "ft.h"

class DFTGpu : public FT {
public:
    explicit DFTGpu(FImage *image, QObject *parent = 0);
    ~DFTGpu();

private:
    Complex *calculateFourier(float *input, bool inverse = false);

    bool initOpenCL();
};

#endif // DFTGPU_H
