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
	float3 Tangent          : TANGENT0;
	float3 Binormal		    : BINORMAL0;
};

struct VertexStageData
{
    float4 position		: SV_POSITION;
    float4 positionWS   : POSITION0;
    float4 previousPosition : POSITION1;
    float2 uvCoord      : TEXCOORD0;
    float depth         : DEPTH;
    float3 normal       : NORMAL0;
    float3 tangent      : TANGENT0;
    float3 binormal		: BINORMAL0;
};

VertexStageData EntryPointVS( VertexBufferData VertexBuffer )
{
    VertexStageData output = (VertexStageData)0;

    output.positionWS       = mul( ModelMatrix, float4( VertexBuffer.Position, 1.0f ) );
    output.position         = mul( float4( output.positionWS.xyz, 1.0f ), ViewProjectionMatrix );
    output.previousPosition = mul( float4( output.positionWS.xyz, 1.0f ), g_PreviousViewProjectionMatrix ).xywz;
   
    output.uvCoord = VertexBuffer.TexCoordinates;

#if PH_SCALE_UV_BY_MODEL_SCALE
    // TODO Make UV Scaling as an option?   
    float scaleX = length( float3( ModelMatrix._11, ModelMatrix._12, ModelMatrix._13 ) );
    float scaleY = length( float3( ModelMatrix._21, ModelMatrix._22, ModelMatrix._23 ) );

    output.uvCoord *= float2( scaleX, scaleY );
#endif

    float4 PositionVS = mul( float4( output.positionWS.xyz, 1.0f ), ViewMatrix );
    output.depth = ( PositionVS.z / PositionVS.w );

#if PH_USE_NORMAL_MAPPING
    output.normal = normalize( mul( ModelMatrix, float4( VertexBuffer.Normal, 0.0f ) ) ).xyz;
    output.tangent = normalize( mul( ModelMatrix, float4( VertexBuffer.Tangent, 0.0f ) ) ).xyz;
    output.binormal = normalize( mul( ModelMatrix, float4( VertexBuffer.Binormal, 0.0f ) ) ).xyz;
#endif

	return output;
}
