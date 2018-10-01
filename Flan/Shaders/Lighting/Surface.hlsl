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

#if PH_HEIGHTFIELD
Texture2D g_TexHeightmap    : register( t0 );

struct MaterialEditionInput
{
    // Input can have 4 states:
    //      0: none
    //      1: constant 1d value
    //      2: constant 3d vector    
    //      3: texture input
    float3  Input3D;
    float   Input1D;
    
    int     Type;
    uint    SamplingMode;
    float2  EXPLICIT_PADDING; // Holds Texture pointer on CPU side
};
#include <MaterialsShared.h>
cbuffer MaterialEdition : register( b8 )
{
    // Flags
    uint                    g_WriteVelocity; // Range: 0..1 (should be 0 for transparent surfaces)
    uint                    g_EnableAlphaTest;
    uint                    g_EnableAlphaBlend;
    uint                    g_IsDoubleFace;
    
    uint                    g_CastShadow;
    uint                    g_ReceiveShadow;
    uint                    g_EnableAlphaToCoverage;
    uint                    g_LayerCount;
    
    MaterialLayer           g_Layers[MAX_LAYER_COUNT];
};
#endif

struct VertexBufferData
{
	float3 Position         : POSITION0;
	float3 Normal           : NORMAL0;
	float2 TexCoordinates   : TEXCOORD0;
};

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

    output.uvCoord = uvCoordinates;

#if PH_HEIGHTFIELD
    float2 heightCoords = float2( VertexBuffer.Position.x, VertexBuffer.Position.z );
    
	float3 positionModelSpace = VertexBuffer.Position;
    float height = g_TexHeightmap[abs( heightCoords )].r;

    output.positionWS       = mul( ModelMatrix, float4( positionModelSpace.x, height * g_Layers[0].HeightmapWorldHeight, positionModelSpace.z, 1.0f ) );
#else
    output.positionWS       = mul( ModelMatrix, float4( VertexBuffer.Position, 1.0f ) );
#endif
    
    output.position         = mul( float4( output.positionWS.xyz, 1.0f ), ViewProjectionMatrix );
    output.previousPosition = mul( float4( output.positionWS.xyz, 1.0f ), g_PreviousViewProjectionMatrix ).xywz;
   
    float4 PositionVS = mul( float4( output.positionWS.xyz, 1.0f ), ViewMatrix );
    output.depth = ( PositionVS.z / PositionVS.w );

#if PH_USE_NORMAL_MAPPING
    output.normal = normalize( mul( ModelMatrix, float4( VertexBuffer.Normal, 0.0f ) ) ).xyz;
#endif

	return output;
}
