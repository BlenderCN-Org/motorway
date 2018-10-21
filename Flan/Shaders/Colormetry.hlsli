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
