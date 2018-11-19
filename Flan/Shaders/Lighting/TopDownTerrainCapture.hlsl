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
#include <Shared.hlsli>
#include <Colormetry.hlsli>

SamplerState LinearSampler { Filter = MIN_MAG_LINEAR_MIP_POINT; AddressU = Clamp; AddressV = Clamp; };
Texture2D g_TexMainRenderTarget : register( t0 );

cbuffer TerrainInfos : register( b0 )
{
    float3  g_PositionWorldSpace;
    float   g_HeightmapSize;
    float3  g_CameraPositionWorldSpace;
};

float4 EntryPointPS( psDataScreenQuad_t p ) : SV_TARGET
{
    // Top-left origin
    float3 terrainOriginWorldSpace = g_PositionWorldSpace - ( g_HeightmapSize / 2.0f );
    float3 originToCamera = g_CameraPositionWorldSpace - terrainOriginWorldSpace;
    
    float2 uvCoordinates = p.uv + ( originToCamera / g_HeightmapSize );
    
    return g_TexMainRenderTarget.Sample( LinearSampler, uvCoordinates );
}
