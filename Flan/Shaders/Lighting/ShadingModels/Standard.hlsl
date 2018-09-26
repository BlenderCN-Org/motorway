#include <Lighting.hlsli>

float3 DoShading( in float3 L, in LightSurfaceInfos surface )
{
    const float3 H = normalize( surface.V + L );
    const float LoH = saturate( dot( L, H ) );
    const float NoL = saturate( dot( surface.N, L ) );
    const float NoH = saturate( dot( surface.N, H ) );

    // Diffuse BRDF
    float diffuse = Diffuse_Disney( surface.NoV, NoL, LoH, surface.DisneyRoughness ) * INV_PI;

#ifndef PA_PROBE_CAPTURE
    // Specular BRDF
    float3 fresnel = Fresnel_Schlick( surface.FresnelColor, surface.F90, LoH );
    float visibility = Visibility_SmithGGXCorrelated( surface.NoV, NoL, surface.LinearRoughness );
    float distribution = Distribution_GGX( NoH, surface.LinearRoughness );

    float3 specular = ( distribution * fresnel * visibility ) * INV_PI;

    return ( ( diffuse * surface.Albedo ) + specular ) * NoL;
#else
    // Disable specular for probe capture; use reflectance color to avoid pitch black surfaces (e.g. metallic materials)
    float3 surfaceColor = surface.FresnelColor + surface.Albedo;
    return diffuse * surfaceColor;
#endif
}
