#ifndef __BRDF_H__
#define __BRDF_H__ 1

#include <MathsHelpers.hlsli>

float3 Fresnel_Schlick( in float3 f0, in float f90, in float u )
{
    return f0 + ( f90 - f0 ) * pow( 1.0f - u, 5.0f );
}

float Diffuse_LambertWrapped( in float NoL, in float linearRoughness )
{
	return saturate( ( NoL + linearRoughness ) / ( ( 1 + linearRoughness ) * ( 1 + linearRoughness ) ) ) * INV_PI;
}

float Diffuse_Disney( in float NoV, in float NoL, in float LoH, in float linearRoughness )
{
    float energyBias = lerp( 0.0, 0.5, linearRoughness ); 
    float energyFactor = lerp( 1.0, 1.0 / 1.51, linearRoughness );
    float fd90 = energyBias + 2.0 * LoH * LoH * linearRoughness;
    
    const float3 f0 = float3( 1.0f, 1.0f, 1.0f );
    float lightScatter = Fresnel_Schlick( f0, fd90, NoL ).r;
    float viewScatter = Fresnel_Schlick( f0, fd90, NoV ).r;

    return ( lightScatter * viewScatter * energyFactor );
}

float Visibility_Schlick( in float Roughness, in float NoV, in float NoL )
{
    float k = Square( Roughness ) * 0.5;
    float Vis_SchlickV = NoV * ( 1 - k ) + k;
    float Vis_SchlickL = NoL * ( 1 - k ) + k;
    
    return 0.25 / ( Vis_SchlickV * Vis_SchlickL );
}

float Visibility_SmithGGXCorrelated( in float NdotL, in float NdotV, float alphaG )
{
    // This is the optimized version
    float alphaG2 = Square( alphaG );
    
    // Caution : the " NdotL *" and " NdotV *" are explicitely inversed , this is not a mistake .
    float Lambda_GGXV = NdotL * sqrt( ( -NdotV * alphaG2 + NdotV ) * NdotV + alphaG2 );
    float Lambda_GGXL = NdotV * sqrt( ( -NdotL * alphaG2 + NdotL ) * NdotL + alphaG2 );

    return 0.5f / ( Lambda_GGXV + Lambda_GGXL );
}

float Visibility_Kelemen( float LoH )
{
    return rcp( 4 * Square( LoH ) );
}

float Distribution_GGX( in float NoH, in float linearRoughness )
{
    float linearRoughnessSquared = Square( linearRoughness );
    float NoHSquared = NoH * NoH;
    float d = NoHSquared * ( linearRoughnessSquared - 1.0f ) + 1.0f;

    return linearRoughnessSquared / ( d * d );
}

float3 getDiffuseDominantDir( float3 N, float3 V, float NdotV, float roughness )
{
    float a = 1.02341f * roughness - 1.51174f;
    float b = -0.511705f * roughness + 0.755868f;
    float lerpFactor = saturate( ( NdotV * a + b ) * roughness );

    // The result is not normalized as we fetch in a cubemap
    return lerp( N, V, lerpFactor );
}

float3 getSpecularDominantDir( float3 N, float3 R, float roughness )
{
    float smoothness = saturate( 1 - roughness );
    float lerpFactor = smoothness * ( sqrt( smoothness ) + roughness );

    // The result is not normalized as we fetch in a cubemap
    return lerp( N, R, lerpFactor );
}

inline float linearRoughnessToMipLevel( float linearRoughness, int mipCount )
{
    return ( sqrt( linearRoughness ) * mipCount );
}

inline float computeSpecOcclusion( float NoV, float AO, float roughness )
{
    return saturate( pow( NoV + AO, exp2( -16.0f * roughness - 1.0f ) ) - 1.0f + AO );
}

float computeDistanceBaseRoughness( float distInteresectionToShadedPoint, float distInteresectionToProbeCenter, float linearRoughness )
{
    float newLinearRoughness = clamp( distInteresectionToShadedPoint / distInteresectionToProbeCenter * linearRoughness, 0.0, linearRoughness );
    return lerp( newLinearRoughness, linearRoughness, linearRoughness );
}
#endif
