#define DISSOLVE_NO_MAIN
#include "Dissolve.PS.hlsl"

static const float SPEEDLINE_PI2 = 6.28318530718f;

struct SpeedLine
{
    float2 center;
    float intensity;
    float time;
    float lineCount;
    float speed;
    float innerRadius;
    float outerRadius;
    float4 color;
};
ConstantBuffer<SpeedLine> gSpeedLine : register(b4);

float SpeedLineHash(float value)
{
    return frac(sin(value) * 43758.5453123f);
}

float SpeedLinePattern(float angle01, float radius)
{
    float sector = floor(angle01 * max(gSpeedLine.lineCount, 1.0f));
    float frame = floor(gSpeedLine.time * gSpeedLine.speed);
    float randomValue = SpeedLineHash(sector * 12.9898f + frame * 78.233f);

    float pulsePhase = gSpeedLine.time * gSpeedLine.speed + radius * 28.0f + randomValue * SPEEDLINE_PI2;
    float pulse = sin(pulsePhase) * 0.5f + 0.5f;
    float lineDensity = smoothstep(0.62f, 1.0f, randomValue);
    float lineFlash = smoothstep(0.18f, 1.0f, pulse);

    return lineDensity * lineFlash;
}

float SpeedLineFlowMask(float radius)
{
    float flow = frac(radius * 3.0f - gSpeedLine.time * gSpeedLine.speed * 0.35f);
    return smoothstep(0.0f, 0.25f, flow) * (1.0f - smoothstep(0.75f, 1.0f, flow));
}

float SpeedLineAlpha(float2 uv)
{
    if (gSpeedLine.intensity <= 0.001f)
    {
        return 0.0f;
    }

    float2 toPixel = uv - gSpeedLine.center;
    float radius = length(toPixel);
    float angle01 = atan2(toPixel.y, toPixel.x) / SPEEDLINE_PI2 + 0.5f;

    float lineValue = SpeedLinePattern(angle01, radius);
    float radialMask = smoothstep(gSpeedLine.innerRadius, gSpeedLine.outerRadius, radius);
    float edgeMask = 1.0f - smoothstep(gSpeedLine.outerRadius, gSpeedLine.outerRadius + 0.35f, radius);
    float flowMask = SpeedLineFlowMask(radius);

    return saturate(lineValue * radialMask * edgeMask * flowMask * gSpeedLine.intensity * gSpeedLine.color.a);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output = ShadingDissolve(input);

    float alpha = SpeedLineAlpha(input.texcoord);
    output.color.rgb = lerp(output.color.rgb, output.color.rgb + gSpeedLine.color.rgb, alpha);

    return output;
}
