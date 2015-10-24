#ifndef DFTCPU_H
#define DFTCPU_H

#include "ft.h"

class DFTCpu : public FT {
public:
    explicit DFTCpu(FImage *image, QObject *parent = 0);
    ~DFTCpu();

private:
    Complex *calculateFourier(Complex *input, bool inverse = false);
};

#endif // DFTCPU_H
