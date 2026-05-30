#include "object3d.hlsli"
#include "DistanceCulling.PS.hlsl"

Texture2D<float4> albedoTexture : register(t0);
Texture2D<float4> normalTexture : register(t2);
Texture2D<float4> metallicTexture : register(t3);
Texture2D<float4> roughnessTexture : register(t4);
TextureCube<float4> gEnvironmentTexture : register(t1);
SamplerState gSampler : register(s0);

static const float PI = 3.14159265f;

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
    float3 specularColor;
    int enableMetallic;
    float environmentCoefficient;
};
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct Camera
{
    float3 worldPosition;
    float nearClipDistance;
    float farClipDistance;
};
ConstantBuffer<Camera> gCamera : register(b1);

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
    float3 specularColor;
};
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b2);

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float dacay;
    float3 specularColor;
};
ConstantBuffer<PointLight> gPointLight : register(b3);

struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float dacay;
    float cosAngle;
    float cosFalloffStart;
    float3 specularColor;
};
ConstantBuffer<SpotLight> gSpotLight : register(b4);

struct MaterialTemplate
{
    float metallic;
    float roughness;
    float2 padding;
};
ConstantBuffer<MaterialTemplate> gMaterialTemplate : register(b5);

struct padd
{
    float pad0;
    float pad1;
    float pad2;
    float pad3;
};
ConstantBuffer<padd> gPad : register(b6);

struct CullingTemplate
{
    float drawHeight;
};
ConstantBuffer<CullingTemplate> gCullingTemplate : register(b7);

struct ScanParam
{
    float3 color;
    float width;
    float radius;
};
ConstantBuffer<ScanParam> gScanParam : register(b8);

static float Max3(float3 value)
{
    // RGBのうち最大値を取る小さな補助関数。
    // metallic mapやnormal mapのフォールバック判定では、1チャンネルだけに依存すると素材によって誤判定しやすいため、全チャンネルを見て「何か値が入っているか」を判断する。
    return max(value.r, max(value.g, value.b));
}

static float3 SampleWorldNormal(VertexShaderOutput input, float2 uv)
{
    // PBRでは法線方向がライティング結果に強く影響するため、normal mapがある場合はここでワールド空間法線へ変換する。
    float3 vertexNormal = normalize(input.normal);
    float3 normalSample = normalTexture.Sample(gSampler, uv).rgb;

    if (Max3(normalSample) <= 0.01f)
    {
        // 現状のフォールバックnormal mapは黒テクスチャを使うことがある。
        // 黒(0,0,0)をそのまま -1..1 に展開すると不正な法線になり、全面が暗くなるため、値がほぼ無い場合は頂点法線を使う。
        return vertexNormal;
    }

    // normal mapは接線空間で保存されているため、0..1 の値を -1..1 に戻してからTBN行列でワールド空間へ変換する。
    float3 tangentNormal = normalize(normalSample * 2.0f - 1.0f);

    float3 tangent = normalize(input.tangent);
    // tangentを法線に対して再直交化する。モデル由来のtangentが少し歪んでいても、TBNの破綻を抑えるため。
    tangent = normalize(tangent - vertexNormal * dot(tangent, vertexNormal));
    float3 sourceBitangent = normalize(input.bitangent);
    // bitangentをそのまま使うと、tangent再直交化後にTBNが直交しない場合がある。
    // tangentとnormalから再生成し、元bitangentとの向き比較で左右反転だけ引き継ぐ。
    float bitangentSign = dot(cross(vertexNormal, tangent), sourceBitangent) < 0.0f ? -1.0f : 1.0f;
    float3 bitangent = normalize(cross(vertexNormal, tangent)) * bitangentSign;

    float3x3 tbn = float3x3(tangent, bitangent, vertexNormal);
    return normalize(mul(tangentNormal, tbn));
}

static float SampleMetallic(float2 uv)
{
    // metallicは「マテリアル定数」と「metallic map」の両方を扱う。
    // テストシーンのようにテクスチャ無しで係数だけ変えたいケースがあるので、map値と定数の大きい方を採用している。
    float metallicMap = Max3(metallicTexture.Sample(gSampler, uv).rgb);
    // saturateで0..1に丸め、入力ミスやテクスチャ値の揺れがBRDFを壊さないようにする。
    return saturate(max(gMaterialTemplate.metallic, metallicMap));
}

