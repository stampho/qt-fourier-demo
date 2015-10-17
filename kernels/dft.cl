__kernel void dft(__constant float *input,
                  __global float *outReal,
                  __global float *outImag)
{
    int u = get_global_id(0);
    int v = get_global_id(1);

    int cols = get_global_size(0);
    int rows = get_global_size(1);

    float sumReal = 0.0f;
    float sumImag = 0.0f;

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            int f = input[x + y * cols];
            float a = (float)u * (float)x / (float)cols;
            float b = (float)v * (float)y / (float)rows;
            float angle = -2.0 * M_PI * (a + b);
            float sinval, cosval;

            sinval = sincos(angle, &cosval);
            sumReal += f * cosval;
            sumImag += f * sinval;
        }
    }

    int index = v + u * cols;
    outReal[index] = sumReal;
    outImag[index] = sumImag;
}
