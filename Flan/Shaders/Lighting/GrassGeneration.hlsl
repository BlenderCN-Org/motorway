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
    
	// Cull 32 texel per thread (assuming 2k x 2k grass map)
	float heightSample = g_TexTopDownCapture.Load( uint3( DispatchThreadID.x, g_GrassMapSize - DispatchThreadID.y, 0)).r;	
	float4 grassMapSample = g_TexGrassMap.Load( uint3( DispatchThreadID.xy, 0 ) );
    
    // TODO Could be precomputed
    float texelScale = ( g_GrassMapSize / g_HeightfieldSize );
    
    float3 generatedWorldPosition = float3( g_HeightfieldOriginWorldSpace.x + ( DispatchThreadID.x * texelScale ), heightSample, g_HeightfieldOriginWorldSpace.y + ( DispatchThreadID.y * texelScale ) );
    
    // Create a scatter around the single pixel sampled from the drape.
    [unroll]
    for ( int scatterKernelIndex = 0; scatterKernelIndex < 8; ++scatterKernelIndex ) {
        // Build and append grass instance to the render buffer
        // TODO Sample noise and randomize stuff
        Instance instance = (Instance)0;
        instance.position = float3(generatedWorldPosition.x + (scatterKernel[scatterKernelIndex].x * texelScale), 
                                    generatedWorldPosition.y, 
                                    generatedWorldPosition.z + (scatterKernel[scatterKernelIndex].y * texelScale));;     
        instance.specular = 0.5f;
        instance.albedo = float3( 0, 1, 0 );
        instance.vertexOffsetAndSkew = 0;
        instance.rotation = float2( 0, 0 );
        instance.scale = float2( 0, 0 );

        g_GrassInstanceBuffer.Append( instance );
    }
}
