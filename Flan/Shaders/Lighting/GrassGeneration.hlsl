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
#include "GrassShared.hlsli"

#include <RenderableEntities.hlsli>

#define PA_FAKE_IRRADIANCE 1
#define PA_DONT_RECEIVE_SHADOWS 1
#include <Lighting.hlsli>

cbuffer GrassGenerationBuffer : register( b0 )
{
	float4 	 g_CameraFrustumPlanes[6]; // NOTE This is main camera frustum (NOT top down camera!)
	
    float3	 g_CameraPositionWorldSpace;
    float    g_HeightfieldSize; // NOTE Terrrain geometry should match heightmap size
    
    float2   g_HeightfieldOriginWorldSpace; // Top-left corner position
    float    g_GrassMapSize;
}

Texture2D<float4> 	g_TexGrassMap  		: register( t0 ); // RGB color A height
Texture2D<float4> 	g_TexTopDownCapture : register( t1 );
Texture2D<float4>	g_TexRandomness		: register( t2 );

AppendStructuredBuffer<Instance> g_GrassInstanceBuffer : register( u0 );

inline float DistanceToPlane( const float4 plane, const float3 vPoint )
{
    return dot( plane, float4( vPoint, 1.0f ) );
}

float CullSphere( const float3 sphereCenter, const float fRadius )
{
    float dist01 = min( DistanceToPlane( g_CameraFrustumPlanes[0], sphereCenter ), DistanceToPlane( g_CameraFrustumPlanes[1], sphereCenter ) );
    float dist23 = min( DistanceToPlane( g_CameraFrustumPlanes[2], sphereCenter ), DistanceToPlane( g_CameraFrustumPlanes[3], sphereCenter ) );
    float dist45 = min( DistanceToPlane( g_CameraFrustumPlanes[4], sphereCenter ), DistanceToPlane( g_CameraFrustumPlanes[5], sphereCenter ) );

    return min( min( dist01, dist23 ), dist45 ) + fRadius;
}

float3 BlendNormals_UDN( in float3 baseNormal, in float3 topNormal )
{
    static const float3 c = float3( 2, 1, 0 );  
    float3 blendedNormals = topNormal * c.yyz + baseNormal.xyz;
    return normalize( blendedNormals );
}

