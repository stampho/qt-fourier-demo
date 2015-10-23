#ifndef DFTGPU_H
#define DFTGPU_H

#include "ft.h"
#include "gpu.h"

class DFTGpu : public FT, public GPU {
public:
    explicit DFTGpu(FImage *image, QObject *parent = 0);
    ~DFTGpu();

private:
    Complex *calculateFourier(float *input, bool inverse = false) const;

};

#endif // DFTGPU_H
