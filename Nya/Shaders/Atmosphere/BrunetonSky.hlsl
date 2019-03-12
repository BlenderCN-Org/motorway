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
#ifdef USE_LUMINANCE
#define GetSolarRadiance GetSolarLuminance
#define GetSkyRadiance GetSkyLuminance
#define GetSkyRadianceToPoint GetSkyLuminanceToPoint
#define GetSunAndSkyIrradiance GetSunAndSkyIlluminance
#endif

#include <CameraData.hlsli>
#include <AutoExposure/SharedAutoExposure.hlsli>
#include "Atmosphere.hlsli"

cbuffer AtmosphereBuffer : register( b1 )
{
    float3  g_EarthCenter;
    float   g_SunSizeX;
    float3  g_SunDirection;
    float   g_SunSizeY;
};

struct DEFAULT_VS_OUT 
{ 
    float4 position : SV_Position; 
    float4 viewRay : POSITION0;
};

DEFAULT_VS_OUT EntryPointVS( uint id : SV_VERTEXID )
{
    DEFAULT_VS_OUT output;
    
    float x = ( float )( id / 2 );
    float y = ( float )( id % 2 );

    // Clip-space position
    output.position = float4(
        x * 4.0 - 1.0,
        y * 4.0 - 1.0,
        0.0,
        1.0
    );

    output.viewRay = mul( output.position, g_InverseViewProjectionMatrix ).xzyw;

    return output;
}

StructuredBuffer<AutoExposureInfos> AutoExposureBuffer : register( t8 );

float4 EntryPointPS( in DEFAULT_VS_OUT VertexStage ) : SV_TARGET
{
    float3 viewDirection = normalize( VertexStage.viewRay.xyz );

    float3 transmittance;
    float3 radiance = GetSkyRadiance( float3( g_WorldPosition.xz * 0.05f, 0.05f ) - g_EarthCenter, viewDirection, 0, g_SunDirection, transmittance );

#if NYA_FIXED_EXPOSURE
    float4 color = float4( 1.0 - exp( -radiance / 0.10f ), 1.0f );
#else
    AutoExposureInfos currentExposure = GetAutoExposureParameters( AutoExposureBuffer );
    float4 color = float4( 1.0 - exp( -radiance / ( currentExposure.EngineLuminanceFactor * 0.10f ) ), 1.0f );
#endif

#if NYA_RENDER_SUN_DISC
    [branch]
    if ( dot( viewDirection, g_SunDirection ) > g_SunSizeY ) {
        color.rgb += ( transmittance * GetSolarRadiance() * 0.0002f );
    }
#endif

    return color;
}
