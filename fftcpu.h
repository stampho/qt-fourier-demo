#ifndef FFTCPU_H
#define FFTCPU_H

#include "ft.h"

class FFTCpu : public FT {
public:
    explicit FFTCpu(FImage *image, QObject *parent = 0);
    ~FFTCpu();

private:
    Complex *calculateFourier(Complex *input, bool inverse = false);

    void fft1D(Complex *, unsigned, bool) const;
    void revbinPermute(Complex *, unsigned) const;
    inline int revbin(unsigned, unsigned) const;
};

#endif // FFTCPU_H
