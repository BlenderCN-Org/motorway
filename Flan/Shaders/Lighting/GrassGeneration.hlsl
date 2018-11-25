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

// Scatter of foliage around a fin centre. 
static const float2 scatterKernel[8] =
{
    float2(0 , 0),
    float2(0.8418381f , -0.8170416f),
    float2(-0.9523101f , 0.5290064f),
    float2(-0.1188585f , -0.1276977f),
    float2(-0.207716f, 0.09361804f),
    float2(0.1588526f , 0.440437f),
    float2(-0.6105742f , 0.07276237f),
    float2(-0.09883061f , 0.4942337f)
};

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

[numthreads( 32, 32, 1 )]
void EntryPointCS( uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID )
{
    // TODO Frustum culling (using main viewport frustum)
    // See GRID Autosport paper for more details

    // TODO Could be precomputed
    const float texelScale = ( g_GrassMapSize / g_HeightfieldSize );
    
    //float2 grassToCaptureOffset = g_HeightfieldOriginWorldSpace - g_CameraPositionWorldSpace.xz; 
    //grassToCaptureOffset *= texelScale;
    
	// Cull 32 texel per thread (assuming 2k x 2k grass map)
	float heightSample = g_TexTopDownCapture.Load( uint3( DispatchThreadID.xy, 0 ) ).r;
    
    uint3 grassMapSampleCoords = uint3( DispatchThreadID.xy * texelScale, 0 );
	float4 grassMapSample = g_TexGrassMap.Load( grassMapSampleCoords.xyz );
    
    float4 randomNoise = g_TexRandomness.Load( uint3( GroupThreadID.xy * 2, 0 ) ); // Assuming 64x64 RGBA noise texture
	
    // TODO Per terrain instance height value (right now we assume the height scale is ALWAYS 64.0f, which might not be true in the future)
    [branch]
    if ( grassMapSample.a < ( heightSample / 64.0f ) ) {
        return;
    }
    
    [branch]
    if ( DispatchThreadID.x >= (uint)g_HeightfieldSize 
      && DispatchThreadID.y >= (uint)g_HeightfieldSize ) {
        return;
    }
    
    const float texelScale2 = ( g_HeightfieldSize / g_GrassMapSize );
    float3 generatedWorldPosition = float3(  ( DispatchThreadID.x / texelScale2 ), heightSample, ( DispatchThreadID.y / texelScale2 ) );
    
    // Frustum cull once we know the world position
    [branch]
    if ( CullSphere( generatedWorldPosition, ( 1.0f / texelScale2 ) ) <= 0.0f ) {
        return;
    }
    
    float distanceToCamera = distance( g_CameraPositionWorldSpace, generatedWorldPosition ) / 512.0f;
    
    // Use LoD to draw as little as possible
    [branch]
    if ( distanceToCamera > 0.05f ) {        
        // Use noise for the farest LoD
        int cullProba = ( distanceToCamera > 0.9f ) 
                        ? (int)( distanceToCamera * ( 100.0f * randomNoise.a ) ) 
                        : (int)( distanceToCamera * 100.0f );
        
        if ( generatedWorldPosition.x % cullProba == 0 
          && generatedWorldPosition.z % cullProba == 0 ) {
            return;
        } 
    }
    
  
    // Create a scatter around the single pixel sampled from the drape.
    [unroll]
    for ( int scatterKernelIndex = 0; scatterKernelIndex < 8; ++scatterKernelIndex ) {
        // Build and append grass instances to the buffer
        Instance instance = (Instance)0;
        instance.position = float3(  generatedWorldPosition.x + ( scatterKernel[scatterKernelIndex].x * texelScale2 ), 
                                    generatedWorldPosition.y, 
                                    generatedWorldPosition.z + ( scatterKernel[scatterKernelIndex].y * texelScale2 ) );
        instance.specular = lerp( 0.25f, 0.50f, randomNoise.r );
        instance.albedo = grassMapSample.rgb * max( 0.1f, randomNoise.ggg );
        instance.vertexOffsetAndSkew = 0;
        instance.rotation = float2( sin( 145.0f ), cos( 0.0f ) ) * randomNoise.bb;
        instance.scale = max( float2( 0.25f, 0.25f ), float2( 2, 2 ) * randomNoise.aa );

        g_GrassInstanceBuffer.Append( instance );
    }
}
