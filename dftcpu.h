#ifndef DFTCPU_H
#define DFTCPU_H

#include "ft.h"

class DFTCpu : public FT {
public:
    explicit DFTCpu(FImage *image, QObject *parent = 0);
    ~DFTCpu();

private:
    Complex *calculateFourier(float *input, bool inverse = false) const;
};

#endif // DFTCPU_H
