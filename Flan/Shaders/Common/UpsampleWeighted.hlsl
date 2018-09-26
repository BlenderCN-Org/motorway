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
#include <Samplers.hlsli>
#include <ScreenSpaceShared.hlsli>

// Next Generation Post Processing in Call of Duty AW [Jorge Jimenez, 2017]
// http://www.iryoku.com/downloads/Next-Generation-Post-Processing-in-Call-of-Duty-Advanced-Warfare-v18.pptx
// Progressive Upscaling (from mip N to mip N+1)

Texture2D g_TextureInput : register( t3 );
Texture2D g_TextureAccumulatorInput : register( t2 );

cbuffer BlurInfos : register( b0 )
{
    float2  g_TextureInputInverseDimensions;
    float   g_BlurRadius;
}

float4 EntryPointPS( ScreenSpaceVertexStageData VertexStage ) : SV_TARGET0
{
    float4 d = float4( g_TextureInputInverseDimensions, g_TextureInputInverseDimensions ) * float4( 1, 1, -1, 0 ) * 1.0f;

    // 3x3 Tent Filter
    //             -------  
    //            | 1 2 1 |
    // ( 1 / 16 ) | 2 4 2 |
    //            | 1 2 1 |
    //             -------
    float4 Fetch01 = g_TextureInput.Sample( g_LinearSampler, VertexStage.TexCoordinates - d.xy );
    float4 Fetch02 = g_TextureInput.Sample( g_LinearSampler, VertexStage.TexCoordinates - d.wy ) * 2;
    float4 Fetch03 = g_TextureInput.Sample( g_LinearSampler, VertexStage.TexCoordinates - d.zy );

    float4 Fetch04 = g_TextureInput.Sample( g_LinearSampler, VertexStage.TexCoordinates + d.zw ) * 2;
    float4 Fetch05 = g_TextureInput.Sample( g_LinearSampler, VertexStage.TexCoordinates ) * 4;
    float4 Fetch06 = g_TextureInput.Sample( g_LinearSampler, VertexStage.TexCoordinates + d.xw ) * 2;

    float4 Fetch07 = g_TextureInput.Sample( g_LinearSampler, VertexStage.TexCoordinates + d.zy );
    float4 Fetch08 = g_TextureInput.Sample( g_LinearSampler, VertexStage.TexCoordinates + d.wy ) * 2;
    float4 Fetch09 = g_TextureInput.Sample( g_LinearSampler, VertexStage.TexCoordinates + d.xy );
    
    // Sample the previous mip (since we want to accumulate mip levels)
    float4 PreviousMip = g_TextureAccumulatorInput.Sample( g_LinearSampler, VertexStage.TexCoordinates );
    float4 CurrentMip = ( Fetch01 + Fetch02 + Fetch03 + Fetch04 + Fetch05 + Fetch06 + Fetch07 + Fetch08 + Fetch09 ) * ( 1.0 / 16 );

    return ( PreviousMip + CurrentMip );
}
