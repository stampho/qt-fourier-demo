#ifndef DFTGPU_H
#define DFTGPU_H

#include "ft.h"

class GPU;

class DFTGpu : public FT {
public:
    explicit DFTGpu(FImage *image, QObject *parent = 0);
    ~DFTGpu();

private:
    Complex *calculateFourier(Complex *input, bool inverse = false);

    QScopedPointer<GPU> m_gpu;
};

#endif // DFTGPU_H
