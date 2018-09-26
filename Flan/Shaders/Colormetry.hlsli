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
float3 accurateSRGBToLinear( in float3 sRGBCol )
{
    float3 linearRGBLo = sRGBCol / 12.92;

    // Ignore X3571, incoming vector will always be superior to 0
    float3 linearRGBHi = pow( abs( ( sRGBCol + 0.055 ) / 1.055 ), 2.4 );

    float3 linearRGB = ( sRGBCol <= 0.04045 ) ? linearRGBLo : linearRGBHi;

    return linearRGB;
}

float3 fastLinearToSRGB( in float3 linearCol )
{
    return pow( abs( linearCol ), ( 1.0 / 2.2 ) );
}

float3 accurateLinearToSRGB( in float3 linearCol )
{
    float3 sRGBLo = linearCol * 12.92;
    float3 sRGBHi = ( pow( abs( linearCol ), 1.0 / 2.4 ) * 1.055 ) - 0.055;
    float3 sRGB = ( linearCol <= 0.0031308 ) ? sRGBLo : sRGBHi;
    return sRGB;
}

// Based on The Order : 1886 SIGGRAPH course notes implementation
float adjustRoughness( in float inputRoughness, in float avgNormalLength )
{
    float adjustedRoughness = inputRoughness;

    if ( avgNormalLength < 1.0f ) {
        float avgNormLen2 = avgNormalLength * avgNormalLength;
        float kappa = ( 3 * avgNormalLength - avgNormalLength * avgNormLen2 ) / ( 1 - avgNormLen2 );
        float variance = 1.0f / ( 2.0 * kappa );

        adjustedRoughness = sqrt( inputRoughness * inputRoughness + variance );
    }

    return adjustedRoughness;
}

// RGB to luminance
float rgbToLuminance( float3 colour )
{
    // Use this equation: http://en.wikipedia.org/wiki/Luminance_(relative)
    return dot( float3( 0.2126f, 0.7152f, 0.0722f ), colour );
}

float CalcLuminance( float3 color )
{
    return dot( color, float3( 0.299f, 0.587f, 0.114f ) );
}

// RGB to log-luminance
float rgbToLogLuminance( float3 colour )
{
    return log( rgbToLuminance( colour ) );
}

float3 ACESFilm( float3 x )
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;

    return saturate( ( x*( a*x + b ) ) / ( x*( c*x + d ) + e ) );
}

float3 ACESFilmRec2020( float3 x )
{
    float a = 15.8f;
    float b = 2.12f;
    float c = 1.2f;
    float d = 5.92f;
    float e = 1.9f;
    return ( x * ( a * x + b ) ) / ( x * ( c * x + d ) + e );
}

float ToneMappingFilmic_Insomniac( float x )
{
    float	w = 1.0f;	// White point
    float	b = 0.00f;				// Black point
    float	c = 0.50f;				// Junction point
    float	t = 0.0f;				// Toe strength
    float	s = 0.00f;				// Shoulder strength
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

float computeEV100FromAvgLuminance( float avg_luminance )
{
    // We later use the middle gray at 12.7% in order to have
    // a middle gray at 18% with a sqrt (2) room for specular highlights
    // But here we deal with the spot meter measuring the middle gray
    // which is fixed at 12.5 for matching standard camera
    // constructor settings (i.e. calibration constant K = 12.5)
    return log2( avg_luminance * 100.0 / 12.5 );
}

float convertEV100ToExposure( float EV100 )
{
    // Compute the maximum luminance possible with H_sbs sensitivity
    // maxLum = 78 / ( S * q ) * N^2 / t
    // = 78 / ( S * q ) * 2^ EV_100
    // = 78 / (100 * 0.65) * 2^ EV_100
    // = 1.2 * 2^ EV
    float max_luminance = 1.2 * pow( 2.0, EV100 );
    return 1.0 / max_luminance;
}
