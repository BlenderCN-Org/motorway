#include <BRDF.hlsli>
#include <CascadedShadowMapHelpers.hlsli>
#include <Atmosphere/Atmosphere.hlsli>

#include <ImageEffects/SeparableSSS.h>

static const float PI = 3.1415926535897932384626433f;
static const float INV_PI = ( 1.0 / PI );

cbuffer AtmosphereBuffer : register( b5 )
{
    float3  g_EarthCenter;
    float   g_SunSizeX;
    float3  g_SunDirection;
    float   g_SunSizeY;
};

struct LightSurfaceInfos
{
    float3  V;
    float   AmbientOcclusion;

    float3  N;
    float   Roughness;

    float3  Albedo;
    float   Metalness;

    float3  FresnelColor;
    float   F90;

    float3  PositionWorldSpace;
    float   LinearRoughness;

    float3  R;
    float   DisneyRoughness;

    float3  SecondaryNormal;
    float   NoV;

    float   ClearCoat;
    float   ClearCoatGlossiness;
    float   SubsurfaceScatteringStrength; // 0..1 range
    uint    PADDING;
};

float rectangleSolidAngle( float3 worldPos, float3 p0, float3 p1, float3 p2, float3 p3 )
{
    float3 v0 = p0 - worldPos;
    float3 v1 = p1 - worldPos;
    float3 v2 = p2 - worldPos;
    float3 v3 = p3 - worldPos;

    float3 n0 = normalize( cross( v0, v1 ) );
    float3 n1 = normalize( cross( v1, v2 ) );
    float3 n2 = normalize( cross( v2, v3 ) );
    float3 n3 = normalize( cross( v3, v0 ) );

    float g0 = acos( dot( -n0, n1 ) );
    float g1 = acos( dot( -n1, n2 ) );
    float g2 = acos( dot( -n2, n3 ) );
    float g3 = acos( dot( -n3, n0 ) );

    return g0 + g1 + g2 + g3 - 2 * PI;
}

float Trace_plane( float3 o, float3 d, float3 planeOrigin, float3 planeNormal )
{
    return dot( planeNormal, ( planeOrigin - o ) / dot( planeNormal, d ) );
}

float Trace_triangle( float3 o, float3 d, float3 A, float3 B, float3 C )
{
    float3 planeNormal = normalize( cross( B - A, C - B ) );
    float t = Trace_plane( o, d, A, planeNormal );
    float3 p = o + d*t;

    float3 N1 = normalize( cross( B - A, p - B ) );
    float3 N2 = normalize( cross( C - B, p - C ) );
    float3 N3 = normalize( cross( A - C, p - A ) );

    float d0 = dot( N1, N2 );
    float d1 = dot( N2, N3 );

    float threshold = 1.0f - 0.001f;
    return ( d0 > threshold && d1 > threshold ) ? 1.0f : 0.0f;
}

float Trace_rectangle( float3 o, float3 d, float3 A, float3 B, float3 C, float3 D )
{
    return max( Trace_triangle( o, d, A, B, C ), Trace_triangle( o, d, C, D, A ) );
}

float3 ClosestPointOnSegment( float3 a, float3 b, float3 c )
{
    float3 ab = b - a;
    float t = dot( c - a, ab ) / dot( ab, ab );
    return a + saturate( t ) * ab;
}

float3 ClosestPointOnLine( float3 a, float3 b, float3 c )
{
    float3 ab = b - a;
    float t = dot( c - a, ab ) / dot( ab, ab );
    return a + t * ab;
}

float illuminanceSphereOrDisk( float cosTheta, float sinSigmaSqr )
{
    float sinTheta = sqrt( 1.0f - cosTheta * cosTheta );
    float illuminance = 0.0f;

    if ( cosTheta * cosTheta > sinSigmaSqr ) {
        illuminance = PI * sinSigmaSqr * saturate( cosTheta );
    } else {
        float x = sqrt( 1.0f / sinSigmaSqr - 1.0f ); // For a disk this simplify to x = d / r
        float y = -x * ( cosTheta / sinTheta );
        float sinThetaSqrtY = sinTheta * sqrt( 1.0f - y * y );
        illuminance = ( cosTheta * acos( y ) - x * sinThetaSqrtY ) * sinSigmaSqr + atan( sinThetaSqrtY / x );
    }

    return max( illuminance, 0.0f );
}

float3 getSpecularDominantDirArea( float3 N, float3 R, float roughness )
{
    return normalize( lerp( N, R, ( 1 - roughness ) ) );
}

