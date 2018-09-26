#include <Lighting.hlsli>

float3 DoShading( in float3 L, in LightSurfaceInfos surface )
{
    return surface.Albedo;
}
