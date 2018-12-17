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
#include "SharedAutoExposure.hlsli"

Texture2D mainRenderTarget : register( t2 );
Texture2D bloomRenderTarget : register( t3 );

SamplerState LinearSampler : register( s3 );

cbuffer PerPass : register( b2 )
{
    float g_BloomExposureCompensation; // -8.0f
    float g_BloomStrength; // 0.04f
    float g_WhitePoint; // 1.0f
    float g_BlackPoint; // 0.0f
    
    float g_JunctionPoint; // 0.50f
    float g_ToeStrength; // 0.50f
    float g_ShoulderStrength; // 0.50f
    uint explicitpadding;
}

float ToneMappingFilmic_Insomniac( float x )
{
    float	w = g_WhitePoint;	// White point
    float	b = g_BlackPoint;				// Black point
    float	c = g_JunctionPoint;				// Junction point
    float	t = g_ToeStrength;				// Toe strength
    float	s = g_ShoulderStrength;				// Shoulder strength
    float	k = ( 1.0f - t ) * ( c - b ) / ( ( 1.0f - s ) * ( w - c ) + ( 1.0f - t ) * ( c - b ) );				// Junction factor

    // "Optimized" version where coeffs for Toe and Shoulder could be passed as float4 to the CB
    float4	Coeffs_Toe = float4( k - k*t, -t, -k*b + k*b*t, c - b + t*b );
    float4	Coeffs_Shoulder = float4( k*( s - 1 ) + 1, s, k*( 1 - s )*( w - c ) - k*s*c - c*( 1 - k ), ( 1 - s )*( w - c ) - s*c );

    float4	Coeffs = x < c ? Coeffs_Toe : Coeffs_Shoulder;
    float2	Fraction = Coeffs.xy * x + Coeffs.zw;
    return Fraction.x / Fraction.y;
}

float3 ToneMappingFilmic_Insomniac( float3 Color )
{
    float Lum = rgbToLuminance( Color );
    float nLum = ToneMappingFilmic_Insomniac( Lum );
    float scale = nLum / Lum;

    return ( scale * Color );
}

float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;

float3 Uncharted2Tonemap(float3 x)
{
     return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float Vignette( const in float2 fragCoordinates )
{
	return 0.3 + 0.7 * pow( 16.0 * fragCoordinates.x * fragCoordinates.y * ( 1.0 - fragCoordinates.x ) * ( 1.0 - fragCoordinates.y ), 0.2 );
}

float3 InterleavedGradientNoise( float2 uv )
{
    const float3 magic = float3( 0.06711056, 0.00583715, 52.9829189 );
    return frac( magic.z * frac( dot( uv, magic.xy ) ) );
}

float4 FilmGrain( in float4 finalColor, const in float2 fragCoordinates )
{
    float strength = 8.0;

    float x = ( fragCoordinates.x + 4.0 ) * ( fragCoordinates.y + 4.0 ) * 10.0f;
    float g = fmod( ( fmod( x, 13.0 ) + 1.0 ) * ( fmod( x, 123.0 ) + 1.0 ), 0.01 ) - 0.005;
    float4 grain = float4( g, g, g, g ) * strength;
    grain = 1.0 - grain;

    return finalColor * grain;
}

float3 computeBloomLuminance( float3 bloomColor, float bloomEC, float currentEV )
{
    // currentEV is the value calculated at the previous frame
    float bloomEV = currentEV + bloomEC;

    // convert to luminance
    // See equation (12) for explanation about converting EV to luminance
    return bloomColor * pow( 2, bloomEV - 3 );
}

float4 EntryPointPS( psDataScreenQuad_t p ) : SV_TARGET0
{
    autoExposure_t currentExposure = ReadAutoExposureParameters();

    float currentEV = computeEV100FromAvgLuminance( currentExposure.EngineLuminanceFactor );
	float4 finalColor = mainRenderTarget.SampleLevel( LinearSampler, p.uv, 0.0 );

    // Add Bloom
    // NOTE Bloom must be done on a unexposed HDR render target (since we interpolate both unexposed render target below, and then tonemap)
    float4 bloomColor = bloomRenderTarget.Sample( LinearSampler, p.uv );
    float3 bloomLuminance = computeBloomLuminance( bloomColor.rgb, g_BloomExposureCompensation, currentEV );
    finalColor.rgb = lerp( finalColor.rgb, bloomLuminance.rgb, g_BloomStrength );

    // Apply Tonemapping
	 float ExposureBias = 2.0f;
     float3 curr = Uncharted2Tonemap(ExposureBias*finalColor.rgb * currentExposure.EngineLuminanceFactor);
     float3 whiteScale = 1.0f/Uncharted2Tonemap(W);
     float3 color = curr*whiteScale;
    
    //finalColor.rgb = ToneMappingFilmic_Insomniac( finalColor.rgb * currentExposure.EngineLuminanceFactor );
    
    // Do linear to SRGB colorspace conversion
    finalColor.rgb = accurateLinearToSRGB( finalColor.rgb );

	//finalColor *= Vignette( p.uv );
    //finalColor = FilmGrain( finalColor, p.uv );

    // Add a dithering pattern to attenuate color banding
    float3 rnd = InterleavedGradientNoise( p.position.xy ) / 255.0;
    finalColor.rgb = finalColor.rgb + rnd;
    
    // Register luma for post composition steps (e.g. FXAA)
    finalColor.a = CalcLuminance( finalColor.rgb );

	return finalColor;
}
