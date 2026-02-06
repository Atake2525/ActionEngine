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

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    //float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    //output.color = textureColor * gMaterial.color;
    
    float3 viewDir = normalize(input.texcoord);

    // --- Sky gradient ---
    float t = saturate(viewDir.y * 0.5f + 0.5f);

    float3 topColor = float3(0.45, 0.65, 1.0); // ‹óF
    float3 bottomColor = float3(0.02, 0.05, 0.15); // —•F

    float3 skyColor = lerp(bottomColor, topColor, t);

    // --- Sun direction (CPU ‚©‚ç“n‚·‚×‚«) ---
    float3 sunDir = normalize(float3(0.0, 0.7, 0.7)); // ¡‚ÍŒÅ’è

    // viewDir ‚ª‘¾—z•ûŒü‚É‹ß‚¢‚Ù‚Ç’l‚ª‘å‚«‚­‚È‚é
    float sunInfluence = saturate(dot(viewDir, sunDir));

    // ‘¾—z‚Ì‰e‹¿‚ğ_‚ç‚©‚­‚·‚é
    sunInfluence = pow(sunInfluence, 2.0);

    // ‘¾—z•ûŒü‚ÉŒü‚©‚¤‚Ù‚Ç‹ó‚ğ–¾‚é‚­‚·‚é
    float3 horizonBright = float3(0.25, 0.25, 0.20); // –¾‚é‚³‚ÌF–¡
    skyColor += horizonBright * sunInfluence;

    // --- Sun disk ---
    float sunAmount = saturate(dot(viewDir, sunDir));
    float sunGlow = pow(sunAmount, 800.0);
    float3 sunColor = float3(1.0, 0.95, 0.8);

    float3 finalColor = skyColor + sunColor * sunGlow;

    output.color = float4(finalColor, 1.0);

    
    return output;
}