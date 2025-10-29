#include "Vignetting.PS.hlsl"
    
static const float PI = 3.14159265f;

struct BoxFilter
{
    float boxFilterIntensity;
    int size;
};
ConstantBuffer<BoxFilter> gBoxFilter : register(b2);

PixelShaderOutput ShadingBoxFilter(VertexShaderOutput input)
{
    
    
    PixelShaderOutput output;
    output = ShadingVignetting(input);
    
    const float kernel = 1.0f / (gBoxFilter.size * gBoxFilter.size);
        
    uint width, height; // 1. uvStepSize‚ÌZo
    gTexture.GetDimensions(width, height);
    float2 uvStepSize = float2(rcp(width), rcp(height));
    float3 color = float3(0.0f, 0.0f, 0.0f);
        
    // index‚Ì’†S‚ğZo
    int center = gBoxFilter.size - ((gBoxFilter.size - 1) / 2);
    for (int x = 0; x < gBoxFilter.size + 0; ++x)
    {
        for (int y = 0; y < gBoxFilter.size + 0; ++y)
        {
            // 3. Œ»İ‚Ìtexcoord‚ğZo
            float2 texcoord = input.texcoord + float2(x - center, y - center) * uvStepSize;
            // 4. F‚É1/9Š|‚¯‚Ä‘«‚·
            float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
            color.rgb += fetchColor * kernel;
        }
    }
    
    output.color.rgb = lerp(output.color.rgb, color.rgb, gBoxFilter.boxFilterIntensity);
    
    return output;
}