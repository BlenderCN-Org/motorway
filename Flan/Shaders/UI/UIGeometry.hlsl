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

struct VertexBufferData
{
    float3 Position         : POSITION;
    float2 TexCoordinates   : TEXCOORD0;
    float4 Color            : COLOR0;
};

struct VertexStageData
{
    float4 Position         : SV_POSITION;
    float4 Color            : COLOR0;
    float2 TexCoordinates   : TEXCOORD0;
};

cbuffer PassData : register( b0 )
{
    uint2 BackbufferDimension;
}

float2 projectPoint( float2 vPoint )
{
    float2 vProjected;
    vProjected.x = vPoint.x * ( 1.0f / ( BackbufferDimension.x * 0.5f ) ) - 1.0f;
    vProjected.y = vPoint.y * ( 1.0f / ( BackbufferDimension.y * 0.5f ) ) + 1.0f;
    return vProjected;
}

VertexStageData EntryPointVS( in VertexBufferData VertexBuffer )
{
    VertexStageData output;
    
    output.Position = float4( projectPoint( VertexBuffer.Position.xy ), VertexBuffer.Position.z, 1 );
    output.Color = VertexBuffer.Color;
    output.TexCoordinates = VertexBuffer.TexCoordinates;

    return output;
}
