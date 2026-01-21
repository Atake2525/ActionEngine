#include "DebugLine.hlsli"

struct VertexShaderInput
{
    float4 position : POSITION0;
    float4 color : COLOR0;
};

struct TransformationMatrix
{
    float4x4 WVP;
};
ConstantBuffer<TransformationMatrix> gtransformationMatrix : register(b0);

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gtransformationMatrix.WVP);
    output.color = input.color;
    return output;
}