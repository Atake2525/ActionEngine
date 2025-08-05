#include "Sprite.hlsli"




struct TransformationMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

//struct VertexShaderOutput{
//    float32_t4 position : SV_POSITION;
//};

struct VertexShaderInput
{
   // float32_t4 position : POSITION0;
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    
    output.texcoord = input.texcoord;
    
    output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.World));
    
    output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    
    return output;
}