static float SampleRoughness(float2 uv)
{
    // roughnessは鏡面反射の広がりを決める値。0に近いほど鋭いハイライト、1に近いほど鈍いハイライトになる。
    float roughnessMap = roughnessTexture.Sample(gSampler, uv).r;
    // フォールバックroughness mapが黒の場合に粗さ0扱いになると、GGXの分母が極端になり白飛びしやすい。
    // そのためmapが実質未設定ならMaterialTemplate側のroughnessをそのまま使う。
    // mapがある場合は「テクスチャの模様」x「素材全体のroughness係数」として扱う。
    float roughness = roughnessMap > 0.001f ? roughnessMap * max(gMaterialTemplate.roughness, 0.001f) : gMaterialTemplate.roughness;
    // roughnessを0.04未満にしない。完全な0に近い粗さはリアルタイム描画でノイズや過剰な輝度を出しやすいため。
    return clamp(roughness, 0.04f, 1.0f);
}

static float DistributionGGX(float3 normal, float3 halfVector, float roughness)
{
    // GGX/Trowbridge-Reitzの法線分布関数(D)。
    // マイクロファセットがhalfVector方向を向いている割合を表し、roughnessが低いほど鋭いスペキュラーになる。
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float nDotH = saturate(dot(normal, halfVector));
    float nDotH2 = nDotH * nDotH;
    float denom = nDotH2 * (alpha2 - 1.0f) + 1.0f;
    return alpha2 / max(PI * denom * denom, 0.0001f);
}

static float GeometrySchlickGGX(float nDotV, float roughness)
{
    // Schlick-GGXの幾何減衰項の片側分。
    // 視線方向またはライト方向から見て、微細面が互いに隠し合う効果を近似する。
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    return nDotV / max(nDotV * (1.0f - k) + k, 0.0001f);
}

static float GeometrySmith(float3 normal, float3 viewDir, float3 lightDir, float roughness)
{
    // Smith法で視線側とライト側の幾何減衰を合成する。
    // 斜め方向ほど反射が弱くなるため、金属や低roughnessの見え方が自然になる。
    float nDotV = saturate(dot(normal, viewDir));
    float nDotL = saturate(dot(normal, lightDir));
    return GeometrySchlickGGX(nDotV, roughness) * GeometrySchlickGGX(nDotL, roughness);
}

static float3 FresnelSchlick(float cosTheta, float3 f0)
{
    // Schlick近似のフレネル項(F)。
    // 物体を浅い角度で見た時に反射が強くなる現象を安価に再現する。
    // f0は正面から見た反射率で、非金属は約0.04、金属はbaseColor寄りにする。
    return f0 + (1.0f - f0) * pow(1.0f - saturate(cosTheta), 5.0f);
}

static float3 FresnelSchlickRoughness(float cosTheta, float3 f0, float roughness)
{
    // IBL用のフレネル近似。roughnessが高い素材では斜め方向の反射が強くなりすぎないよう、F90側をroughnessで抑える。
    // UE系のsplit-sum IBLでよく使われる形で、BRDF LUTなしでも粗さ別の環境反射を自然に見せやすい。
    float3 f90 = max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), f0);
    return f0 + (f90 - f0) * pow(1.0f - saturate(cosTheta), 5.0f);
}

static float2 ApproximateEnvironmentBRDF(float nDotV, float roughness)
{
    // 本来のPBR IBLではBRDF LUTを参照し、スペキュラー環境光のスケール/バイアスを得る。
    // ここではLUTテクスチャを増やさず、Epic/UE4系で使われる近似式で同じ役割を持たせる。
    // 戻り値xがF0に掛かるscale、戻り値yがgrazing反射のbiasに相当する。
    float4 c0 = float4(-1.0f, -0.0275f, -0.572f, 0.022f);
    float4 c1 = float4(1.0f, 0.0425f, 1.04f, -0.04f);
    float4 r = roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28f * nDotV)) * r.x + r.y;
    return float2(-1.04f, 1.04f) * a004 + r.zw;
}