float TracePlane( float3 o, float3 d, float3 planeOrigin, float3 planeNormal )
{
    return dot( planeNormal, ( planeOrigin - o ) / dot( planeNormal, d ) );
}

float3 GetDirectionalLightIlluminance( in DirectionalLight light, in LightSurfaceInfos surface, in float depth, inout float3 L )
{
    float r = sin( light.SunColorAndAngularRadius.w );
    float d = cos( light.SunColorAndAngularRadius.w );

    float DoR = dot( light.SunDirectionAndIlluminanceInLux.xyz, surface.R );
    float3 S = surface.R - DoR * light.SunDirectionAndIlluminanceInLux.xyz;

    L = ( DoR < d ) ? normalize( d * light.SunDirectionAndIlluminanceInLux.xyz + normalize( S ) * r ) : surface.R;

    float illuminance = light.SunDirectionAndIlluminanceInLux.w * saturate( dot( surface.N, light.SunDirectionAndIlluminanceInLux.xyz ) );

    // Add shadow
    // We assume the surface is lit if not covered by the shadow map
    float3 shadowVisibility = 1.0f;
    float3 surfaceTransmittance = float3( 0, 0, 0 );
    
#ifndef PA_DONT_RECEIVE_SHADOWS
    // Figure out which cascade to sample from
    uint cascadeIdx = ~0;

    if ( depth <= ShadowSplitDistances.x ) cascadeIdx = 0;
    else if ( depth <= ShadowSplitDistances.y ) cascadeIdx = 1;
    else if ( depth <= ShadowSplitDistances.z ) cascadeIdx = 2;
    else if ( depth <= ShadowSplitDistances.w ) cascadeIdx = 3;

    [branch]
    if ( cascadeIdx <= 3 ) {
        float NoL = saturate( dot( surface.N, L ) );

        // Apply offset
        float3 offset = GetCascadedShadowMapShadowPosOffset( NoL, surface.N ) / abs( CascadeScales[cascadeIdx].z );

        // Project into shadow space
        float3 samplePos = surface.PositionWorldSpace + offset;
        float3 shadowPosition = mul( float4( samplePos, 1.0f ), ShadowMatrixShared ).xyz;
        float3 shadowPosDX = ddx_fine( shadowPosition );
        float3 shadowPosDY = ddy_fine( shadowPosition );

        shadowVisibility = SampleShadowCascade( light, shadowPosition, shadowPosDX, shadowPosDY, cascadeIdx );

        // Make sure the value is within a valid range (avoid NaN)
        shadowVisibility = saturate( shadowVisibility );

        // Sample the next cascade, and blend between the two results to
        // smooth the transition
        float nextSplit = ShadowSplitDistances[cascadeIdx];
        float splitSize = cascadeIdx == 0 ? nextSplit : nextSplit - ShadowSplitDistances[cascadeIdx - 1];
        float fadeFactor = ( nextSplit - depth ) / splitSize;

        [branch]
        if ( fadeFactor <= CSM_SLICE_BLEND_THRESHOLD && cascadeIdx != CSM_SLICE_COUNT - 1 ) {
            float3 nextSplitVisibility = SampleShadowCascade( light, shadowPosition, shadowPosDX, shadowPosDY, cascadeIdx + 1 );
            float lerpAmt = smoothstep( 0.0f, CSM_SLICE_BLEND_THRESHOLD, fadeFactor );
            shadowVisibility = lerp( nextSplitVisibility, shadowVisibility, lerpAmt );
            shadowVisibility = saturate( shadowVisibility );
        }
    }
    
    [branch]
    if ( surface.SubsurfaceScatteringStrength > 0.0f ) { 
        float NoL = saturate( dot( surface.N, L ) );

        // Apply offset
        float3 offset = GetCascadedShadowMapShadowPosOffset( NoL, surface.N ) / abs( CascadeScales[cascadeIdx].z );

        // Project into shadow space
        float3 samplePos = surface.PositionWorldSpace + offset;
        float4 shrinkedPos = float4(surface.PositionWorldSpace - 0.005 * surface.N, 1.0);
        float3 ssShadowPosition = mul( shrinkedPos, ShadowMatrixShared ).xyz;
        float3 ssShadowPosDX = ddx_fine( ssShadowPosition );
        float3 ssShadowPosDY = ddy_fine( ssShadowPosition );

        float d1 = SampleShadowCascade( light, ssShadowPosition, ssShadowPosDX, ssShadowPosDY, cascadeIdx ).g; //SSSSSample(shadowMap, shadowPosition.xy / shadowPosition.w).r; // 'd1' has a range of 0..1
        d1 = saturate( d1 );
        
        surfaceTransmittance = SSSSTransmittance( surface.SubsurfaceScatteringStrength, 0.005f, surface.N, L, ( d1 * 128.0f ), ssShadowPosition.z );
    }    
#endif

    // Get Sun Irradiance
    // NOTE Do not add sky irradiance since we already have IBL as ambient term
    float3 skyIrradiance = float3( 0, 0, 0 );
    float3 sunIrradiance = GetSunAndSkyIrradiance( surface.PositionWorldSpace.xzy * 1.0 - g_EarthCenter, surface.N.xzy, g_SunDirection, skyIrradiance );

    float3 lightIlluminance = ( sunIrradiance * illuminance * shadowVisibility );
    
    return lightIlluminance + ( lightIlluminance * surfaceTransmittance );
}

