#include "BoxFilter.PS.hlsl"

Texture2D<float> gMaskTexture : register(t1);

struct Dissolve
{
    float3 edgeColor;
    float threshold;
    //float edgerange;
};
ConstantBuffer<Dissolve> gDissolve : register(b3);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    output = ShadingBoxFilter(input);
    // 必要なら box filter を取得（今回はシーン色を使う）
    // output = ShadingBoxFilter(input); // もしフィルタを使う場合は scene を上書きせず保持する

    float mask = gMaskTexture.Sample(gSampler, input.texcoord); // 0..1

    // フェードを滑らかにする
    //float t = mask - gDissolve.threshold/* / max(gDissolve.edgeRange, 1e-6))*/;
    if (mask <= gDissolve.threshold)
    {
        output.color = float4(gDissolve.edgeColor, 1.0f);
        return output;
    }

    // シーン色を取得（ポスト処理前の生の色を参照すること）
    //output.color += gTexture.Sample(gSampler, input.texcoord);

    //// 溶解色とシーン色を線形補間して出力
    //float3 outRgb = (t * output.color.rgb) * ((1.0f - t) * gDissolve.edgeColor);
    
    //output.color = float4(outRgb, output.color.a);
    
    return output;
}

