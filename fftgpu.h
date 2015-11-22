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
    inline bool isPowerOfTwo(unsigned) const;

    // TODO(pvarga): Remove these!
    void fft1D(Complex *, unsigned, bool) const;
    void revbinPermute(Complex *, unsigned) const;
    inline int revbin(unsigned, unsigned) const;

    QScopedPointer<GPU> m_gpu;
};


#endif // FFTGPU_H