float3 GetPointLightIlluminance( in PointLight light, in LightSurfaceInfos surface, in float depth, inout float3 L )
{
    float3 unormalizedL = light.PositionAndRadius.xyz - surface.PositionWorldSpace;
    float distance = length( unormalizedL );
    L = normalize( unormalizedL );

    float illuminance = pow( saturate( 1 - pow( ( distance / light.PositionAndRadius.w ), 4 ) ), 2 ) / ( distance * distance + 1 );
    float luminancePower = light.ColorAndPowerInLux.a / ( 4.0f * sqrt( light.PositionAndRadius.w * PI ) );

    return ( light.ColorAndPowerInLux.rgb ) * luminancePower * illuminance;
}

float3 GetSpotLightIlluminance( in SpotLight light, in LightSurfaceInfos surface, in float depth, inout float3 L )
{
    float3 unormalizedL = light.PositionAndRadius.xyz - surface.PositionWorldSpace;
    float distance = length( unormalizedL );
    L = normalize( unormalizedL );

    float ConeAngleFalloff = saturate( ( dot( L, -light.LightDirectionAndFallOffRadius.xyz ) - light.LightDirectionAndFallOffRadius.w ) * light.InvCosConeDifference );
    ConeAngleFalloff = ConeAngleFalloff * ConeAngleFalloff;

    float DistanceAttenuation = 1.0f;
    float DistanceSqr = dot( unormalizedL, unormalizedL );
    DistanceAttenuation = 1 / ( DistanceSqr + 1 );

    float lightInvRadius = 1 / light.Radius;
    float truc = ( DistanceSqr * lightInvRadius * lightInvRadius );
    truc = truc * truc;
    float LightRadiusMask = saturate( 1 - truc );
    LightRadiusMask = LightRadiusMask * LightRadiusMask;

    DistanceAttenuation *= LightRadiusMask;

    float illuminance = ConeAngleFalloff * DistanceAttenuation;

    float falloff = pow( saturate( 1 - pow( ( distance / light.PositionAndRadius.w ), 4 ) ), 2 ) / ( distance * distance + 1 );

    float luminancePower = light.ColorAndPowerInLux.a / ( 4.0f * sqrt( light.Radius * PI ) );

    return ( light.ColorAndPowerInLux.rgb ) * luminancePower * illuminance;
}

float3 GetSphereLightIlluminance( in SphereLight light, in LightSurfaceInfos surface, in float depth, inout float3 L )
{
    float3 unormalizedL = light.PositionAndRadius.xyz - surface.PositionWorldSpace;
    L = normalize( unormalizedL );
    float sqrDist = dot( unormalizedL, unormalizedL );

    float cosTheta = clamp( dot( surface.N, L ), -0.999, 0.999 );
    float sqrLightRadius = light.PositionAndRadius.w * light.PositionAndRadius.w;
    float sinSigmaSqr = min( sqrLightRadius / sqrDist, 0.9999f );

    float illuminance = illuminanceSphereOrDisk( cosTheta, sinSigmaSqr );
    float luminancePower = light.ColorAndPowerInLux.a / ( 4.0f * sqrt( light.PositionAndRadius.w * PI ) );

    float3 r = getSpecularDominantDirArea( surface.N, surface.R, surface.Roughness );

    float3 centerToRay = dot( unormalizedL, r ) * r - unormalizedL;
    float3 closestPoint = unormalizedL + centerToRay * saturate( light.PositionAndRadius.w / length( centerToRay ) );
    L = normalize( closestPoint );

    return ( light.ColorAndPowerInLux.rgb ) * luminancePower * illuminance;
}

