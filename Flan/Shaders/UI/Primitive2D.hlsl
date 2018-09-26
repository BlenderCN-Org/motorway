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

sampler  g_BaseColorSampler : register( s0 );
sampler  g_AlphaMaskSampler : register( s7 );

Texture2D g_TexBaseColor : register( t0 );
Texture2D g_TexAlphaMask : register( t7 );

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
    uint    SamplingChannel; // (default = 0 = red)
    float2  EXPLICIT_PADDING;
};

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
    float   				g_AlphaCutoff;    
    
    MaterialEditionInput   	g_BaseColor;
    MaterialEditionInput   	g_Reflectance;   
    MaterialEditionInput   	g_Normal;
    MaterialEditionInput   	g_Roughness;
    
    MaterialEditionInput   	g_Metalness;
    MaterialEditionInput   	g_AmbientOcclusion;
    MaterialEditionInput   	g_Emissivity;
    MaterialEditionInput   	g_AlphaMask;
    
    float2					g_LayerScale;
    float   				g_Refraction;
    float   				g_RefractionIor;
};


float ReadInput1D( in float2 uvCoord, in MaterialEditionInput materialInput, in sampler texSampler, in Texture2D materialInputTex, const float defaultValue )
{
    float input = defaultValue;
        
    if ( materialInput.Type == 1 ) input = materialInput.Input1D;
    else if ( materialInput.Type == 2 ) input = materialInput.Input3D.r;
    else if ( materialInput.Type == 3 ) input = materialInputTex.Sample( texSampler, uvCoord * g_LayerScale ).r;

    return input;
}

float3 ReadInput3D( in float2 uvCoord, in MaterialEditionInput materialInput, in sampler texSampler, in Texture2D materialInputTex, const float3 defaultValue )
{
    float3 input = defaultValue;
        
    if ( materialInput.Type == 1 ) input = materialInput.Input1D.rrr;
    else if ( materialInput.Type == 2 ) input = materialInput.Input3D.rgb;
    else if ( materialInput.Type == 3 ) input = materialInputTex.Sample( texSampler, uvCoord * g_LayerScale ).rgb;

    return input;
}

struct VertexBufferData
{
    float3 position         : POSITION;
    float2 uvCoord   : TEXCOORD0;
};

cbuffer Parameters : register( b0 )
{
    float4x4    OrthoProjectionMatrix;
    float4x4    PrimitiveModelMatrix;
}

struct VertexStageData
{
    float4 position  : SV_POSITION;
    float2 uvCoord   : TEXCOORD0;
};

VertexStageData EntryPointVS( in VertexBufferData VertexBuffer )
{
    // Compute ScreenSpace position for the instance being rendered
    float4 PositionSS = mul( PrimitiveModelMatrix, float4( VertexBuffer.position, 1.0f ) );
    PositionSS = mul( float4( PositionSS.xyz, 1.0f ), OrthoProjectionMatrix );
    
    VertexStageData output;   
    output.position = float4( PositionSS.xyz, 1.0f );
    output.uvCoord = VertexBuffer.uvCoord;

    return output;
}
    
float4 EntryPointPS( VertexStageData VertexStage ) : SV_TARGET0
{
    float3 BaseColor = ReadInput3D( VertexStage.uvCoord, g_BaseColor, g_BaseColorSampler, g_TexBaseColor, float3( 0.42, 0.42, 0.42 ) ); 
    float AlphaMask = ReadInput1D( VertexStage.uvCoord, g_AlphaMask, g_AlphaMaskSampler, g_TexAlphaMask, 1.0f );
        
    BaseColor *= AlphaMask;
    return float4( BaseColor, AlphaMask );
}
