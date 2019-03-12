#include <BRDF.hlsli>

Texture2D g_DFGLUTStandard : register( t4 );

float3 DoShading( in float3 L, in LightSurfaceInfos surface )
{
    return surface.Albedo;
}
