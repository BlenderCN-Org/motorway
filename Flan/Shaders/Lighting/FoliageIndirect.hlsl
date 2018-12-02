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

#include <CameraData.hlsli>
#include <RenderableEntities.hlsli>
#include <Lighting.hlsli>

#include "GrassShared.hlsli"

struct VertexStageData
{
    float4 position		    : SV_POSITION;
    float4 albedo           : COLOR;
    float2 texCoordinates   : TEXCOORD0;
    float depth             : DEPTH;
};

StructuredBuffer<Instance> g_GrassInstanceBuffer : register( t0 );

// x cross quads (6x6 vertex)
static const float3 VERTICES[12] = {
	float3(-1.0f, 0.0f,  0.0f),
	float3( 1.0f, 0.0f,  0.0f),
	float3(-1.0f, 1.0f,  0.0f),
	float3(-1.0f, 1.0f,  0.0f),
	float3( 1.0f, 0.0f,  0.0f),
	float3( 1.0f, 1.0f,  0.0f),
    
	float3( 0.0f, 0.0f, -1.0f),
	float3( 0.0f, 0.0f,  1.0f),
	float3( 0.0f, 1.0f, -1.0f),
	float3( 0.0f, 1.0f, -1.0f),
	float3( 0.0f, 0.0f,  1.0f),
	float3( 0.0f, 1.0f,  1.0f)
};

VertexStageData EntryPointVS( uint VertexID : SV_VertexID )
{
    VertexStageData output = (VertexStageData)0;

	uint foliageFinID = floor( VertexID / 12 );
	uint finVertexID = VertexID - ( foliageFinID * 12 );

	Instance instance = g_GrassInstanceBuffer[foliageFinID];

	// sin/cos for rotation matrix
	float s = instance.rotation.x;
	float c = instance.rotation.y;
	
    float scaleX = instance.scale.x;
    float scaleY = instance.scale.y;
    
    float skew = f16tof32( instance.vertexOffsetAndSkew & 0x0000FFFF );
    float offset = f16tof32( instance.vertexOffsetAndSkew >> 16 );
    
	// Build world matrix
	float3 worldPosition = instance.position;
	float4x4 ModelMatrix = float4x4(
        float4( c * instance.scale.x, 0, s, worldPosition.x ),
        float4( 0, 1, 0, worldPosition.y ),
        float4( -s, 0, c * instance.scale.y, worldPosition.z ),
        float4( 0, 0, 0, 1 )
    );
    
    float3 viewSpacePosition = VERTICES[finVertexID];
	    
    bool isTopVertex = ( finVertexID == 2 || finVertexID == 3 || finVertexID == 5 ) 
                    || ( finVertexID == 8 || finVertexID == 9 || finVertexID == 11 );
    
    // Apply skew
    if ( isTopVertex ) {
        ModelMatrix[1][0] += skew;
        ModelMatrix[0][1] += offset;
    }
    
    float4 positionWS       = mul( ModelMatrix, float4( viewSpacePosition, 1.0f ) );
    output.position         = mul( float4( positionWS.xyz, 1.0f ), ViewProjectionMatrix );
	
	output.texCoordinates =  ( finVertexID > 5 ) ? viewSpacePosition.zy : viewSpacePosition.xy * float2( 0.5f, 1.0f ) + float2( 0.5f, 0.0f );
    output.texCoordinates.y = ( 1.0f - output.texCoordinates.y );
    
	output.albedo 			= float4( instance.albedo, instance.specular );
    float4 PositionVS = mul( float4( positionWS.xyz, 1.0f ), ViewMatrix );
    output.depth = ( PositionVS.z / PositionVS.w );
	
	return output;
}

sampler     g_BilinearSampler   : register( s0 );
Texture2D   g_TexAlbedo         : register( t1 );
Texture2DMS<float>   g_DepthBuffer       : register( t2 );

float4 EntryPointPS( VertexStageData VertexStage ) : SV_TARGET0
{
    float2 screenCoords = VertexStage.position.xy / VertexStage.position.w * 0.5f + 0.5f;
    
    float depthSample = g_DepthBuffer.Load( screenCoords, 0 ).r;
    float4 colorMask = g_TexAlbedo.Sample( g_BilinearSampler, VertexStage.texCoordinates ).rgba;  
    
    // Sample alpha mask and do a cheap sharpening to prevent from alpha to coverage artifacts
    float alphaMask = colorMask.a;
    alphaMask = ( alphaMask - 0.5f ) / max( fwidth( alphaMask ), 0.0001f ) + 0.5f;
    
    float reversedDepth = saturate( VertexStage.depth / 512.0f );
    
    float occlusion = ( depthSample.r <= reversedDepth ) ? 1.0f : 0.0f;
    
    // NOTE Lighting calculation is done per instance in the compute shader
    return float4( colorMask.rgb * VertexStage.albedo.rgb * occlusion, alphaMask );
}
