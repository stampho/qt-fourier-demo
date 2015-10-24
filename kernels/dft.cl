__kernel void dft(__global float *input,
                  const float dir,
                  const float norm,
                  __global float2 *output)
{
    int u = get_global_id(0);
    int v = get_global_id(1);

    int cols = get_global_size(0);
    int rows = get_global_size(1);

    float2 sum = (float2)(0.0f, 0.0f);

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            float a = (float)u * (float)x / (float)cols;
            float b = (float)v * (float)y / (float)rows;
            float angle = dir * 2.0 * M_PI * (a + b);
            float sinval, cosval;

            float2 f = (float2)(input[x + y * cols]);
            sinval = sincos(angle, &cosval);
            sum += (float2)(cosval, sinval) * f;
        }
    }

    int index = u + v * cols;
    output[index] = sum * (float2)(norm);
}
