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
#include "GrassShared.hlsli"

struct VertexStageData
{
    float4 position		: SV_POSITION;
    float3 albedo       : COLOR;
    float2 texCoordinates : TEXCOORD0;
};

StructuredBuffer<Instance> g_GrassInstanceBuffer : register( t0 );

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

	uint foliageFinID = floor( VertexID / 6 );
	uint finVertexID = VertexID - ( foliageFinID * 6 );

	Instance instance = g_GrassInstanceBuffer[foliageFinID];

	// sin/cos for rotation matrix
	float s = instance.rotation.x;
	float c = instance.rotation.y;
	
    float scaleX = instance.scale.x;
    float scaleY = instance.scale.y;
    
	// Build world matrix
	float3 worldPosition = instance.position;
	float4x4 ModelMatrix = float4x4(
        float4( c * instance.scale.x, 0, s, worldPosition.x ),
        float4( 0, 1, 0, worldPosition.y ),
        float4( -s, 0, c * instance.scale.y, worldPosition.z ),
        float4( 0, 0, 0, 1 )
    );
    
    float3 viewSpacePosition = VERTICES[finVertexID];
	    
    float4 positionWS       = mul( ModelMatrix, float4( viewSpacePosition, 1.0f ) );
    output.position         = mul( float4( positionWS.xyz, 1.0f ), ViewProjectionMatrix );
	output.texCoordinates =  viewSpacePosition.xy * float2(0.5f, 1.0f) + float2(0.5f, 0.0f);
    output.texCoordinates.y = 1.0f - output.texCoordinates.y;
    
	output.albedo 			= instance.albedo;
	
	return output;
}

sampler  g_BilinearSampler : register( s0 );
Texture2D g_TexAlbedo : register( t1 );
Texture2D g_TexAlphaMap : register( t2 );

float4 EntryPointPS( VertexStageData VertexStage ) : SV_TARGET0
{
    float3 colorMask = g_TexAlbedo.Sample( g_BilinearSampler, VertexStage.texCoordinates ).rgb;  
    float alphaMask = g_TexAlphaMap.Sample( g_BilinearSampler, VertexStage.texCoordinates ).r;   
    
	return float4( colorMask * VertexStage.albedo, alphaMask );
}