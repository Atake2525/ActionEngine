#include "Dissolve.PS.hlsl"

// リソース定義 (ルートシグネチャに合わせてレジスタは調整してください)
struct MotionBluer {
    float2 iResolution; // 画面解像度
    float iTime; // 経過時間
    float padding; // アライメント用
};
ConstantBuffer<MotionBluer> gMotionBluer : register(b4);

static const float BLUR_RADIUS_MAX = 120.0;
static const float SPEED = 3.0;

float4 BlurH(Texture2D source, SamplerState samp, float2 size, float2 uv, float radius)
{
    if (radius >= 1.0)
    {
        float4 C = float4(0.0, 0.0, 0.0, 0.0);
        float width = 1.0 / size.x;
        float divisor = 0.0;

        // GLSLのループロジックを維持
        // 実際にはループ回数はintで行う方が安全ですが、元の挙動を再現しています
        for (float x = -radius; x <= 0.0; x += 1.0)
        {
            // 元コードの uv + float4(...) は型不一致のため float2(...) に修正
            C += source.Sample(samp, uv + float2(x * width, 0.0));
            divisor += 1.0;
        }

        return float4(C.rgb / divisor, 1.0);
    }

    return source.Sample(samp, uv);
}

//float4 BlurV(Texture2D source, SamplerState samp, float2 size, float2 uv, float radius)
//{
//    if (radius >= 1.0)
//    {
//        float4 C = float4(0.0, 0.0, 0.0, 0.0);
//        float height = 1.0 / size.y;
//        float divisor = 0.0;

//        for (float y = -radius; y <= radius; y += 1.0)
//        {
//            C += source.Sample(samp, uv + float2(0.0, y * height));
//            divisor += 1.0;
//        }

//        return float4(C.rgb / divisor, 1.0);
//    }

//    return source.Sample(samp, uv);
//}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output = ShadingDissolve(input); // BoxFilterを取得（今回はシーン色を使う）)
    float2 uv = input.texcoord;
    float percent = (sin(gMotionBluer.iTime * SPEED) + 1.0) / 2.0;
    float blurRadius = percent * BLUR_RADIUS_MAX;
    // 元コード: texture(..., -1000.0) -> LODバイアスでミップレベル0を強制する意図と解釈
    // SampleLevel(..., 0) で明示的にLOD 0をサンプリングします
    float4 A = gTexture.SampleLevel(gSampler, uv, 0) * (1.0 - percent);
    
    // BlurH関数呼び出し
    A += BlurH(gTexture, gSampler, gMotionBluer.iResolution, uv, blurRadius) * percent;
    output.color = A;
    return output;
}