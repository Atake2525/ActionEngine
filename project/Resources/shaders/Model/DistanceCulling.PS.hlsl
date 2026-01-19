static float DistanceCulling(float3 worldPos, float3 cameraPos, float nearDistance, float farDistance)
{
    float dist = length(worldPos - cameraPos);
    float range = max(farDistance - nearDistance, 1e-6); // ƒ[ƒœZ‰ñ”ğ
    float normalizedDistance = saturate((dist - nearDistance) / range - 1.0f);
    return 1.0f - normalizedDistance; // 0=near, 1=far
}

static float HeightCulling(float3 worldPos, float drawHeight)
{
    return saturate(drawHeight - worldPos.y);   
}

//static float CameraShadow(float3 worldPos, float3 cameraPos, float nearDistance, float farDistance)
//{
//    float dist = length(worldPos - cameraPos);
//    float range = max(farDistance - nearDistance, 1e-6); // ƒ[ƒœZ‰ñ”ğ
//    float normalizedDistance = saturate((dist - nearDistance) / range - 1.0f);
//    normalizedDistance *= 10.0f;
//    normalizedDistance = round(normalizedDistance);
    
//}