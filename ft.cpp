#include "ft.h"

#include <QTime>

#include "dftgpu.h"
#include "dftcpu.h"
#include "fftcpu.h"
#include "fftgpu.h"
#include "fimage.h"


Complex::Complex()
    : real(0.0)
    , imag(0.0)
{
}

Complex::Complex(float real, float imag)
    : real(real)
    , imag(imag)
{
}

inline float Complex::magnitude() const
{
    return qSqrt((real * real) + (imag * imag));
}
inline float Complex::phase() const
{
    return qAtan2(imag, real);
}

QDebug operator<<(QDebug debug, const Complex &c)
{
    QDebug verbose = debug.nospace().noquote();
    verbose << c.real << "+" << c.imag << "i";
    return verbose;
}


FT *FT::createFT(FTType type, FImage *image)
{
    switch(type) {
    case FTType::DFTCPU:
        return new DFTCpu(image);
    case FTType::DFTGPU:
        return new DFTGpu(image);
    case FTType::FFTCPU:
        return new FFTCpu(image);
    case FTType::FFTGPU:
        return new FFTGpu(image);
    default:
        return 0;
    }
}

FT::FT(QObject *parent)
    : QObject(parent)
{
}

FT::FT(FImage *image, QObject *parent)
    : QObject(parent)
    , m_rows(image->height())
    , m_cols(image->width())
    , m_fourier(0)
    , m_magnitude(0)
    , m_phase(0)
{
    unsigned size = image->data().size();
    Q_ASSERT(size == m_rows * m_cols);

    m_imageData = new Complex[size];
    const uchar *values = image->data().constData();
    for (unsigned i = 0; i < size; ++i)
        m_imageData[i] = Complex((float)values[i], 0.0);
}

FT::~FT()
{
    if (m_imageData)
        delete m_imageData;

    if (m_fourier)
        delete m_fourier;
    if (m_magnitude)
        delete m_magnitude;
    if (m_phase)
        delete m_phase;
}

int FT::init()
{
    QTime timer;
    timer.start();

    m_fourier = calculateFourier(m_imageData, false);

    int elapsed = timer.elapsed();

    m_magnitude = calculateMagnitude(m_fourier);
    m_phase = calculatePhase(m_fourier);

    return elapsed;
}

int FT::bench()
{
    QTime timer;
    timer.start();

    calculateFourier(m_imageData, false);

    return timer.elapsed();
}

FImage FT::magnitudeImage() const
{
    if (!m_magnitude)
        return FImage(m_cols, m_rows);

    unsigned size = m_cols * m_rows;
    uchar *data = new uchar[size];

    for (unsigned i = 0; i < size; ++i) {
        float mag = m_magnitude[i];
        float value = 20 * log(mag + 1);
        if (value > 255.0)
            value = 255.0;
        data[i] = (uchar)value;
    }

    return FImage(fftshift<uchar>(data), m_cols, m_rows);
}

float *FT::calculateMagnitude(Complex *input) const
{
    unsigned size = m_rows * m_cols;
    float *magnitude = new float[size];

    for (unsigned i = 0; i < size; ++i)
        magnitude[i] = input[i].magnitude();

    return magnitude;
}

FImage FT::reconstructFromMagnitude()
{
    if (!m_magnitude)
        return FImage(m_cols, m_rows);

    unsigned size = m_cols * m_rows;
    uchar *data = new uchar[size];

    Complex *magnitude = new Complex[size];
    for (unsigned i = 0; i < size; ++i)
        magnitude[i] = Complex(m_magnitude[i], 0.0);

    Complex *rec = calculateFourier(magnitude, true);
    float *recMagnitude = calculateMagnitude(rec);

    for (unsigned i = 0; i < size; ++i) {
        float value = recMagnitude[i];
        if (value > 255.0)
            value = 255.0;
        data[i] = (uchar)value;
    }

    delete magnitude;
    delete rec;
    delete recMagnitude;

    return FImage(data, m_cols, m_rows);
}


FImage FT::phaseImage() const
{
    if (!m_phase)
        return FImage(m_cols, m_rows);

    int size = m_cols * m_rows;
    uchar *data = new uchar[size];

    for (int i = 0; i < size; ++i) {
        float value = m_phase[i] + (float)M_PI;
        value *= 255.0/(2.0 * (float)M_PI);
        data[i] = (uchar)value;
    }

    return FImage(fftshift<uchar>(data), m_cols, m_rows);
}

float *FT::calculatePhase(Complex *input) const
{
    unsigned size = m_rows * m_cols;
    float *phase = new float[size];

    for (unsigned i = 0; i < size; ++i)
        phase[i] = input[i].phase();

    return phase;
}

FImage FT::reconstructFromPhase()
{
    if (!m_phase)
        return FImage(m_cols, m_rows);

    unsigned size = m_cols * m_rows;
    uchar *data = new uchar[size];

    Complex *phase = new Complex[size];
    for (unsigned i = 0; i < size; ++i)
        phase[i] = Complex(m_phase[i], 0.0);

    Complex *rec = calculateFourier(phase, true);
    float *recPhase = calculatePhase(rec);

    for (unsigned i = 0; i < size; ++i) {
        float value = recPhase[i] + (float)M_PI;
        value *= 255.0/(2.0 * (float)M_PI);
        data[i] = (uchar)value;
    }

    delete phase;
    delete rec;
    delete recPhase;

    return FImage(data, m_cols, m_rows);
}

FImage FT::reconstructOriginalImage()
{
    if (!m_fourier)
        return FImage(m_cols, m_rows);

    unsigned size = m_cols * m_rows;
    uchar *data = new uchar[size];

    Complex *rec = calculateFourier(m_fourier, true);

    for (unsigned i = 0; i < size; ++i)
        data[i] = (uchar)rec[i].real;

    delete rec;

    return FImage(data, m_cols, m_rows);
}

template <typename T>
T *FT::fftshift(const T *input, bool inverse) const
{
    unsigned size = m_rows * m_cols;
    T *output = new T[size];

    int rowsMid = m_rows / 2;
    int oddRows = m_rows & 1;

    int colsMid = m_cols / 2;
    int oddCols = m_cols & 1;

    for (int i = 0; i < m_rows; ++i) {
        T vector[m_cols];
        for (int j = 0; j < m_cols; ++j) {
            int index = j + i * m_cols;

            int colIndex = j + colsMid + (oddCols & inverse);
            if (colIndex >= m_cols)
                colIndex = colIndex - m_cols;

            vector[colIndex] = input[index];
        }

        int rowIndex = i + rowsMid + (oddRows & inverse);
        if (rowIndex >= m_rows)
            rowIndex = rowIndex - m_rows;

        memcpy(output + rowIndex * m_cols, &vector, m_cols * sizeof(T));
    }

    return output;
}
