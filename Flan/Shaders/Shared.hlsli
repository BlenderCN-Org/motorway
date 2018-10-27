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
#include <Shared.h>

// NOTE This header should only contain critical shared stuff (global objects shared between any pipeline state, persistent buffers, useful constants, ...)
// If you can avoid including this header in your shader, do it
#ifndef SAMPLER_POSTFX
sampler PostEffectSampler : register( s2 );
#define SAMPLER_POSTFX PostEffectSampler
#endif

#ifndef __SHARED__
#define __SHARED__
// Constants
static const float PI = 3.1415926535897932384626433f;
static const float INV_PI = ( 1.0 / PI );

// Shared samplers
// DO NOT ALTER THIS SAMPLERS (materials use these registers)
sampler GeometrySampler : register( s1 );
//sampler LinearSampler : register( s3 );
SamplerComparisonState ShadowMapSampler : register( s4 );
sampler LinearMirrorSampler : register( s5 );
sampler LinearWrapSampler : register( s6 );
//sampler PointSampler : register( s7 );

struct psDataScreenQuad_t
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// TODO SHOULDNT BE HERE!
struct DirectionalLight
{
    float4      SunColorAndAngularRadius;
    float4      SunDirectionAndIlluminanceInLux;
};

struct PointLight
{
    float4 PositionAndRadius;
    float4 ColorAndPowerInLux;
};

struct SpotLight
{
    float4  PositionAndRadius;
    float4  ColorAndPowerInLux;
    float4  LightDirectionAndFallOffRadius;
    float   Radius;
    float   InvCosConeDifference;
    float2  EXPLICIT_PADDING;
};

struct SphereLight
{
    float4 PositionAndRadius;
    float4 ColorAndPowerInLux;
};

struct DiscLight
{
    float4 PositionAndRadius;
    float4 ColorAndPowerInLux;
    float3 PlaneNormal;
};

struct RectangleLight
{
    float4  PositionAndRadius;
    float4  ColorAndPowerInLux;
    float3  PlaneNormal;
    float   Width;
    float3  UpVector;
    float   Height;
    float3  LeftVector;
    uint    IsTubeLight;
};

struct EnvironmentProbe
{
    float4 PositionAndRadiusWorldSpace;
    float4x4 InverseModelMatrix;
    
    double RenderKey;
    uint Flags; // IsCaptured; IsDynamic; IsFallbackProbe; UnusedFlags (1 byte per flag) 
    uint CaptureFrequencyPadded;

    float3 PADDING;
    uint Index;
};

cbuffer LightData : register( b1 )
{
    uint                DirectionalLightCount;
    uint                PointLightCount;
    uint                SpotLightCount;
    uint                SphereLightCount;
    uint                DiscLightCount;
    uint                RectangleAndTubeLightCount;
    uint                LocalEnvironmentProbeCount;
    uint                GlobalEnvironmentProbeCount;

    DirectionalLight    DirectionalLights[MAX_DIRECTIONAL_LIGHT_COUNT];
    PointLight          PointLights[MAX_POINT_LIGHT_COUNT];
    SpotLight           SpotLights[MAX_SPOT_LIGHT_COUNT];
    SphereLight         SphereLights[MAX_SPHERE_LIGHT_COUNT];
    DiscLight           DiscLights[MAX_DISC_LIGHT_COUNT];
    RectangleLight      RectangleAndTubeLights[MAX_RECTANGLE_LIGHT_COUNT];

    EnvironmentProbe    GlobalEnvProbes[MAX_GLOBAL_ENVIRONMENT_PROBE_COUNT];
    EnvironmentProbe    LocalEnvProbes[MAX_LOCAL_ENVIRONMENT_PROBE_COUNT];
};
// END TODO

cbuffer ActiveCameraBuffer : register( b2 )
{
    float4x4	ViewMatrix;
    float4x4	ProjectionMatrix;
    float4x4	InverseViewMatrix;
    float4x4	InverseProjectionMatrix;
    float4x4	ViewProjectionMatrix;
    float4x4	g_InverseViewProjectionMatrix;
    float4		WorldPosition;
    float4x4	DepthProjectionMatrix;
    float4x4	DepthViewProjectionMatrix;
    float4      CascadeOffsets[4];
    float4      CascadeScales[4];
    float4      ShadowSplitDistances;
    float4x4    ShadowMatrices[4];
    float4x4    ShadowMatrixShared;
};

cbuffer RenderInfos : register( b4 )
{
    float   TimeDelta;  // In Seconds
    float   g_WorldTime;  // World time; not system nor engine one
    uint   g_FrameNumber;
    uint   RESERVED_RENDERINFOS;
    uint2  BackbufferDimension;
}

cbuffer AtmosphereBuffer : register( b5 )
{
    float3  g_EarthCenter;
    float   g_SunSizeX;
    float3  g_SunDirection;
    float   g_SunSizeY;
};

// TODO Shouldn't be here too
// Calculate the number of tiles in the horizontal direction
uint GetNumTilesX()
{
    return ( uint )( ( BackbufferDimension.x + TILE_RES - 1 ) / (float)TILE_RES );
}

// Calculate the number of tiles in the vertical direction
uint GetNumTilesY()
{
    return ( uint )( ( BackbufferDimension.y + TILE_RES - 1 ) / (float)TILE_RES );
}

uint GetMaxNumLightsPerTile()
{
    uint clampedHeight = min( BackbufferDimension.y, 720 );
    return ( MAX_NUM_LIGHTS_PER_TILE - ( ADJUSTMENT_MULTIPLIER * ( clampedHeight / 120 ) ) );
}

float4 ScreenSpaceToViewSpace( in float3 pointScreenSpace, bool is_point )
{
    float4 pointClipSpace = float4( pointScreenSpace.xy * float2( 2, 2 ) - float2( 1, 1 ), pointScreenSpace.z, is_point );
    pointClipSpace.y *= -1;

    float4 pointViewSpace = mul( pointClipSpace, InverseProjectionMatrix );
    pointViewSpace.xyz /= pointViewSpace.w;

    return pointViewSpace;
}
/*
inline float4 WorldSpaceToPreviousViewSpace( in float4 pointWorldSpace, bool is_point )
{
    // World Space to N-1 Frame View Space
    // Basically, this is only used for reprojection based methods (TAA, Viewport TR, Motion Vectors reprojection, ...)
    float4 worldPoint = mul( float4( pointWorldSpace.xyz, is_point ), g_PreviousViewProjectionMatrix );
    
    // Do perspective projection
    worldPoint.xyz /= worldPoint.w;

    return worldPoint;
}
*/
inline float4 WorldSpaceToViewSpace( in float4 pointWorldSpace )
{
    float4 pointViewSpace = mul( ViewProjectionMatrix, pointWorldSpace );

    // Do perspective projection
    pointViewSpace.xyz /= pointViewSpace.w;

    return pointViewSpace;
}

float2 ViewSpaceToScreenSpace( in float4 pointViewSpace )
{
    return ( pointViewSpace.xy * float2( 0.5f, -0.5f ) + float2( 0.5f, 0.5f ) );
}
#endif
