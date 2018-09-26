#include <Lighting.hlsli>

float3 DoShading( in float3 L, in LightSurfaceInfos surface )
{
    const float NoL = saturate( dot( surface.N, L ) );
    return surface.Albedo * NoL;
}

float3 GetIndirectLighting( in LightSurfaceInfos surface, in EnvironmentProbe probe )
{
    return float3( 0, 0, 0 );
}
