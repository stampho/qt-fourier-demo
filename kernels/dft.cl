__kernel void dft(__global float2 *input,
                  const uint width,
                  const uint height,
                  const float dir,
                  const float norm,
                  __global float2 *output)
{
    int u = get_global_id(0);
    int v = get_global_id(1);

    if (u >= width || v >= height)
        return;

    float2 sum = (float2)(0.0f, 0.0f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float a = (float)u * (float)x / (float)width;
            float b = (float)v * (float)y / (float)height;

            float angle = dir * 2.0 * M_PI * (a + b);
            float sinval, cosval;
            sinval = sincos(angle, &cosval);

            float2 c;
            float2 f = input[x + y * width];
            c.x = (cosval * f.x) - (sinval * f.y);
            c.y = (cosval * f.y) + (f.x * sinval);

            sum += c;
        }
    }

    int index = u + v * width;
    output[index] = sum * (float2)(norm);
}
