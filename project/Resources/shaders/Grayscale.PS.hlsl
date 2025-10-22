#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct Grayscale
{
    int enableGrayscale;
    float3 toneColor;
    float alpha;
};
ConstantBuffer<Grayscale> gGrayscale : register(b0);

PixelShaderOutput ShadingGrayscale(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    if (gGrayscale.enableGrayscale)
    {
        float value = dot(output.color.rbg, float3(0.2125f, 0.7154f, 0.0721f));
    
        output.color.rgb = value * float3(gGrayscale.toneColor.x, gGrayscale.toneColor.y, gGrayscale.toneColor.z);
        output.color.a = gGrayscale.alpha;
    }
   

    return output;
}