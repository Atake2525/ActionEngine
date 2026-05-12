#include "Particle.hlsli"


struct PreView
{
    float4x4 viewProjection;
    float4x4 billboardMatrix;
};
ConstantBuffer<PreView> gPreView : register(b0);
StructuredBuffer<Particle> gPareView : register(b0);

struct ParticleForGPU{
    float32_t4x4 WVP;
    float32_t4x4 World;
    float32_t4 color;
};
StructuredBuffer<ParticleForGPU> gParticle : register(t0);

struct VertexShaderInput{
    float4 position : POSITION0;
    //float32_t4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID){
    VertexShaderOutput output;
    output.position = mul(input.position, gParticle[instanceId].WVP);
    output.texcoord = input.texcoord;
    output.color = gParticle[instanceId].color;
    return output;
}

