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

SamplerState LinearSampler { Filter = MIN_MAG_LINEAR_MIP_POINT; AddressU = Clamp; AddressV = Clamp; };
Texture2D mainRenderTarget : register( t0 );

cbuffer BlurInfos : register( b0 )
{
    float   Sigma;
    int     IsHorizontal;
    int2    InputSize;
};

// Calculates the gaussian blur weight for a given distance and sigmas
float CalcGaussianWeight( int sampleDist, float sigma )
{
    float g = 1.0f / sqrt( 2.0f * PI * sigma * sigma );
    return ( g * exp( -( sampleDist * sampleDist ) / ( 2 * sigma * sigma ) ) );
}

float4 EntryPointPS( psDataScreenQuad_t p ) : SV_TARGET
{
    float4 color = 0;

    half2 inputSizeFloat = InputSize;
    float2 samplingOffset = IsHorizontal == 1 ? float2( 1, 0 ) : float2( 0, 1 );

    for ( int i = -6; i < 6; i++ ) {
        float weight = CalcGaussianWeight( i, Sigma );
        float2 texCoord = p.uv;
        texCoord += ( i / inputSizeFloat ) * samplingOffset;
        float4 sample = mainRenderTarget.Sample( LinearSampler, texCoord );
        color += sample * weight;
    }

    return color;
}
