#include "object3d.hlsli"
#include "DistanceCulling.PS.hlsl"

Texture2D<float4> albedoTexture : register(t0);
Texture2D<float4> normalTexture : register(t2);
Texture2D<float4> metallicTexture : register(t3);
Texture2D<float4> roughnessTexture : register(t4);
TextureCube<float4> gEnvironmentTexture : register(t1);
SamplerState gSampler : register(s0);

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
    float4 color; //!< ライトの色
    float3 direction; //!< ライトの向き
    float intensity; //!< 輝度
    float3 specularColor;
};
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b2);

struct PointLight
{
    float4 color; //!< ライトの色
    float3 position; //!< ライトの位置
    float intensity; //!< 輝度
    float radius; //!< ライトの届く最大距離
    float dacay; //!< 減衰率
    float3 specularColor;
};
ConstantBuffer<PointLight> gPointLight : register(b3);

struct SpotLight
{
    float4 color; //!< ライトの色
    float3 position; //!< ライトの位置
    float intensity; //!< 輝度
    float3 direction; //!< スポットライトの方向
    float distance; //!< ライトの届く最大距離
    float dacay; //!< 減衰率
    float cosAngle; //!< スポットライトの余弦
    float cosFalloffStart; // falloffが開始される角度
    float3 specularColor;
};
ConstantBuffer<SpotLight> gSpotLight : register(b4);

struct MaterialTemplate
{
    float metallic;
    float roughness;
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

static PixelShaderOutput RoadMaterialTemplate(PixelShaderOutput output, VertexShaderOutput input)
{
    // 環境マップ
    float3 cameraToPosition = normalize(input.worldPosition - gCamera.worldPosition);
    float3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
    float4 environmentColor = gEnvironmentTexture.Sample(gSampler, reflectedVector);
    
    output.color.rgb += environmentColor.rgb * gMaterialTemplate.metallic * float(gMaterial.enableMetallic);
    
    return output;
}

static float3 TorranceSparrow(float3 lightDir, float3 viewDir, float3 normal, float roughness, float3 F0)
{
    float3 halfVector = normalize(lightDir + viewDir);
    
    // 法線分布関数
    float NdotH = saturate(dot(normal, halfVector));
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = (NdotH * NdotH) * (alpha2 - 1.0f) + 1.0f;
    float D = alpha2 / (3.14159f * denom * denom);
    
    // 幾何学的減衰関数
    float NdotV = saturate(dot(normal, viewDir));
    float NdotL = saturate(dot(normal, lightDir));
    float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f; // Schlickの近似
    float G_V = NdotV / (NdotV * (1.0f - k) + k);
    float G_L = NdotL / (NdotL * (1.0f - k) + k);
    float G = G_V * G_L;
    
    // フレネル反射率
    float VdotH = saturate(dot(viewDir, halfVector));
    float3 F = F0 + (1.0f - F0) * pow(1.0f - VdotH, 5.0f); // Schlickの近似
    
    return (D * G * F) / (4.0f * (NdotL * NdotV) + 0.001f);
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
    output.color = gMaterial.color;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = albedoTexture.Sample(gSampler, transformedUV.xy);
    float3 normal = normalTexture.Sample(gSampler, transformedUV.xy).rgb;
    float metallic = metallicTexture.Sample(gSampler, transformedUV.xy).b;
   
    float roughness = roughnessTexture.Sample(gSampler, transformedUV.xy).r;
    
    float3 tangent = normalize(input.tangent);
    float3 bitangent = normalize(input.bitangent);
    normal = normalize(normal * 2.0f - 1.0f);
    
    float3x3 TBN = float3x3(tangent, bitangent, normalize(input.normal));
    
    normal = normalize(mul(normal, TBN));
    
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
    { // Lightingしない場合。前回までと同じ計算
        output.color = gMaterial.color * textureColor;
    }
    output.color.rgb = RoadMaterialTemplate(output, input).color.rgb;

    output.color.a = HeightCulling(input.worldPosition, gCullingTemplate.drawHeight);
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