float3 GetDiscLightIlluminance( in DiscLight light, in LightSurfaceInfos surface, in float depth, inout float3 L )
{
    float3 unormalizedL = light.PositionAndRadius.xyz - surface.PositionWorldSpace;
    L = normalize( unormalizedL );
    float sqrDist = dot( unormalizedL, unormalizedL );

    const float cosTheta = dot( surface.N, L );
    float sqrLightRadius = light.PositionAndRadius.w * light.PositionAndRadius.w;

    // Do not let the surface penetrate the light
    float sinSigmaSqr = sqrLightRadius / ( sqrLightRadius + max( sqrLightRadius, sqrDist ) );

    // Multiply by saturate ( dot ( planeNormal , -L)) to better match ground truth .
    float illuminance = illuminanceSphereOrDisk( cosTheta, sinSigmaSqr ) * saturate( dot( light.PlaneNormal.xyz, -L ) );
    float luminancePower = light.ColorAndPowerInLux.a / ( sqrt( light.PositionAndRadius.w * PI ) );

    float3 r = getSpecularDominantDirArea( surface.N, surface.R, surface.Roughness );

    float t = TracePlane( surface.PositionWorldSpace, r, light.PositionAndRadius.xyz, light.PlaneNormal.xyz );
    float3 p = surface.PositionWorldSpace + r * t;
    float3 centerToRay = p - light.PositionAndRadius.xyz;
    float3 closestPoint = unormalizedL + centerToRay * saturate( light.PositionAndRadius.w / length( centerToRay ) );
    L = normalize( closestPoint );

    return ( light.ColorAndPowerInLux.rgb ) * luminancePower * illuminance;
}

float3 GetRectangleLightIlluminance( in RectangleLight light, in LightSurfaceInfos surface, in float depth, inout float3 L )
{
    float illuminance = 0.0f;

    float3 unormalizedL = light.PositionAndRadius.xyz - surface.PositionWorldSpace;
    float dist = length( unormalizedL );
    L = unormalizedL / dist;

    float halfWidth = light.Width * 0.5;
    float halfHeight = light.Height * 0.5;

    float3 p0 = light.PositionAndRadius.xyz + light.LeftVector * -halfWidth + light.UpVector * halfHeight;
    float3 p1 = light.PositionAndRadius.xyz + light.LeftVector * -halfWidth + light.UpVector * -halfHeight;
    float3 p2 = light.PositionAndRadius.xyz + light.LeftVector * halfWidth + light.UpVector * -halfHeight;
    float3 p3 = light.PositionAndRadius.xyz + light.LeftVector * halfWidth + light.UpVector * halfHeight;

    float solidAngle = rectangleSolidAngle( surface.PositionWorldSpace, p0, p1, p2, p3 );

    if ( dot( surface.PositionWorldSpace - light.PositionAndRadius.xyz, light.PlaneNormal.xyz ) > 0 ) {
        illuminance = solidAngle * 0.2 * (
            saturate( dot( normalize( p0 - surface.PositionWorldSpace ), surface.N ) ) +
            saturate( dot( normalize( p1 - surface.PositionWorldSpace ), surface.N ) ) +
            saturate( dot( normalize( p2 - surface.PositionWorldSpace ), surface.N ) ) +
            saturate( dot( normalize( p3 - surface.PositionWorldSpace ), surface.N ) ) +
            saturate( dot( normalize( unormalizedL ), surface.N ) ) );
    }

    {
        float3 r = getSpecularDominantDirArea( surface.N, surface.R, surface.Roughness );
        float traced = Trace_rectangle( surface.PositionWorldSpace, r, p0, p1, p2, p3 );

        [branch]
        if ( traced > 0 ) {
            // Trace succeeded so the light vector L is the reflection vector itself
            L = r;
        } else {
            // The trace didn't succeed, so we need to find the closest point to the ray on the rectangle

            // We find the intersection point on the plane of the rectangle
            float3 tracedPlane = surface.PositionWorldSpace + r * Trace_plane( surface.PositionWorldSpace, r, light.PositionAndRadius.xyz, light.PlaneNormal.xyz );

            // Then find the closest point along the edges of the rectangle (edge = segment)
            float3 PC[4] = {
                ClosestPointOnSegment( p0, p1, tracedPlane ),
                ClosestPointOnSegment( p1, p2, tracedPlane ),
                ClosestPointOnSegment( p2, p3, tracedPlane ),
                ClosestPointOnSegment( p3, p0, tracedPlane ),
            };
            float dist[4] = {
                distance( PC[0], tracedPlane ),
                distance( PC[1], tracedPlane ),
                distance( PC[2], tracedPlane ),
                distance( PC[3], tracedPlane ),
            };

            float3 min = PC[0];
            float minDist = dist[0];
            [unroll]
            for ( uint iLoop = 1; iLoop < 4; iLoop++ ) {
                if ( dist[iLoop] < minDist ) {
                    minDist = dist[iLoop];
                    min = PC[iLoop];
                }
            }

            L = min - surface.PositionWorldSpace;
        }
        L = normalize( L ); // TODO: Is it necessary?
    }

    float luminancePower = light.ColorAndPowerInLux.a / ( light.Width * light.Height * PI );

    return ( light.ColorAndPowerInLux.rgb ) * luminancePower * illuminance;
}

