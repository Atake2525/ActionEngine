#include "object3d.hlsli"

Texture2D<float4> albedoTexture : register(t0);
Texture2D<float4> normalTexture : register(t2);
Texture2D<float4> metallicTexture : register(t3);
Texture2D<float4> roughnessTexture : register(t4);
TextureCube<float4> gEnvironmentTexture : register(t1);
SamplerState gSampler : register(s0);

/*struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};*/

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

static float3 Flusnel(float3 viewDir, float3 halfVector, float3 F0)
{
    float VdotH = saturate(dot(viewDir, halfVector));
    float3 F = F0 + (1.0f - F0) * pow(1.0f - VdotH, 5.0f); // Schlickの近似
    return F;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gMaterial.color;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = albedoTexture.Sample(gSampler, transformedUV.xy);
    float3 normal = normalTexture.Sample(gSampler, transformedUV.xy).rgb;
    float metallic = metallicTexture.Sample(gSampler, transformedUV.xy).r;
   
    float roughness = roughnessTexture.Sample(gSampler, transformedUV.xy).r;
    
    float3 tangent = normalize(input.tangent);
    float3 bitangent = normalize(input.bitangent);
    normal = normalize(normal * 2.0f - 1.0f);
    
    float3x3 TBN = float3x3(tangent, bitangent, normalize(input.normal));
    
    normal = normalize(mul(normal, TBN));
    
    if (gMaterial.enableLighting != 0)
    { // Lightingする場合
        // Half lambert
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        
        // Phong Reflection Model
        // 計算式 R = reflect(L,N) specular = (V.R)n
        float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        //float32_t3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
    
        // HalfVectorを求めて計算する
        float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
        float NDotH = dot(normalize(input.normal), halfVector);
        
        //float RdotE = dot(reflectLight, toEye);
        float specularPow = pow(saturate(NDotH), gMaterial.shininess); // 反射強度
        
        // pointLight
        float3 pointLightDirection = normalize(input.worldPosition - gPointLight.position);
        
        float NdotLPointLight = dot(normalize(input.normal), -pointLightDirection);
        float cosPointLight = pow(NdotLPointLight * 0.5f + 0.5f, 2.0f);
        
        float3 halfVectorPointLight = normalize(-pointLightDirection + toEye);
        float NDotHPointLight = dot(normalize(input.normal), halfVectorPointLight);
        
        float specularPowPointLight = pow(saturate(NDotH), gMaterial.shininess);
        
        float distance = length(gPointLight.position - input.worldPosition); // ポイントライトへの距離
        float factor = pow(saturate(-distance / gPointLight.radius + 1.0f), gPointLight.dacay); // 逆に上による減衰係数
        
        // spotLight
        float3 spotLightDirectionOnSurFace = normalize(input.worldPosition - gSpotLight.position);
        
        float cosAngle = dot(spotLightDirectionOnSurFace, gSpotLight.direction);
        float falloffFactor = saturate((cosAngle - gSpotLight.cosAngle) / (gSpotLight.cosFalloffStart - gSpotLight.cosAngle));
        
        float NdotLSpotLight = dot(normalize(input.normal), -spotLightDirectionOnSurFace);
        float cosSpotLight = pow(NdotLSpotLight * 0.5f + 0.5f, 2.0f);
        
        float3 halfVectorSpotLight = normalize(-gSpotLight.direction + toEye);
        float NDotHSpotLight = dot(normalize(input.normal), halfVectorSpotLight);
        
        float specularPowSpotLight = pow(saturate(NDotHSpotLight), gMaterial.shininess);
        
        //float32_t attenuationFactor = saturate((cosAngle - gSpotLight.distance) / (1.0f - gSpotLight.distance));
        
        float spotLightdistance = length(gSpotLight.position - input.worldPosition); // ポイントライトへの距離
        float attenuationFactor = pow(saturate(-spotLightdistance / gSpotLight.distance + 1.0f), gSpotLight.dacay); // 逆に上による減衰係数
        
         // 環境マップ
        float3 cameraToPosition = normalize(input.worldPosition - gCamera.worldPosition);
        float3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
        float4 environmentColor = gEnvironmentTexture.Sample(gSampler, reflectedVector);
        
        
        
        // baseColor はアルベドから取得したカラー（通常は sRGB → linear 変換済み）
        float3 baseColor = gMaterial.color.rgb * textureColor.rgb;
        float3 dielectricF0 = float3(0.04f, 0.04f, 0.04f); // 非金属のスペキュラー反射率

        // metallic によって補間：金属なら baseColor、非金属なら 0.04
        float3 metal = lerp(dielectricF0, baseColor, metallic);
        
        // DirectionalLight
        // 拡散反射
        float3 diffuseDirectionalLight = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        
        //diffuseDirectionalLight = diffuseDirectionalLight * (1.0f - metallic); // 金属は拡散反射しない
        
        float3 environmentColorDirectionalLight = (environmentColor.rgb * metallic) * diffuseDirectionalLight;
        
        // 鏡面反射                                                                                      ↓ 物体の鏡面反射の色。ここでは白にしている materialで設定できたりすると良い
        //float3 specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * gDirectionalLight.specularColor;
        float3 specularDirectionalLight = TorranceSparrow(-gDirectionalLight.direction, toEye, normal, roughness, gDirectionalLight.specularColor) * gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        
        //float3 directionalLight = environmentColorDirectionalLight + (diffuseDirectionalLight * (1.0f - Flusnel(toEye, normalize(-gDirectionalLight.direction + toEye), gMaterial.specularColor))) + specularDirectionalLight;
        float3 directionalLight = environmentColorDirectionalLight + diffuseDirectionalLight + specularDirectionalLight;
        
        // PointLight
        // 拡散反射
        float3 diffusePointLight = gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * cos * gPointLight.intensity * factor;
        
        // 鏡面反射                                                                                      ↓ 物体の鏡面反射の色。ここでは白にしている materialで設定できたりすると良い
        //float3 specularPointLight = gPointLight.color.rgb * gPointLight.intensity * factor * specularPowPointLight * gPointLight.specularColor.rgb;
        float3 specularPointLight = TorranceSparrow(-pointLightDirection, toEye, normal, roughness, gPointLight.specularColor) * gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * cosPointLight * gPointLight.intensity * factor;
        
        float3 environmentColorPointLight = (environmentColor.rgb * metallic) * diffusePointLight;
        
        float3 pointLight = environmentColorPointLight + diffusePointLight + specularPointLight;
        
        // SpotLight
         // 拡散反射
        //float32_t3 diffuseSpotLight = gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * cosSpotLight * gSpotLight.intensity * falloffFactor * attenuationFactor;
        float3 diffuseSpotLight = gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * cosSpotLight * gSpotLight.intensity * falloffFactor * attenuationFactor;
        
        // 鏡面反射                                                                                      ↓ 物体の鏡面反射の色。ここでは白にしている materialで設定できたりすると良い
        //float3 specularSpotLight = gSpotLight.color.rgb * gSpotLight.intensity * attenuationFactor * gMaterial.specularColor * gSpotLight.specularColor * specularPowSpotLight;
        float3 specularSpotLight = TorranceSparrow(-gSpotLight.direction, toEye, normal, roughness, gSpotLight.specularColor) * gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * cosSpotLight * gSpotLight.intensity * falloffFactor * attenuationFactor;
        
        float3 environmentColorSpotLight = (environmentColor.rgb * metallic) * diffuseSpotLight;
        
        float3 spotLight = environmentColorSpotLight + diffuseSpotLight + specularSpotLight;
        
        float3 ambient = float3(0.05f, 0.05f, 0.06f);
        
        ambient = ambient * gMaterial.color.rgb * textureColor.rgb;
        
        
        // 拡散反射 + 鏡面反射
        output.color.rgb = ambient + directionalLight + pointLight + spotLight;
        // アルファは今まで通り
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    { // Lightingしない場合。前回までと同じ計算
        output.color = gMaterial.color * textureColor;
    }
    output.color.rgb = RoadMaterialTemplate(output, input).color.rgb;

    
    if (output.color.a < 0.2f)
    {
        discard;
    }
    
    return output;
}