[numthreads( 32, 32, 1 )]
void EntryPointCS( uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID, uint3 GroupID : SV_GroupID )
{
    // TODO Frustum culling (using main viewport frustum)
    // See GRID Autosport paper for more details

    // TODO Could be precomputed
    //const float texelScale = ( g_GrassMapSize / g_HeightfieldSize );
    
    //float2 grassToCaptureOffset = g_HeightfieldOriginWorldSpace - g_CameraPositionWorldSpace.xz; 
    //grassToCaptureOffset *= texelScale;
    
    const uint2 texelIndex = uint2( 32, 32 ) * GroupID.xy + GroupThreadID.xy;
    
	// Cull 32 texel per thread (assuming 2k x 2k grass map)
	float heightSample = g_TexTopDownCapture.Load( uint3( texelIndex.x, texelIndex.y, 0 ) ).r;
    
    uint3 grassMapSampleCoords = uint3( texelIndex.x, texelIndex.y, 0 );
	float4 grassMapSample = g_TexGrassMap.Load( grassMapSampleCoords.xyz );
    float grassHeight = grassMapSample.a;
    
    float4 randomNoise = g_TexRandomness.Load( uint3( GroupThreadID.xy * 2, 0 ) ); // Assuming 64x64 RGBA noise texture
	
    // TODO Per terrain instance height value (right now we assume the height scale is ALWAYS 64.0f, which might not be true in the future)
    [branch]
    if ( grassHeight < ( heightSample / 64.0f ) ) {
        return;
    }
    
    [branch]
    if ( texelIndex.x > (uint)g_HeightfieldSize 
      || texelIndex.y > (uint)g_HeightfieldSize ) {
        return;
    }
    
    const float texelScale2 = ( g_HeightfieldSize / g_GrassMapSize );
    float3 generatedWorldPosition = float3(  ( texelIndex.x / texelScale2 ), heightSample, ( texelIndex.y / texelScale2 ) );
    
    // Frustum cull once we know the world position
    [branch]
    if ( CullSphere( generatedWorldPosition, texelScale2 ) <= 0.0f ) {
        return;
    }
    
    float distanceToCamera = ( distance( g_CameraPositionWorldSpace, generatedWorldPosition ) / 512.0f );
    
    // Feather distance to break up transitions betweem LODs
    distanceToCamera -= ( distanceToCamera * randomNoise.r ) * 0.5f;
    
    // Use LoD to draw as little as possible
    int lodGrassSkip = 1;
    float lodScaleFactor = 1.0f;
    float3 lodColor = float3( 1, 0, 0 );
    
    [branch]
    if ( distanceToCamera > 1.0f ) {
        return;
    } else if ( distanceToCamera > 0.50f ) {
        lodScaleFactor = 8.0f;
        lodGrassSkip = 32;
        lodColor = float3( 1, 0, 1 );

        float grassHeightFadeFactor = ( 1.0f - distanceToCamera ) / 0.750f;
        grassHeight *= grassHeightFadeFactor;

        [branch]
        if ( grassHeight < ( heightSample / 64.0f ) ) {
            return;
        }
    } else if ( distanceToCamera > 0.250f ) {
        lodScaleFactor = 6.0f;
        lodGrassSkip = 16;
        lodColor = float3( 1, 1, 0 );
    } else if ( distanceToCamera > 0.1500f ) {
        lodScaleFactor = 4.00f;
        lodGrassSkip = 8;
        lodColor = float3( 0, 0, 1 );
    } else if ( distanceToCamera > 0.0500f ) {
        lodGrassSkip = 4;
        lodScaleFactor = 2.00f;
        lodColor = float3( 0, 1, 0 );
    } 
    
    [branch]
    if ( generatedWorldPosition.x % lodGrassSkip != 0 
      && generatedWorldPosition.z % lodGrassSkip != 0 ) {
        return;
    } 
    
    // Build and append grass instances to the buffer
    Instance instance = (Instance)0;
    instance.position = generatedWorldPosition;

    instance.position.xz += float2( lodScaleFactor, lodScaleFactor ) * 0.5f * randomNoise.ra;
    
    instance.specular = lerp( 0.05f, 0.45f, randomNoise.r );
    instance.albedo = grassMapSample.rgb * clamp( 0.5f + randomNoise.ggg, 0.5f, 1.0f );
    
    // Calculate lighting per instance (instead of per pixel/vertex)
    float3 N = float3( 0, 1, 0 );
	float3 L = float3( 0, 0, 0 );
    
    float3 V = normalize( g_CameraPositionWorldSpace.xyz - instance.position );
    float3 R = reflect( -V, N );
    
    const LightSurfaceInfos surface = {
        V,
        1.0f,

        N,
        0.0f,

        // Albedo
        instance.albedo.rgb,
        0.0f,

        // F0
        float3( 0.0f, 0.0f, 0.0f ),

        // F90
        saturate( 50.0 * dot( 0.01f, 0.33 ) ),

        instance.position,

        // LinearRoughness
        1.0f,

        // Reflection Vector
        R,

        // Disney BRDF LinearRoughness
        ( 0.5f + 1.0f / 2.0f ),

        // Clear Coat Normal
        N,

        // N dot V
        // FIX Use clamp instead of saturate (clamp dot product between epsilon and 1)
        clamp( dot( N, V ), 1e-5f, 1.0f ),

        0.0f,
        0.0f,

        0.0f,
        0u // Explicit Padding
    };
 
    float3 lightContribution = float3( 0, 0, 0 );
    
    // Directional Lights
    [loop]
    for ( uint i = 0; i < DirectionalLightCount; ++i ) {
        float3 directionalLightIlluminance = GetDirectionalLightIlluminance( DirectionalLights[i], surface, 0.0f, L );
		
		float3 RL = reflect( -L, N );
    
		const float NoL = saturate( dot( surface.N, L ) );
		const float VoR = saturate( dot( V, RL ) );			
		
        lightContribution += Diffuse_LambertWrapped( NoL, surface.LinearRoughness ) * surface.Albedo * directionalLightIlluminance;
    }
    
    instance.albedo = lightContribution;
    
#if PA_GRASS_LOD_DEBUG_COLOR
    instance.albedo = lodColor;
#endif

    float skew = lerp( 0.0f, 0.4f, randomNoise.g );
    float offset = lerp( 0.0f, 0.4f, randomNoise.r );

    // Store offset and skew in a 32bits integer (VGPR optimization)
    instance.vertexOffsetAndSkew = ( f32tof16( offset ) << 16 | f32tof16( skew ) );
    
    instance.rotation = float2( sin( 180.0f ), cos( 0.0f ) ) * randomNoise.bb;
    instance.scale = max( float2( 0.5f, 0.5f ), float2( 4, 4 ) * ( grassMapSample.a * randomNoise.aa ) ) * lodScaleFactor;

    g_GrassInstanceBuffer.Append( instance );
}
