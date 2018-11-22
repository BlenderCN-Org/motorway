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

struct Instance
{
	float3	position;
	float 	specular;
	float3	albedo;
	uint	vertexOffsetAndSkew;
	float2	rotation;
	float2	scale;
};

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

[numthreads( 32, 32, 1 )]
void EntryPointCS( uint3 DispatchThreadID : SV_DISPATCHTHREADID )
{
    // TODO Frustum culling (using main viewport frustum)
    // See GRID Autosport paper for more details
    float2 grassToCaptureOffset = g_CameraPositionWorldSpace.xz - g_HeightfieldOriginWorldSpace;
	
	// Cull 32 texel per thread (assuming 2k x 2k grass map)
	float heightSample = g_TexTopDownCapture.Load( uint3( DispatchThreadID.xy, 0 ) ).r;	
	float4 grassMapSample = g_TexGrassMap.Load( uint3( grassToCaptureOffset + DispatchThreadID.xy, 0 ) );
    float4 randomNoise = g_TexRandomness.Load( uint3( DispatchThreadID.xy * 2, 0 ) ); // Assuming 64x64 RGBA noise texture
	
    // TODO Could be precomputed
    const float texelScale = ( g_GrassMapSize / g_HeightfieldSize );
    float3 generatedWorldPosition = float3( g_HeightfieldOriginWorldSpace.x + ( DispatchThreadID.x * texelScale ), heightSample, g_HeightfieldOriginWorldSpace.y + ( DispatchThreadID.y * texelScale ) );
    
    // Create a scatter around the single pixel sampled from the drape.
    [unroll]
    for ( int scatterKernelIndex = 0; scatterKernelIndex < 8; ++scatterKernelIndex ) {
        // Build and append grass instances to the buffer
        Instance instance = (Instance)0;
        instance.position = float3( generatedWorldPosition.x + ( scatterKernel[scatterKernelIndex].x * texelScale ), 
                                    generatedWorldPosition.y, 
                                    generatedWorldPosition.z + ( scatterKernel[scatterKernelIndex].y * texelScale ) );
        instance.specular = lerp( 0.25f, 0.50f, randomNoise.r );
        instance.albedo = randomNoise.ggg;
        instance.vertexOffsetAndSkew = 0;
        instance.rotation = float2( 90.0f, 0.0f ) * randomNoise.bb;
        instance.scale = float2( 1, 1 ) * randomNoise.aa;

        g_GrassInstanceBuffer.Append( instance );
    }
}
