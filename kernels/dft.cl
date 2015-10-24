__kernel void dft(__global float *input,
                  const uint2 sizes,
                  const float dir,
                  const float norm,
                  __global float2 *output)
{
    int u = get_global_id(0);
    int v = get_global_id(1);

    if (u >= sizes.x || v >= sizes.y)
        return;

    float2 sum = (float2)(0.0f, 0.0f);

    for (int y = 0; y < sizes.y; ++y) {
        for (int x = 0; x < sizes.x; ++x) {
            float a = (float)u * (float)x / (float)sizes.x;
            float b = (float)v * (float)y / (float)sizes.y;
            float angle = dir * 2.0 * M_PI * (a + b);
            float sinval, cosval;

            float2 f = (float2)(input[x + y * sizes.x]);
            sinval = sincos(angle, &cosval);
            sum += (float2)(cosval, sinval) * f;
        }
    }

    int index = u + v * sizes.x;
    output[index] = sum * (float2)(norm);
}
