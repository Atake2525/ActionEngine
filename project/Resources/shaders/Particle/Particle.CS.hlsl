struct Particle
{
    float3 transalte;
    float3 scale;
    float lifeTime;
    float3 velocity;
    float currentTime;
    float4 color;
};
static const uint maxParticles = 1024;
RWStructuredBuffer<Particle> gParticles : register(u0);


[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint particleIndex = DTid.x;
    if (particleIndex < maxParticles)
    {
        // Particle構造体の全要素を0で埋める
        gParticles[particleIndex] = (Particle) 0;

    }
}