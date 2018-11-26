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
#ifndef PA_DONT_RECEIVE_SHADOWS
#include <RenderableEntities.hlsli>
#include <CameraData.hlsli>
#include <ShadowMappingShared.h>

Texture2D ShadowMapTest : register( t15 );

float2 ComputeReceiverPlaneDepthBias( float3 texCoordDX, float3 texCoordDY )
{
    float2 biasUV;
    biasUV.x = texCoordDY.y * texCoordDX.z - texCoordDX.y * texCoordDY.z;
    biasUV.y = texCoordDX.x * texCoordDY.z - texCoordDY.x * texCoordDX.z;
    biasUV *= 1.0f / ( ( texCoordDX.x * texCoordDY.y ) - ( texCoordDX.y * texCoordDY.x ) );

    return biasUV;
}

float SampleCascadedShadowMap( in float2 base_uv, in float u, in float v, in float2 shadowMapSizeInv, in uint cascadeIdx, in float depth, in float2 receiverPlaneDepthBias )
{
    float2 uv = base_uv + float2( u, v ) * shadowMapSizeInv;

    float z = depth + dot( float2( u, v ) * shadowMapSizeInv, receiverPlaneDepthBias );

    float2 shiftedCoordinates = uv.xy;
    shiftedCoordinates.x = float( cascadeIdx ) / float(CSM_SLICE_COUNT);
    shiftedCoordinates.x += ( uv.x / float(CSM_SLICE_COUNT) );
    
    return ShadowMapTest.SampleCmpLevelZero( g_ShadowMapSampler, shiftedCoordinates.xy, z );
}

float SampleCascadedShadowMapOptimizedPCF( in float3 shadowPos, in float3 shadowPosDX, in float3 shadowPosDY, in uint cascadeIdx )
{
    static const float2 shadowMapSize = float2( CSM_SHADOW_MAP_DIMENSIONS, CSM_SHADOW_MAP_DIMENSIONS );

    float lightDepth = shadowPos.z;
    float2 texelSize = 1.0f / shadowMapSize;

    float2 receiverPlaneDepthBias = ComputeReceiverPlaneDepthBias( shadowPosDX, shadowPosDY );

    // Static depth biasing to make up for incorrect fractional sampling on the shadow map grid
    float fractionalSamplingError = 2 * dot( float2( 1.0f, 1.0f ) * texelSize, abs( receiverPlaneDepthBias ) );
    lightDepth -= min( fractionalSamplingError, 0.01f );

    float2 uv = shadowPos.xy * shadowMapSize; // 1 unit - 1 texel
    float2 shadowMapSizeInv = 1.0 / shadowMapSize;

    float2 base_uv;
    base_uv.x = floor( uv.x + 0.5 );
    base_uv.y = floor( uv.y + 0.5 );

    float s = ( uv.x + 0.5 - base_uv.x );
    float t = ( uv.y + 0.5 - base_uv.y );

    base_uv -= float2( 0.5, 0.5 );
    base_uv *= shadowMapSizeInv;

    float sum = 0;
    float uw0 = ( 5 * s - 6 );
    float uw1 = ( 11 * s - 28 );
    float uw2 = -( 11 * s + 17 );
    float uw3 = -( 5 * s + 1 );

    float u0 = ( 4 * s - 5 ) / uw0 - 3;
    float u1 = ( 4 * s - 16 ) / uw1 - 1;
    float u2 = -( 7 * s + 5 ) / uw2 + 1;
    float u3 = -s / uw3 + 3;

    float vw0 = ( 5 * t - 6 );
    float vw1 = ( 11 * t - 28 );
    float vw2 = -( 11 * t + 17 );
    float vw3 = -( 5 * t + 1 );

    float v0 = ( 4 * t - 5 ) / vw0 - 3;
    float v1 = ( 4 * t - 16 ) / vw1 - 1;
    float v2 = -( 7 * t + 5 ) / vw2 + 1;
    float v3 = -t / vw3 + 3;

    sum += uw0 * vw0 * SampleCascadedShadowMap( base_uv, u0, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw1 * vw0 * SampleCascadedShadowMap( base_uv, u1, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw2 * vw0 * SampleCascadedShadowMap( base_uv, u2, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw3 * vw0 * SampleCascadedShadowMap( base_uv, u3, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );

    sum += uw0 * vw1 * SampleCascadedShadowMap( base_uv, u0, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw1 * vw1 * SampleCascadedShadowMap( base_uv, u1, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw2 * vw1 * SampleCascadedShadowMap( base_uv, u2, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw3 * vw1 * SampleCascadedShadowMap( base_uv, u3, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );

    sum += uw0 * vw2 * SampleCascadedShadowMap( base_uv, u0, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw1 * vw2 * SampleCascadedShadowMap( base_uv, u1, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw2 * vw2 * SampleCascadedShadowMap( base_uv, u2, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw3 * vw2 * SampleCascadedShadowMap( base_uv, u3, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );

    sum += uw0 * vw3 * SampleCascadedShadowMap( base_uv, u0, v3, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw1 * vw3 * SampleCascadedShadowMap( base_uv, u1, v3, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw2 * vw3 * SampleCascadedShadowMap( base_uv, u2, v3, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw3 * vw3 * SampleCascadedShadowMap( base_uv, u3, v3, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );

    return sum * 1.0f / 2704;
}

float3 GetCascadedShadowMapShadowPosOffset( in float nDotL, in float3 normal )
{
    static const float texelSize = 2.0f / CSM_SHADOW_MAP_DIMENSIONS;
    float nmlOffsetScale = saturate( 1.0f - nDotL );
    return texelSize * 1.0f * nmlOffsetScale * normal;
}

float3 SampleShadowCascade( in DirectionalLight light, in float3 shadowPosition, in float3 shadowPosDX, in float3 shadowPosDY, in uint cascadeIdx )
{
    shadowPosition += CascadeOffsets[cascadeIdx].xyz;
    shadowPosition *= CascadeScales[cascadeIdx].xyz;

    shadowPosDX *= CascadeScales[cascadeIdx].xyz;
    shadowPosDY *= CascadeScales[cascadeIdx].xyz;

    float3 cascadeColor = 1.0f;

     const float3 CascadeColors[4] =
     {
        float3( 1.0f, 0.0, 0.0f ),
        float3( 0.0f, 1.0f, 0.0f ),
        float3( 0.0f, 0.0f, 1.0f ),
        float3( 1.0f, 1.0f, 0.0f )
     };

     cascadeColor = CascadeColors[cascadeIdx];

    return  + SampleCascadedShadowMapOptimizedPCF( shadowPosition, shadowPosDX, shadowPosDY, cascadeIdx );
}
#endif
