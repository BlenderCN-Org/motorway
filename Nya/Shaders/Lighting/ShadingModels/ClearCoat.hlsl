#include <BRDF.hlsli>

Texture2D g_DFGLUTStandard : register( t4 );

float3 DoShading( in float3 L, in LightSurfaceInfos surface )
{
    const float3 H = normalize( surface.V + L );
    const float LoH = saturate( dot( L, H ) );
    const float NoH = saturate( dot( surface.N, H ) );
    const float VoH = saturate( dot( surface.V, H ) ) + 1e-5f;

    // IOR = 1.5 -> F0 = 0.04
    static const float ClearCoatF0 = 0.04f;
    static const float ClearCoatIOR = 1.5f;
    static const float ClearCoatRefractionFactor = 1 / ClearCoatIOR;
    
    const float clearCoatRoughness = 1.0f - surface.ClearCoatGlossiness;
    const float clearCoatLinearRoughness = max( 0.01f, ( clearCoatRoughness * clearCoatRoughness ) );

    // 1. Compute the specular reflection off of the clear coat using the roughness and IOR (F0 specular intensity)
    float clearCoatDistribution = Distribution_GGX( NoH, clearCoatLinearRoughness );
    float clearCoatVisibility = Visibility_Kelemen( VoH );

    // 2. Compute the amount of light transmitted (refracted) into the clear coat using fresnel equations and IOR
    float Fc = pow( 1 - VoH, 5 );
    float clearCoatFresnel = Fc + ( 1 - Fc ) * ClearCoatF0;
    float clearCoatSpecular = ( clearCoatDistribution * clearCoatFresnel * clearCoatVisibility );

    // Refract rays
    float3 L2 = refract( -L, -H, ClearCoatRefractionFactor );
    float3 V2 = refract( -surface.V, -H, ClearCoatRefractionFactor );

    float3 H2 = normalize( V2 + L2 );
    float NoL2 = saturate( dot( surface.N, L2 ) ) + 1e-5f;
    float NoV2 = saturate( dot( surface.N, V2 ) ) + 1e-5f;
    float NoH2 = saturate( dot( surface.N, H2 ) );
    float LoH2 = saturate( dot( L2, H2 ) );

    float3 AbsorptionColor = ( 1 - surface.ClearCoat ) + surface.Albedo * ( surface.ClearCoat * ( 1 / 0.9 ) );
    float  AbsorptionDist = rcp( NoV2 ) + rcp( NoL2 );
    float3 Absorption = pow( AbsorptionColor, 0.5 * AbsorptionDist );

    float F21 = Fresnel_Schlick( ClearCoatF0, surface.F90, saturate( dot( V2, H ) ) );

    float k = Square( surface.Roughness ) * 0.5;
    float G_SchlickV2 = NoV2 / ( NoV2 * ( 1 - k ) + k );
    float G_SchlickL2 = NoL2 / ( NoL2 * ( 1 - k ) + k );
    float TotalInternalReflection = 1 - F21 * G_SchlickV2 * G_SchlickL2;

    float3 LayerAttenuation = ( ( 1.0f - clearCoatFresnel ) * TotalInternalReflection ) * Absorption;

    // Specular BRDF
    float3 fresnel = Fresnel_Schlick( 0.9, surface.F90, LoH2 );
    float visibility = Visibility_Schlick( surface.Roughness, NoV2, NoL2 );
    float distribution = Distribution_GGX( NoH2, surface.Roughness );

    float3 baseLayer = ( distribution * fresnel * visibility ) + ( INV_PI * surface.Albedo );

    return ( clearCoatSpecular + baseLayer * LayerAttenuation );
}

// float3 GetIndirectContribution()
// {
    // // Compute Specular Reflections for the ClearCoat Layer
    // // IOR = 1.5 -> F0 = 0.04
    // float ClearCoatF0 = 0.04f;
    // float ClearCoatRoughness = 1.0f - surface.ClearCoatGlossiness;
    // float ClearCoatLinearRoughness = ( ClearCoatRoughness * ClearCoatRoughness );
    // float ClearCoatNoV = saturate( dot( surface.V, surface.ClearCoatNormalMap ) ) + 1e-5f;

    // float Fc = pow( 1 - ClearCoatNoV, 5.0f );
    // float ClearCoatFresnel = Fc + ( 1 - Fc ) * ClearCoatF0;
    // ClearCoatFresnel *= surface.ClearCoat;

    // float3 ClearCoatR = reflect( -surface.V, surface.ClearCoatNormalMap );

    // float LightTransmitAmt = ( 1.0f - ClearCoatFresnel );

    // // Accentuate rim lighting (which is usually strong on clearcoat-like paints)
    // float3 diffuseSum, ClearCoatSpecular;
    // EvaluateIBL( diffuseSum, ClearCoatSpecular, nNextLightIndex, scaledTileIndex, surface.NoV, ClearCoatRoughness, ClearCoatLinearRoughness, surface.ClearCoatNormalMap, surface.V, ClearCoatR, surface.PositionWorldSpace, surface.AmbientOcclusion, float3( 0, 0, 0 ), ClearCoatF0, ( 1.0f - surface.ClearCoat ) );

    // float Mip = linearRoughnessToMipLevel( ClearCoatLinearRoughness, 8 );
    // float3 ClearCoatEnvironment = g_EnvProbeCaptureArray.SampleLevel( g_DiffuseEnvProbeSampler, float4( ClearCoatR, 0 ), Mip ).xyz;
    
    // float3 iblClearCoat = ( IndirectDiffuse + ( ( SpecularReflections * LightTransmitAmt ) + ClearCoatSpecular ) ) + ( ClearCoatEnvironment * ClearCoatFresnel );

    // LightContribution.rgb += iblClearCoat;
// }
