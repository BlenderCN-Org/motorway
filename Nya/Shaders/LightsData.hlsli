#ifndef __LIGHTS_DATA_H__
#define __LIGHTS_DATA_H__ 1
#include <Shared.h>

struct DirectionalLight
{
    float4 SunColorAndAngularRadius;
    float4 SunDirectionAndIlluminanceInLux;
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
    
    uint Flags; // IsCaptured; IsDynamic; IsFallbackProbe; UnusedFlags (1 byte per flag) 
    uint CaptureFrequencyPadded; // Frequency (16bits) / Padding(16bits)
    uint Index;
};

cbuffer LightsBuffer : register( b2 )
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
#endif
