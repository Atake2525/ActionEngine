#include "Skybox.hlsli"

TextureCube<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct Material
{
    float4 color;
};
ConstantBuffer<Material> gMaterial : register(b0);

struct Sun
{
    float3 sunDirection;
    float3 topColor;
    float3 bottomColor;
};
ConstantBuffer<Sun> gSun : register(b1);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    //float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    //output.color = textureColor * gMaterial.color;
    
    float3 viewDir = normalize(input.texcoord);

    // --- Sky gradient ---
    float t = saturate(viewDir.y * 0.5f + 0.5f);

    float3 topColor = gSun.topColor; // 空色
    float3 bottomColor = gSun.bottomColor; // 藍色

    float3 skyColor = lerp(bottomColor, topColor, t);

    // --- Sun direction (CPU から渡すべき) ---
    float3 dirSun = gSun.sunDirection;
    dirSun.z *= -1.0f;
    float3 sunDir = normalize(dirSun); // 今は固定

    // viewDir が太陽方向に近いほど値が大きくなる
    float sunInfluence = saturate(dot(viewDir, sunDir));

    // 太陽の影響を柔らかくする
    sunInfluence = pow(sunInfluence, 2.0);

    // 太陽方向に向かうほど空を明るくする
    float3 horizonBright = float3(0.25, 0.25, 0.20); // 明るさの色味
    skyColor += horizonBright * sunInfluence;

    // --- Sun disk ---
    float sunAmount = saturate(dot(viewDir, sunDir));
    float sunGlow = pow(sunAmount, 800.0);
    float3 sunColor = float3(1.0, 0.95, 0.8);

    float3 finalColor = skyColor + sunColor * sunGlow;

    output.color = float4(finalColor, 1.0);

    
    return output;
}