static float3 EvaluatePBRLight(
    float3 baseColor,
    float metallic,
    float roughness,
    float3 normal,
    float3 viewDir,
    float3 lightDir,
    float3 lightColor,
    float intensity,
    float attenuation)
{
    // 1つのライトからのPBR寄与を計算する共通関数。
    // Directional/Point/Spotで距離減衰や方向は違うが、BRDF計算自体は同じなのでここに集約している。
    float nDotL = saturate(dot(normal, lightDir));
    float nDotV = saturate(dot(normal, viewDir));
    if (nDotL <= 0.0f || nDotV <= 0.0f || intensity <= 0.0f || attenuation <= 0.0f)
    {
        // ライトが裏側、視線が裏側、強度0、減衰0の場合は寄与なし。無駄なBRDF計算とゼロ除算リスクを避ける。
        return float3(0.0f, 0.0f, 0.0f);
    }

    float3 halfVector = normalize(viewDir + lightDir);
    // F0をmetallicで補間する。非金属は0.04程度の白い反射、金属はbaseColor自体が反射色になる。
    float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), baseColor, metallic);

    // Cook-Torrance BRDFの主要3項。D=法線分布、G=幾何減衰、F=フレネル。
    float d = DistributionGGX(normal, halfVector, roughness);
    float g = GeometrySmith(normal, viewDir, lightDir, roughness);
    float3 f = FresnelSchlick(dot(halfVector, viewDir), f0);

    // specularはCook-Torranceの鏡面反射項。分母のmaxはNdotL/NdotVが極小の時の発散防止。
    float3 specular = (d * g * f) / max(4.0f * nDotV * nDotL, 0.001f);
    // エネルギー保存のため、フレネルで反射した分をdiffuseから引く。
    // 金属は拡散反射を持たないので、metallicが1に近いほどdiffuseを消す。
    float3 kD = (float3(1.0f, 1.0f, 1.0f) - f) * (1.0f - metallic);
    float3 diffuse = kD * baseColor / PI;

    // 最後にライト色、ライト強度、距離/スポット減衰、LambertのNdotLを掛けて、このライト1つ分の色にする。
    return (diffuse + specular) * lightColor * intensity * attenuation * nDotL;
}

