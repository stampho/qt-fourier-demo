#ifndef FFTGPU_H
#define FFTGPU_H

#include "ft.h"

class GPU;

class FFTGpu : public FT {
public:
    explicit FFTGpu(FImage *image, QObject *parent = 0);
    ~FFTGpu();

private:
    Complex *calculateFourier(Complex *input, bool inverse = false);

    QScopedPointer<GPU> m_gpu;
};


#endif // FFTGPU_H
