static float DistanceCulling(float3 worldPos, float3 cameraPos, float nearDistance, float farDistance)
{
    float dist = length(worldPos - cameraPos);
    float range = max(farDistance - nearDistance, 1e-6); // ゼロ除算回避
    float normalizedDistance = saturate((dist - nearDistance) / range - 1.0f);
    return 1.0f - normalizedDistance; // 0=near, 1=far
}

static float HeightCulling(float3 worldPos, float drawHeight)
{
    return saturate(drawHeight - worldPos.y);   
}

static float3 ScanEffect(float3 baseColor, float3 scanColor, float3 worldPos, float3 cameraPos, float scanWidth, float scanRadius)
{
    float dist = length(worldPos - cameraPos);

    float edge = exp(-abs(dist - scanRadius) * 20.0);

    float scan = smoothstep(scanRadius - scanWidth, scanRadius, dist) * (1 - smoothstep(scanRadius, scanRadius + scanWidth, dist));

    float intensity = saturate(scan * 1.5 + edge * 2.0);
    
    float3 finalColor = baseColor + scanColor * intensity;
    
    return finalColor;
}