static float3 EvaluateEnvironment(float3 baseColor, float metallic, float roughness, float3 normal, float3 viewDir)
{
    // split-sum IBLの近似。
    // 1) roughnessに応じて環境CubeMapのmipを選ぶことで、prefiltered environment mapの「粗いほど反射がぼける」挙動を近似する。
    // 2) BRDF LUTの代わりにApproximateEnvironmentBRDF()でscale/biasを計算する。
    // これにより、金属や滑らかな素材が環境を反射し、粗い素材では反射が弱く広がる。
    float3 reflectedVector = normalize(reflect(-viewDir, normal));
    float3 diffuseDirection = normal;
    float maxReflectionMip = 5.0f;
    float specularMip = roughness * maxReflectionMip;

    float3 irradiance = gEnvironmentTexture.SampleLevel(gSampler, diffuseDirection, maxReflectionMip).rgb;
    float3 prefilteredColor = gEnvironmentTexture.SampleLevel(gSampler, reflectedVector, specularMip).rgb;
    float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), baseColor, metallic);
    float nDotV = saturate(dot(normal, viewDir));
    float3 fresnel = FresnelSchlickRoughness(nDotV, f0, roughness);
    float2 envBRDF = ApproximateEnvironmentBRDF(nDotV, roughness);

    // 環境拡散光は非金属にだけ強く出す。金属は拡散反射をほぼ持たず、環境スペキュラーが主になる。
    float3 kD = (float3(1.0f, 1.0f, 1.0f) - fresnel) * (1.0f - metallic);
    float3 diffuseAmbient = irradiance * baseColor * kD;
    // BRDF近似のscale/biasを使って、F0だけでは不足するgrazing反射やroughness依存を補う。
    float3 specularAmbient = prefilteredColor * (f0 * envBRDF.x + envBRDF.y) * gMaterial.environmentCoefficient;
    return diffuseAmbient + specularAmbient;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float2 uv = transformedUV.xy;
    float4 textureColor = albedoTexture.Sample(gSampler, uv);
    // baseColorはマテリアルカラーとアルベドテクスチャの積。
    // PBRではこの値が非金属の拡散色、金属の反射色(F0)として使われる。
    float3 baseColor = saturate(gMaterial.color.rgb * textureColor.rgb);
    float alpha = gMaterial.color.a * textureColor.a;

    if (gMaterial.enableLighting != 0)
    {
        // ライティングON時は、法線、視線方向、metallic、roughnessを先に確定して全ライトで共有する。
        float3 normal = SampleWorldNormal(input, uv);
        float3 viewDir = normalize(gCamera.worldPosition - input.worldPosition);
        float metallic = SampleMetallic(uv);
        float roughness = SampleRoughness(uv);

        // まず環境光をベースに置き、その上に各ライトの直接光を足す。
        float3 color = EvaluateEnvironment(baseColor, metallic, roughness, normal, viewDir);

        // DirectionalLightは距離減衰なし。ライト方向は「面からライトへ向かう方向」に揃えるため符号を反転する。
        float3 directionalLightDir = normalize(-gDirectionalLight.direction);
        color += EvaluatePBRLight(
            baseColor,
            metallic,
            roughness,
            normal,
            viewDir,
            directionalLightDir,
            gDirectionalLight.color.rgb,
            gDirectionalLight.intensity,
            1.0f);

        // PointLightはワールド位置からライト位置へのベクトルを使い、radius内で距離減衰させる。
        float3 pointLightVector = gPointLight.position - input.worldPosition;
        float pointDistance = length(pointLightVector);
        float3 pointLightDir = pointLightVector / max(pointDistance, 0.001f);
        // 既存Light構造体のdacay値を尊重し、radius端で0になる滑らかな減衰にしている。
        float pointAttenuation = pow(saturate(1.0f - pointDistance / max(gPointLight.radius, 0.001f)), max(gPointLight.dacay, 0.0f));
        color += EvaluatePBRLight(
            baseColor,
            metallic,
            roughness,
            normal,
            viewDir,
            pointLightDir,
            gPointLight.color.rgb,
            gPointLight.intensity,
            pointAttenuation);

        // SpotLightはPointLightの距離減衰に加えて、ライトの向きと角度によるfalloffを掛ける。
        float3 spotLightVector = gSpotLight.position - input.worldPosition;
        float spotDistance = length(spotLightVector);
        float3 spotLightDir = spotLightVector / max(spotDistance, 0.001f);
        float3 lightToSurfaceDir = -spotLightDir;
        float spotCos = dot(lightToSurfaceDir, normalize(gSpotLight.direction));
        // cosAngleからcosFalloffStartまでの範囲で滑らかに弱くする。maxで角度差0による除算を避ける。
        float spotFalloff = saturate((spotCos - gSpotLight.cosAngle) / max(gSpotLight.cosFalloffStart - gSpotLight.cosAngle, 0.001f));
        float spotAttenuation = pow(saturate(1.0f - spotDistance / max(gSpotLight.distance, 0.001f)), max(gSpotLight.dacay, 0.0f)) * spotFalloff;
        color += EvaluatePBRLight(
            baseColor,
            metallic,
            roughness,
            normal,
            viewDir,
            spotLightDir,
            gSpotLight.color.rgb,
            gSpotLight.intensity,
            spotAttenuation);

        output.color.rgb = color;
        output.color.a = alpha;
    }
    else
    {
        // ライティングOFF時は従来通り、マテリアルカラーxアルベドだけを出す。UI的な単色表示やデバッグ表示用。
        output.color = float4(baseColor, alpha);
    }

    // ここから下は既存の描画後処理。PBRで作ったRGBは維持し、アルファだけ高さ/距離カリングで調整する。
    output.color.a *= HeightCulling(input.worldPosition, gCullingTemplate.drawHeight);
    output.color.a *= DistanceCulling(input.worldPosition, gCamera.worldPosition, gCamera.nearClipDistance, gCamera.farClipDistance);

    output.color.rgb = ScanEffect(output.color.rgb, gScanParam.color, input.worldPosition, gCamera.worldPosition, gScanParam.width, gScanParam.radius);

    float dist = length(input.worldPosition - gCamera.worldPosition);
    float t = smoothstep(gScanParam.radius, gScanParam.radius * 0.999f, dist);
    output.color.rgb = lerp(float3(1.0f, 1.0f, 1.0f), output.color.rgb, t);

    if (output.color.a < 0.2f)
    {
        discard;
    }

    return output;
}
