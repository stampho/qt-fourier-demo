inline unsigned revbin(unsigned x, unsigned bits)
{
    unsigned r = 0;

    while (bits-- > 0) {
        r = r << 1;
        r = r + (x & 1);
        x = x >> 1;
    }

    return r;
}

inline float2 complexMul(float2 c1, float2 c2)
{
    float2 result;

    result.x = c1.x * c2.x - c1.y * c2.y;
    result.y = c1.y * c2.x + c1.x * c2.y;

    return result;
}


__kernel void fft1DRow(__global float2 *fourier,
                       const float dir)
{
    unsigned row = get_global_id(0);

    if (row >= HEIGHT)
        return;

    float2 vector[WIDTH];

    vector[0] = fourier[row * WIDTH];
    vector[WIDTH - 1] = fourier[WIDTH - 1 + (row * WIDTH)];

    for (unsigned i = 1; i < WIDTH - 1; ++i) {
#if WIDTH > 2
        unsigned r = revbin(i, LDWIDTH);
        if (r > i) {
            vector[i] = fourier[r + (row * WIDTH)];
            vector[r] = fourier[i + (row * WIDTH)];
        } else if (r == i)
#endif
            vector[i] = fourier[i + (row * WIDTH)];
    }

    for (unsigned ldm = 1; ldm <= LDWIDTH; ++ldm) {
        unsigned m = 1 << ldm;
        unsigned mh = m >> 1;

        float alpha = dir * 2.0 * (float)M_PI / (float)m;

        for (unsigned r = 0; r < WIDTH; r += m) {
            for (unsigned j = 0; j < mh; ++j) {
                float sinval, cosval;
                sinval = sincos(alpha * (float)j, &cosval);

                float2 v = complexMul(vector[r + j + mh], (float2) (cosval, sinval));
                float2 u = vector[r + j];

                vector[r + j] = u + v;
                vector[r + j + mh] = u - v;
            }
        }
    }

    for (unsigned i = 0; i < WIDTH; ++i)
        fourier[i + (row * WIDTH)] = vector[i];
}

__kernel void fft1DCol(__global float2 *fourier,
                       const float dir,
                       const float norm)
{
    unsigned col = get_global_id(0);

    if (col >= WIDTH)
        return;

    float2 vector[HEIGHT];

    vector[0] = fourier[col];
    vector[HEIGHT - 1] = fourier[col + (HEIGHT - 1) * WIDTH];

    for (unsigned i = 1; i < HEIGHT - 1; ++i) {
#if HEIGHT > 2
        unsigned r = revbin(i, LDHEIGHT);
        if (r > i) {
            vector[i] = fourier[col + (r * WIDTH)];
            vector[r] = fourier[col + (i * WIDTH)];
        } else if (r == i)
#endif
            vector[i] = fourier[col + (i * WIDTH)];
    }

    for (unsigned ldm = 1; ldm <= LDHEIGHT; ++ldm) {
        unsigned m = 1 << ldm;
        unsigned mh = m >> 1;

        float alpha = dir * 2.0 * (float)M_PI / (float)m;

        for (unsigned r = 0; r < HEIGHT; r += m) {
            for (unsigned j = 0; j < mh; ++j) {
                float sinval, cosval;
                sinval = sincos(alpha * (float)j, &cosval);

                float2 v = complexMul(vector[r + j + mh], (float2) (cosval, sinval));
                float2 u = vector[r + j];

                vector[r + j] = u + v;
                vector[r + j + mh] = u - v;
            }
        }
    }

    for (unsigned i = 0; i < HEIGHT; ++i)
        fourier[col + (i * WIDTH)] = vector[i] * (float2) (norm, norm);
}