float3 GetTubeLightIlluminance( in RectangleLight light, in LightSurfaceInfos surface, in float depth, inout float3 L )
{
    float3 unormalizedL = light.PositionAndRadius.xyz - surface.PositionWorldSpace;
    float dist = length( unormalizedL );
    L = normalize( unormalizedL / dist );
    float sqrDist = dot( unormalizedL, unormalizedL );

    const float halfWidth = light.Width * 0.5f;

    float3 P0 = light.PositionAndRadius.xyz - light.LeftVector * halfWidth;
    float3 P1 = light.PositionAndRadius.xyz + light.LeftVector * halfWidth;

    float3 forward = normalize( ClosestPointOnLine( P0, P1, surface.PositionWorldSpace ) - surface.PositionWorldSpace );
    float3 up = cross( light.LeftVector, forward );

    float3 p0 = light.PositionAndRadius.xyz - light.LeftVector * halfWidth + light.Height * up;
    float3 p1 = light.PositionAndRadius.xyz - light.LeftVector * halfWidth - light.Height * up;
    float3 p2 = light.PositionAndRadius.xyz + light.LeftVector * halfWidth - light.Height * up;
    float3 p3 = light.PositionAndRadius.xyz + light.LeftVector * halfWidth + light.Height * up;

    float solidAngle = rectangleSolidAngle( surface.PositionWorldSpace, p0, p1, p2, p3 );

    float illuminance = solidAngle * 0.2 * (
        saturate( dot( normalize( p0 - surface.PositionWorldSpace ), surface.N ) ) +
        saturate( dot( normalize( p1 - surface.PositionWorldSpace ), surface.N ) ) +
        saturate( dot( normalize( p2 - surface.PositionWorldSpace ), surface.N ) ) +
        saturate( dot( normalize( p3 - surface.PositionWorldSpace ), surface.N ) ) +
        saturate( dot( normalize( light.PositionAndRadius.xyz - surface.PositionWorldSpace ), surface.N ) ) );

    float3 spherePosition = ClosestPointOnSegment( P0, P1, surface.PositionWorldSpace );
    float3 sphereUnormL = spherePosition - surface.PositionWorldSpace;
    float3 sphereL = normalize( sphereUnormL );
    float sqrSphereDistance = dot( sphereUnormL, sphereUnormL );

    float fLightSphere = PI * saturate( dot( sphereL, surface.N ) ) * ( ( light.Height * light.Height ) / sqrSphereDistance );

    illuminance += fLightSphere;

    float3 r = getSpecularDominantDirArea( surface.N, surface.R, surface.Roughness );

    // First, the closest point to the ray on the segment
    float3 L0 = P0 - surface.PositionWorldSpace;
    float3 L1 = P1 - surface.PositionWorldSpace;
    float3 Ld = L1 - L0;

    float t = dot( r, L0 ) * dot( r, Ld ) - dot( L0, Ld );
    t /= dot( Ld, Ld ) - sqrt( dot( r, Ld ) );

    L = ( L0 + saturate( t ) * Ld );

    float3 centerToRay = max( float3( 0.001, 0.001, 0.001 ), dot( L, r ) * r - L );
    float3 closestPoint = L + centerToRay * saturate( light.Height / length( centerToRay ) );
    L = normalize( closestPoint );

    float luminancePower = light.ColorAndPowerInLux.a / ( 2.0f * PI * light.Width * light.Height + 4.0f * sqrt( light.Height * PI ) );

    return ( light.ColorAndPowerInLux.rgb ) * luminancePower * illuminance;
}
