#include "object3d.hlsli"

Texture2D<float4> gTexture : register(t0);
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
};
ConstantBuffer<MaterialTemplate> gMaterialTemplate : register(b5);

static PixelShaderOutput RoadMaterialTemplate(PixelShaderOutput output, VertexShaderOutput input)
{
    // 環境マップ
    float3 cameraToPosition = normalize(input.worldPosition - gCamera.worldPosition);
    float3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
    float4 environmentColor = gEnvironmentTexture.Sample(gSampler, reflectedVector);
    if (gMaterial.enableMetallic)
    {
        output.color.rgb += environmentColor.rgb * gMaterialTemplate.metallic;
    }
    else
    {
        output.color.rgb += environmentColor.rgb * gMaterial.environmentCoefficient;
    }
    return output;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gMaterial.color;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
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
        
        // DirectionalLight
        // 拡散反射
        float3 diffuseDirectionalLight = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        
        // 鏡面反射                                                                                      ↓ 物体の鏡面反射の色。ここでは白にしている materialで設定できたりすると良い
        float3 specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * gDirectionalLight.specularColor;
        
        // PointLight
        // 拡散反射
        float3 diffusePointLight = gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * cos * gPointLight.intensity * factor;
        
        // 鏡面反射                                                                                      ↓ 物体の鏡面反射の色。ここでは白にしている materialで設定できたりすると良い
        float3 specularPointLight = gPointLight.color.rgb * gPointLight.intensity * factor * specularPowPointLight * gPointLight.specularColor.rgb;
        
        // SpotLight
         // 拡散反射
        //float32_t3 diffuseSpotLight = gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * cosSpotLight * gSpotLight.intensity * falloffFactor * attenuationFactor;
        float3 diffuseSpotLight = gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * cosSpotLight * gSpotLight.intensity * falloffFactor * attenuationFactor;
        
        // 鏡面反射                                                                                      ↓ 物体の鏡面反射の色。ここでは白にしている materialで設定できたりすると良い
        //float32_t3 specularSpotLight = gSpotLight.color.rgb * gSpotLight.intensity * attenuationFactor * falloffFactor * gSpotLight.specularColor * specularPow;
        float3 specularSpotLight = gSpotLight.color.rgb * gSpotLight.intensity * attenuationFactor * gMaterial.specularColor * gSpotLight.specularColor * specularPowSpotLight;
        
        
        // 拡散反射 + 鏡面反射
        output.color.rgb = diffuseDirectionalLight + specularDirectionalLight + diffusePointLight + specularPointLight + diffuseSpotLight + specularSpotLight;
        // アルファは今まで通り
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    { // Lightingしない場合。前回までと同じ計算
        output.color = gMaterial.color * textureColor;
    }
    output.color.rgb = RoadMaterialTemplate(output, input).color.rgb;

    
    return output;
}