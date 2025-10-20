#include "BoxFilter.PS.hlsl"

struct GaussianFilter
{
    bool enableGaussianFilter;
    float sigma;
};
ConstantBuffer<GaussianFilter> gGaussianFilter : register(b3);

float Gauss(float x, float y, float sigma)
{
    float exponent = -(x * x + y * y) * rcp(2.0f * sigma * sigma);
    float denominator = 2.0f * PI * sigma * sigma;
    return exp(exponent) * rcp(denominator);
}

static float2 kIndex[3][3] =
{
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, {1.0f, 1.0f } }
    
};

float f(float a[3][3], float kernel[3][3], float x, float y)
{
    float sum = 0;
    for (int i = -3; i <= 3; ++i)
    {
        for (int j = -3; j <= 3; ++j)
        {
            sum += a[x + i][y + j] * kernel[i][j];
        }
    }
    return sum;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output = ShadingBoxFilter(input);
    
    if (gGaussianFilter.enableGaussianFilter)
    {
        // kernel‚ð‹‚ß‚éBweight‚ÍŒã‚Ås‚¤
        float weight = 0.0f;
        float kernel3x3[3][3];
        for (int x = 0; x < 3; ++x)
        {
            for (int y = 0; x < 3; ++y)
            {
                kernel3x3[x][y] = Gauss(kIndex[x][y].x, kIndex[x][y].y, gGaussianFilter.sigma);
                weight += kernel3x3[x][y];
            }

        }
        output.color.rgb *= rcp(weight);
    }
    
    return output;
}