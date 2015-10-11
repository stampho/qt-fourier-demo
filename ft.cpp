#include "ft.h"
#include "fimage.h"

#include <QTime>

#include <math.h>

inline float Complex::magnitude() const
{
    return sqrt((real * real) + (imag * imag));
}
inline float Complex::phase() const
{
    return atan(imag / real);
}

QDebug operator<<(QDebug debug, const Complex &c)
{
    QDebug verbose = debug.nospace().noquote();
    verbose << c.real << "+" << c.imag << "i";
    return verbose;
}


FT::FT(FImage *image, QObject *parent)
    : QObject(parent)
    , m_rows(image->height())
    , m_cols(image->width())
    , m_fourier(0)
    , m_magnitude(0)
    , m_phase(0)
{
    int size = image->data().size();
    Q_ASSERT(size == m_rows * m_cols);

    m_values = new float[size];
    const uchar *values = image->data().constData();
    for (int i = 0; i < size; ++i)
        m_values[i] = (float)values[i];

    QTime timer;
    timer.start();
    qDebug() << "Working on Fourier Transformation...";

    m_fourier = calculateFourier(m_values);
    m_magnitude = calculateMagnitude(m_fourier);
    m_phase = calculatePhase(m_fourier);

    qDebug() << "BOOM! Done.";
    qDebug() << "It took" << timer.elapsed() << "msecs";
}

FT::~FT()
{
    if (m_values)
        delete m_values;

    if (m_fourier)
        delete m_fourier;
    if (m_magnitude)
        delete m_magnitude;
    if (m_phase)
        delete m_phase;
}

FImage FT::magnitudeImage() const
{
    int size = m_cols * m_rows;
    uchar *data = new uchar[size];

    for (int i = 0; i < size; ++i) {
        float mag = m_magnitude[i];
        float value = 20 * log(mag + 1);
        if (value > 255.0)
            value = 255.0;
        data[i] = (uchar)value;
    }

    return FImage(fftshift<uchar>(data), m_cols, m_rows);
}

FImage FT::phaseImage() const
{
    int size = m_cols * m_rows;
    uchar *data = new uchar[size];
    for (int i = 0; i < size; ++i) {
        float phase = m_phase[i];
        float value = phase + M_PI;
        value *= 255.0/(2 * M_PI);
        data[i] = (uchar)value;
    }

    return FImage(fftshift<uchar>(data), m_cols, m_rows);
}

// TODO(pvarga): Complex array as an input is not supported yet
Complex *FT::calculateFourier(float *input, bool inverse)
{
    const float dir = inverse ? 1.0 : -1.0;
    Complex *fourier = new Complex[m_rows * m_cols];

    for (int v = 0; v < m_rows; ++v) {
        for (int u = 0; u < m_cols; ++u) {
            float sumReal = 0.0;
            float sumImag = 0.0;

            for (int y = 0; y < m_rows; ++y) {
                for (int x = 0; x < m_cols; ++x) {
                    float f = (float)input[x + y * m_cols];
                    float a = (float)u * (float)x / (float)m_cols;
                    float b = (float)v * (float)y / (float)m_rows;
                    float angle = dir * 2.0 * M_PI * (a + b);

                    sumReal += (float)f * cos(angle);
                    sumImag += (float)f * sin(angle);
                }
            }

            int index = u + v * m_cols;
            fourier[index].real = sumReal;
            fourier[index].imag = sumImag;
        }
    }

    return fourier;
}

float *FT::calculateMagnitude(Complex *input)
{
    int size = m_rows * m_cols;
    float *magnitude = new float[size];

    for (int i = 0; i < size; ++i)
        magnitude[i] = input[i].magnitude();

    return magnitude;
}

float *FT::calculatePhase(Complex *input)
{
    int size = m_rows * m_cols;
    float *phase = new float[size];

    for (int i = 0; i < size; ++i)
        phase[i] = input[i].phase();

    return phase;
}

template <typename T>
T *FT::fftshift(const T *input, bool inverse) const
{
    int size = m_rows * m_cols;
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
