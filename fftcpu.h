#ifndef FFTCPU_H
#define FFTCPU_H

#include "ft.h"

class FFTCpu : public FT {
public:
    explicit FFTCpu(FImage *image, QObject *parent = 0);
    ~FFTCpu();

private:
    Complex *calculateFourier(float *input, bool inverse = false);

    void fft1D(Complex *, unsigned, float);
    void revbinPermute(Complex *, unsigned);
    inline int revbin(unsigned, unsigned);
    inline bool isPowerOfTwo(unsigned);
};

#endif // FFTCPU_H
