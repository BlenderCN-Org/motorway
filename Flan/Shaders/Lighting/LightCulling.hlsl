/*
    Copyright (C) 2018 Team Horsepower
    https://github.com/ProjectHorsepower

    This file is part of Project Horsepower source code.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <RenderableEntities.hlsli>
#include <CameraData.hlsli>

cbuffer PassData : register( b2 )
{
    uint2 BackbufferDimension;
}

uint GetNumTilesX()
{
    return ( uint )( ( BackbufferDimension.x + TILE_RES - 1 ) / (float)TILE_RES );
}

// Calculate the number of tiles in the vertical direction
uint GetNumTilesY()
{
    return ( uint )( ( BackbufferDimension.y + TILE_RES - 1 ) / (float)TILE_RES );
}

uint GetMaxNumLightsPerTile()
{
    uint clampedHeight = min( BackbufferDimension.y, 720 );
    return ( MAX_NUM_LIGHTS_PER_TILE - ( ADJUSTMENT_MULTIPLIER * ( clampedHeight / 120 ) ) );
}

static const int NUM_THREADS_X = TILE_RES;
static const int NUM_THREADS_Y = TILE_RES;
static const int NUM_THREADS_PER_TILE = ( NUM_THREADS_X * NUM_THREADS_Y );

// Min and Max Depth for a single tile
groupshared uint tileDepthMax;
groupshared uint tileDepthMin;

// Per tile light list
groupshared uint lightIndexCount;
groupshared uint lightIndex[MAX_NUM_LIGHTS_PER_TILE];

RWBuffer<uint> lightIndexBuffer : register( u0 );

#if PH_USE_MSAA
    Texture2DMS<float> depthTexture : register( t1 );
#else
    Texture2D<float> depthTexture : register( t1 );
#endif

float3 CreatePlaneEquation( float3 b, float3 c )
{
    // normalize(cross( b-a, c-a )), except we know "a" is the origin
    // also, typically there would be a fourth term of the plane equation, 
    // -(n dot a), except we know "a" is the origin
    return normalize( cross( b, c ) );
}

// point-plane distance, simplified for the case where 
// the plane passes through the origin
float GetSignedDistanceFromPlane( float3 p, float3 eqn )
{
    // dot(eqn.xyz,p) + eqn.w, , except we know eqn.w is zero 
    // (see CreatePlaneEquation above)
    return dot( eqn, p );
}

bool TestFrustumSides( float3 c, float r, float3 plane0, float3 plane1, float3 plane2, float3 plane3 )
{
    bool intersectingOrInside0 = GetSignedDistanceFromPlane( c, plane0 ) < r;
    bool intersectingOrInside1 = GetSignedDistanceFromPlane( c, plane1 ) < r;
    bool intersectingOrInside2 = GetSignedDistanceFromPlane( c, plane2 ) < r;
    bool intersectingOrInside3 = GetSignedDistanceFromPlane( c, plane3 ) < r;

    return ( intersectingOrInside0 && intersectingOrInside1 &&
             intersectingOrInside2 && intersectingOrInside3 );
}

// convert a point from post-projection space into view space
float4 ConvertProjToView( float4 p )
{
    p = mul( p, InverseProjectionMatrix );
    p /= p.w;
    return p;
}

// convert a depth value from post-projection space into view space
float ConvertProjDepthToView( float z )
{
    z = 1.0f / ( z * InverseProjectionMatrix._34 + InverseProjectionMatrix._44 );
    return z;
}

#if PH_USE_MSAA
void CalculateMinMaxDepthInLds( uint3 globalThreadIdx, uint depthBufferNumSamples )
{
    float minZForThisPixel = 3.402823466e+38F;
    float maxZForThisPixel = 0.f;

    float depth0 = depthTexture.Load( uint2( globalThreadIdx.x, globalThreadIdx.y ), 0 ).x;
    float viewPosZ0 = ConvertProjDepthToView( depth0 );
    if ( depth0 != 0.f ) {
        maxZForThisPixel = max( maxZForThisPixel, viewPosZ0 );
        minZForThisPixel = min( minZForThisPixel, viewPosZ0 );
    }

    for ( uint sampleIdx = 1; sampleIdx < depthBufferNumSamples; sampleIdx++ ) {
        float depth = depthTexture.Load( uint2( globalThreadIdx.x, globalThreadIdx.y ), sampleIdx ).x;
        float viewPosZ = ConvertProjDepthToView( depth );
        if ( depth != 0.f ) {
            maxZForThisPixel = max( maxZForThisPixel, viewPosZ );
            minZForThisPixel = min( minZForThisPixel, viewPosZ );
        }
    }

    uint zMaxForThisPixel = asuint( maxZForThisPixel );
    uint zMinForThisPixel = asuint( minZForThisPixel );
    InterlockedMax( tileDepthMax, zMaxForThisPixel );
    InterlockedMin( tileDepthMin, zMinForThisPixel );
}
#else
void CalculateMinMaxDepthInLds( uint3 globalThreadIdx )
{
    float depth = depthTexture.Load( uint3( globalThreadIdx.x, globalThreadIdx.y, 0 ) ).x;
    float viewPosZ = ConvertProjDepthToView( depth );
    uint z = asuint( viewPosZ );
    if ( depth != 0.f ) {
        InterlockedMax( tileDepthMax, z );
        InterlockedMin( tileDepthMin, z );
    }
}
#endif

[numthreads( NUM_THREADS_X, NUM_THREADS_Y, 1 )]
void EntryPointCS( uint3 globalIdx : SV_DispatchThreadID, uint3 localIdx : SV_GroupThreadID, uint3 groupIdx : SV_GroupID )
{
    uint localIdxFlattened = localIdx.x + localIdx.y * NUM_THREADS_X;

    // Initialize shared variables on the first thread
    if ( localIdxFlattened == 0 ) {
        // FLT_MAX as a uint
        tileDepthMin = 0x7f7fffff;
        tileDepthMax = 0;
        lightIndexCount = 0;
    }

    float3 frustumEqn0, frustumEqn1, frustumEqn2, frustumEqn3;
    {   
        // Construct frustum for this tile
        uint pxm = TILE_RES * groupIdx.x;
        uint pym = TILE_RES * groupIdx.y;
        uint pxp = TILE_RES * ( groupIdx.x + 1 );
        uint pyp = TILE_RES * ( groupIdx.y + 1 );

        uint uWindowWidthEvenlyDivisibleByTileRes = TILE_RES * GetNumTilesX();
        uint uWindowHeightEvenlyDivisibleByTileRes = TILE_RES * GetNumTilesY();

        // Four corners of the tile, clockwise from top-left
        float3 frustum0 = ConvertProjToView( float4( ( float )pxm / (float)uWindowWidthEvenlyDivisibleByTileRes*2.f - 1.f, ( float )( uWindowHeightEvenlyDivisibleByTileRes - pym ) / (float)uWindowHeightEvenlyDivisibleByTileRes*2.f - 1.f, 1.f, 1.f ) ).xyz;
        float3 frustum1 = ConvertProjToView( float4( ( float )pxp / (float)uWindowWidthEvenlyDivisibleByTileRes*2.f - 1.f, ( float )( uWindowHeightEvenlyDivisibleByTileRes - pym ) / (float)uWindowHeightEvenlyDivisibleByTileRes*2.f - 1.f, 1.f, 1.f ) ).xyz;
        float3 frustum2 = ConvertProjToView( float4( ( float )pxp / (float)uWindowWidthEvenlyDivisibleByTileRes*2.f - 1.f, ( float )( uWindowHeightEvenlyDivisibleByTileRes - pyp ) / (float)uWindowHeightEvenlyDivisibleByTileRes*2.f - 1.f, 1.f, 1.f ) ).xyz;
        float3 frustum3 = ConvertProjToView( float4( ( float )pxm / (float)uWindowWidthEvenlyDivisibleByTileRes*2.f - 1.f, ( float )( uWindowHeightEvenlyDivisibleByTileRes - pyp ) / (float)uWindowHeightEvenlyDivisibleByTileRes*2.f - 1.f, 1.f, 1.f ) ).xyz;

        // Create plane equations for the four sides of the frustum, 
        // with the positive half-space outside the frustum (and remember, 
        // view space is left handed, so use the left-hand rule to determine 
        // cross product direction)
        frustumEqn0 = CreatePlaneEquation( frustum0, frustum1 );
        frustumEqn1 = CreatePlaneEquation( frustum1, frustum2 );
        frustumEqn2 = CreatePlaneEquation( frustum2, frustum3 );
        frustumEqn3 = CreatePlaneEquation( frustum3, frustum0 );
    }

    GroupMemoryBarrierWithGroupSync();

    float minZ = 3.402823466e+38F;
    float maxZ = 0.f;
#if PH_USE_MSAA
    uint depthBufferWidth, depthBufferHeight, depthBufferNumSamples;
    depthTexture.GetDimensions( depthBufferWidth, depthBufferHeight, depthBufferNumSamples );
    CalculateMinMaxDepthInLds( globalIdx, depthBufferNumSamples );
#else
    CalculateMinMaxDepthInLds( globalIdx );
#endif

    GroupMemoryBarrierWithGroupSync();

    maxZ = asfloat( tileDepthMax );
    minZ = asfloat( tileDepthMin );

    // NOTE Area light can't be culled efficiently (because of their specular shape), so we don't need to add those to the light index buffer

    // Point light culling
    for ( uint i = localIdxFlattened; i < PointLightCount; i += NUM_THREADS_PER_TILE ) {
        float4 center = PointLights[i].PositionAndRadius;

        float sphereRadius = center.w;
        center.xyz = mul( float4( center.xyz, 1 ), ViewMatrix ).xyz;

        // Test Sphere-Frustum intersection
        if ( TestFrustumSides( center.xyz, sphereRadius, frustumEqn0, frustumEqn1, frustumEqn2, frustumEqn3 ) ) {
            if ( -center.z + minZ < sphereRadius && center.z - maxZ < sphereRadius ) {
                // Add the light index to the list
                uint dstIdx = 0;
                InterlockedAdd( lightIndexCount, 1, dstIdx );
                lightIndex[dstIdx] = i;
            }
        }
    }

    GroupMemoryBarrierWithGroupSync();

    // Spot Lights culling
    uint uNumPointLightsInThisTile = lightIndexCount;
    for ( uint j = localIdxFlattened; j < SpotLightCount; j += NUM_THREADS_PER_TILE ) {
        float4 center = SpotLights[j].PositionAndRadius;

        float discRadius = SpotLights[j].Radius;
        center.xyz = mul( float4( center.xyz, 1 ), ViewMatrix ).xyz;

        if ( TestFrustumSides( center.xyz, discRadius, frustumEqn0, frustumEqn1, frustumEqn2, frustumEqn3 ) ) {
            if ( -center.z + minZ < discRadius && center.z - maxZ < discRadius ) {
                // Add the light index to the list
                uint dstIdx = 0;
                InterlockedAdd( lightIndexCount, 1, dstIdx );
                lightIndex[dstIdx] = j;
            }
        }
    }

    GroupMemoryBarrierWithGroupSync();

    // Local Probe culling
    uint uNumSpotLightsInThisTile = lightIndexCount;
    for ( uint k = localIdxFlattened; k < LocalEnvironmentProbeCount; k += NUM_THREADS_PER_TILE ) {
        float4 center = LocalEnvProbes[k].PositionAndRadiusWorldSpace;

        float sphereRadius = center.w;
        center.xyz = mul( float4( center.xyz, 1 ), ViewMatrix ).xyz;

        // Test Sphere-Frustum intersection
        if ( TestFrustumSides( center.xyz, sphereRadius, frustumEqn0, frustumEqn1, frustumEqn2, frustumEqn3 ) ) {
            if ( -center.z + minZ < sphereRadius && center.z - maxZ < sphereRadius ) {
                // Add the light index to the list
                uint dstIdx = 0;
                InterlockedAdd( lightIndexCount, 1, dstIdx );
                lightIndex[dstIdx] = k;
            }
        }
    }

    GroupMemoryBarrierWithGroupSync();

    uint uNumProbesInThisTile = lightIndexCount;

    uint maxNumLightPerTile = GetMaxNumLightsPerTile();
    uint tileIdxFlattened = groupIdx.x + groupIdx.y * GetNumTilesX();
    uint startOffset = maxNumLightPerTile * tileIdxFlattened;

    // Update the light index buffer
    for ( uint l = localIdxFlattened; l < uNumPointLightsInThisTile; l += NUM_THREADS_PER_TILE ) {
        lightIndexBuffer[startOffset + l] = lightIndex[l];
    }

    for ( uint m = ( localIdxFlattened + uNumPointLightsInThisTile ); m < uNumSpotLightsInThisTile; m += NUM_THREADS_PER_TILE ) {
        lightIndexBuffer[startOffset + m + 1] = lightIndex[m];
    }

    for ( uint n = ( localIdxFlattened + uNumSpotLightsInThisTile ); n < lightIndexCount; n += NUM_THREADS_PER_TILE ) {
        lightIndexBuffer[startOffset + n + 2] = lightIndex[n];
    }

    if ( localIdxFlattened == 0 ) {
        // Point Lights
        lightIndexBuffer[startOffset + uNumPointLightsInThisTile] = 0x7fffffff;

        // Spot Lights
        lightIndexBuffer[startOffset + uNumSpotLightsInThisTile + 1] = 0x7fffffff;

        // Env Probes
        lightIndexBuffer[startOffset + uNumProbesInThisTile + 2] = 0x7fffffff;
    }
}
