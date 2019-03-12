#ifndef __LIGHTS_DATA_H__
#define __LIGHTS_DATA_H__ 1
#include <Shared.h>

struct DirectionalLight
{
    float4      SunColorAndAngularRadius;
    float4      SunDirectionAndIlluminanceInLux;
    float4      SunReservedData;
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

struct IBLProbe
{
    float4 PositionAndRadius;
    float4x4 InverseModelMatrix;
    uint Index;
    
    uint Flags; // IsCaptured; IsDynamic; IsFallbackProbe; UnusedFlags (1 byte per flag) 
    uint CaptureFrequencyPadded; // Frequency (16bits) / Padding(16bits)
};

cbuffer LightsBuffer : register( b2 )
{
    PointLight          g_PointLights[MAX_POINT_LIGHT_COUNT];
    IBLProbe            g_IBLProbes[MAX_IBL_PROBE_COUNT];
    DirectionalLight    g_DirectionalLight;
};
#endif
