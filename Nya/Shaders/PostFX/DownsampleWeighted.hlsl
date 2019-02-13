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

// Next Generation Post Processing in Call of Duty AW [Jorge Jimenez, 2014]
// http://www.iryoku.com/downloads/Next-Generation-Post-Processing-in-Call-of-Duty-Advanced-Warfare-v18.pptx

Texture2D<float4> g_InputRenderTarget : register( t0 );

sampler g_BilinearSampler : register( s0 );

RWTexture2D<float4> g_DownsampledRenderTarget : register( u0 );

// RGB to luminance
inline float rgbToLuminance( float3 colour )
{
    return dot( float3( 0.2126f, 0.7152f, 0.0722f ), colour );
}

groupshared float4 g_Samples;

[numthreads( 16, 16, 1 )]
void EntryPointCS( uint2 id : SV_DispatchThreadID )
{
    float textureWidth, textureHeight;
    g_InputRenderTarget.GetDimensions( textureWidth, textureHeight );

    float2 textureDimensions = float2( textureWidth, textureHeight );
    float2 inverseTextureDimensions = 1 / textureDimensions;

    float2 UV = ( float2( id ) + 0.5 ) * inverseTextureDimensions;

    // Custom hand-crafted 36-texel downsample (13 bilinear fetches)
    float4 Fetch01 = g_InputRenderTarget.SampleLevel( g_BilinearSampler, UV, 0, int2( -1, -1 ) );
    float4 Fetch02 = g_InputRenderTarget.SampleLevel( g_BilinearSampler, UV, 0, int2( +1, -1 ) );
    float4 Fetch03 = g_InputRenderTarget.SampleLevel( g_BilinearSampler, UV, 0, int2( +1, +1 ) );
    float4 Fetch04 = g_InputRenderTarget.SampleLevel( g_BilinearSampler, UV, 0, int2( -1, +1 ) );
    float4 Fetch05 = g_InputRenderTarget.SampleLevel( g_BilinearSampler, UV, 0, int2( -2, -2 ) );
    float4 Fetch06 = g_InputRenderTarget.SampleLevel( g_BilinearSampler, UV, 0, int2( +0, -2 ) );
    float4 Fetch07 = g_InputRenderTarget.SampleLevel( g_BilinearSampler, UV, 0, int2( +0, +0 ) );
    float4 Fetch08 = g_InputRenderTarget.SampleLevel( g_BilinearSampler, UV, 0, int2( -2, +0 ) );
    float4 Fetch09 = g_InputRenderTarget.SampleLevel( g_BilinearSampler, UV, 0, int2( +2, -2 ) );
    float4 Fetch10 = g_InputRenderTarget.SampleLevel( g_BilinearSampler, UV, 0, int2( +2, +0 ) );
    float4 Fetch11 = g_InputRenderTarget.SampleLevel( g_BilinearSampler, UV, 0, int2( +2, +2 ) );
    float4 Fetch12 = g_InputRenderTarget.SampleLevel( g_BilinearSampler, UV, 0, int2( +0, +2 ) );
    float4 Fetch13 = g_InputRenderTarget.SampleLevel( g_BilinearSampler, UV, 0, int2( -2, +2 ) );

#if NYA_USE_KARIS_AVERAGE
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
#else
    // Weighting fetches
    float4 Weighted1 = ( Fetch01 + Fetch02 + Fetch03 + Fetch04 ) * 0.500f;
    float4 Weighted2 = ( Fetch05 + Fetch06 + Fetch07 + Fetch08 ) * 0.125f;
    float4 Weighted3 = ( Fetch06 + Fetch09 + Fetch10 + Fetch07 ) * 0.125f;
    float4 Weighted4 = ( Fetch07 + Fetch10 + Fetch11 + Fetch12 ) * 0.125f;
    float4 Weighted5 = ( Fetch08 + Fetch07 + Fetch12 + Fetch13 ) * 0.125f;

	static const float InvWeightSum = 1.0f;
#endif
	
	GroupMemoryBarrierWithGroupSync();

    // Sum them' up	
    float4 TexelValue = ( Weighted1 + Weighted2 + Weighted3 + Weighted4 + Weighted5 ) * InvWeightSum;

    g_DownsampledRenderTarget.GetDimensions( textureWidth, textureHeight );
	
	float2 downsampledTextureDimensions = ceil( textureDimensions * 0.5f );
	int2 texelCoordinates = UV * float2( textureWidth, textureHeight );
	
	g_DownsampledRenderTarget[texelCoordinates] = TexelValue;
}
