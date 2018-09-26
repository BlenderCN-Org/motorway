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
#include <Colormetry.hlsli>

// Next Generation Post Processing in Call of Duty AW [Jorge Jimenez, 2017]
// http://www.iryoku.com/downloads/Next-Generation-Post-Processing-in-Call-of-Duty-Advanced-Warfare-v18.pptx

Texture2D g_TextureInput : register( t3 );

float4 EntryPointPS( ScreenSpaceVertexStageData VertexStage ) : SV_TARGET0
{
    float textureWidth, textureHeight;
    g_TextureInput.GetDimensions( textureWidth, textureHeight );

    float2 textureDimensions = float2( textureWidth, textureHeight );
    float2 inverseTextureDimensions = 1 / textureDimensions;

    float2 TexelUV = VertexStage.TexCoordinates.xy * textureDimensions;
    float2 Texel = floor( TexelUV );
    float2 UV = ( Texel + 0.5 ) * inverseTextureDimensions;

    // Custom hand-crafted 36-texel downsample (13 bilinear fetches)
    float4 Fetch01 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( -1, -1 ) );
    float4 Fetch02 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +1, -1 ) );
    float4 Fetch03 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +1, +1 ) );
    float4 Fetch04 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( -1, +1 ) );
    float4 Fetch05 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( -2, -2 ) );
    float4 Fetch06 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +0, -2 ) );
    float4 Fetch07 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +0, +0 ) );
    float4 Fetch08 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( -2, +0 ) );
    float4 Fetch09 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +2, -2 ) );
    float4 Fetch10 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +2, +0 ) );
    float4 Fetch11 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +2, +2 ) );
    float4 Fetch12 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +0, +2 ) );
    float4 Fetch13 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( -2, +2 ) );

    // Weighting fetches
    float4 Weighted1 = ( Fetch01 + Fetch02 + Fetch03 + Fetch04 ) * 0.500f;
    float4 Weighted2 = ( Fetch05 + Fetch06 + Fetch07 + Fetch08 ) * 0.125f;
    float4 Weighted3 = ( Fetch06 + Fetch09 + Fetch10 + Fetch07 ) * 0.125f;
    float4 Weighted4 = ( Fetch07 + Fetch10 + Fetch11 + Fetch12 ) * 0.125f;
    float4 Weighted5 = ( Fetch08 + Fetch07 + Fetch12 + Fetch13 ) * 0.125f;

    // Sum them up
    return ( Weighted1 + Weighted2 + Weighted3 + Weighted4 + Weighted5 );
}

// Karis average for MIP0 to MIP1 downsample
float4 EntryPointPS_Karis(ScreenSpaceVertexStageData VertexStage ) : SV_TARGET0
{
    // TODO Compare performances between dynamic retrival and uniform pass 
    float textureWidth, textureHeight;
    g_TextureInput.GetDimensions( textureWidth, textureHeight );

    float2 textureDimensions = float2( textureWidth, textureHeight );
    float2 inverseTextureDimensions = 1 / textureDimensions;

    float2 TexelUV = VertexStage.TexCoordinates.xy * textureDimensions;
    float2 Texel = floor( TexelUV );
    float2 UV = ( Texel + 0.5 ) * inverseTextureDimensions;

    // Custom hand-crafted 36-texel downsample (13 bilinear fetches)
    float4 Fetch01 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( -1, -1 ) );
    float4 Fetch02 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +1, -1 ) );
    float4 Fetch03 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +1, +1 ) );
    float4 Fetch04 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( -1, +1 ) );

    float4 Fetch05 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( -2, -2 ) );
    float4 Fetch06 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +0, -2 ) );
    float4 Fetch07 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +0, +0 ) );
    float4 Fetch08 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( -2, +0 ) );

    float4 Fetch09 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +2, -2 ) );
    float4 Fetch10 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +2, +0 ) );
    float4 Fetch11 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +2, +2 ) );
    float4 Fetch12 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( +0, +2 ) );
    float4 Fetch13 = g_TextureInput.SampleLevel( g_LinearSampler, UV, 0, int2( -2, +2 ) );

    // Partial Karis Average (apply the Karis average in blocks of 4 samples)
    float4 BlockFetch01 = ( Fetch01 + Fetch02 + Fetch03 + Fetch04 );
    float Weight01 = 1.0 / ( rgbToLuminance( BlockFetch01.rgb ) + 1 );

    float4 BlockFetch02 = ( Fetch05 + Fetch06 + Fetch07 + Fetch08 );
    float Weight02 = 1.0 / ( rgbToLuminance( BlockFetch02.rgb ) + 1 );

    float4 BlockFetch03 = ( Fetch06 + Fetch09 + Fetch10 + Fetch07 );
    float Weight03 = 1.0 / ( rgbToLuminance( BlockFetch03.rgb ) + 1 );

    float4 BlockFetch04 = ( Fetch07 + Fetch10 + Fetch11 + Fetch12 );
    float Weight04 = 1.0 / ( rgbToLuminance( BlockFetch04.rgb ) + 1 );

    float4 BlockFetch05 = ( Fetch08 + Fetch07 + Fetch12 + Fetch13 );
    float Weight05 = 1.0 / ( rgbToLuminance( BlockFetch05.rgb ) + 1 );

    // Compute the weight sum (for normalization)
    float WeightSum = ( Weight01 + Weight02 + Weight03 + Weight04 + Weight05 );
    float InvWeightSum = 1 / WeightSum;

    // Weighting fetches
    float4 Weighted1 = BlockFetch01 * Weight01;
    float4 Weighted2 = BlockFetch02 * Weight02;
    float4 Weighted3 = BlockFetch03 * Weight03;
    float4 Weighted4 = BlockFetch04 * Weight04;
    float4 Weighted5 = BlockFetch05 * Weight05;

    // Sum them' up
    return ( Weighted1 + Weighted2 + Weighted3 + Weighted4 + Weighted5 ) * InvWeightSum;
}
