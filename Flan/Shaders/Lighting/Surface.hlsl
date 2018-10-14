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

cbuffer MatricesBuffer : register( b3 )
{
    float4x4	ModelMatrix;
};

struct VertexBufferData
{
	float3 Position         : POSITION0;
	float3 Normal           : NORMAL0;
	float2 TexCoordinates   : TEXCOORD0;
};

#if PH_HEIGHTFIELD
#include <Tessellation.hlsli>

struct VertexStageHeightfieldData
{
    float4 positionMS   : POSITION;
    float2 uvCoord      : TEXCOORD;
    float4 tileInfos    : POSITION1; // xy tile height bounds z skirt id w tesselationFactor
};

VertexStageHeightfieldData EntryPointHeightfieldVS( VertexBufferData VertexBuffer )
{
    VertexStageHeightfieldData output = (VertexStageHeightfieldData)0;

	output.positionMS = float4( VertexBuffer.Position, 0.0f );
    output.uvCoord = VertexBuffer.TexCoordinates;
    output.tileInfos = float4( VertexBuffer.Normal, CalcTessFactor( VertexBuffer.Position ) );
    
    return output;
}
#endif

struct VertexStageData
{
    float4 position		: SV_POSITION;
    float4 positionWS   : POSITION0;
    float4 previousPosition : POSITION1;
    float3 normal       : NORMAL0;
    float depth         : DEPTH;
    float2 uvCoord      : TEXCOORD0;
};

VertexStageData EntryPointVS( VertexBufferData VertexBuffer )
{
    VertexStageData output = (VertexStageData)0;

    float2 uvCoordinates =  VertexBuffer.TexCoordinates;
    
#if PH_SCALE_UV_BY_MODEL_SCALE
    float scaleX = length( float3( ModelMatrix._11, ModelMatrix._12, ModelMatrix._13 ) );
    float scaleY = length( float3( ModelMatrix._21, ModelMatrix._22, ModelMatrix._23 ) );

    uvCoordinates *= float2( scaleX, scaleY );
#endif

    output.uvCoord          = uvCoordinates;
    output.positionWS       = mul( ModelMatrix, float4( VertexBuffer.Position, 1.0f ) );
    output.position         = mul( float4( output.positionWS.xyz, 1.0f ), ViewProjectionMatrix );
    output.previousPosition = mul( float4( output.positionWS.xyz, 1.0f ), g_PreviousViewProjectionMatrix ); //.xywz;
   
    float4 PositionVS = mul( float4( output.positionWS.xyz, 1.0f ), ViewMatrix );
    output.depth = ( PositionVS.z / PositionVS.w );
    output.normal = normalize( mul( ModelMatrix, float4( VertexBuffer.Normal, 0.0f ) ) ).xyz;
    
	return output;
}
