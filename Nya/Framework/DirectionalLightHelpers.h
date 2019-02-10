/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

    This file is part of Project Motorway source code.

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

#pragma once

#include <Shaders/ShadowMappingShared.h>

#include "Cameras/Camera.h"
#include "Light.h"

namespace
{
    // CSM Settings
    static constexpr float SHADOW_MAP_DIM = static_cast<float>( CSM_SHADOW_MAP_DIMENSIONS );

    static constexpr float MinDistance = 0.0f;
    static constexpr float MaxDistance = 1.0f;

    static constexpr float nearClip = 1.0f;
    static constexpr float farClip = 512.0f;
    static constexpr float clipRange = farClip - nearClip;

    static constexpr float minZ = nearClip + MinDistance * clipRange;
    static constexpr float maxZ = nearClip + MaxDistance * clipRange;

    static constexpr float range = maxZ - minZ;
    static constexpr float ratio = maxZ / minZ;

    // Compute the split distances based on the partitioning mode
    static constexpr float CascadeSplits[4] = {
        MinDistance + 0.100f * MaxDistance,
        MinDistance + 0.250f * MaxDistance,
        MinDistance + 0.500f * MaxDistance,
        MinDistance + 1.000f * MaxDistance
    };

    static nyaVec3f TransformVec3( const nyaVec3f& vector, const glm::mat4& matrix )
    {
        float x = ( vector.x * matrix[0].x ) + ( vector.y * matrix[1].x ) + ( vector.z * matrix[2].x ) + matrix[3].x;
        float y = ( vector.x * matrix[0].y ) + ( vector.y * matrix[1].y ) + ( vector.z * matrix[2].y ) + matrix[3].y;
        float z = ( vector.x * matrix[0].z ) + ( vector.y * matrix[1].z ) + ( vector.z * matrix[2].z ) + matrix[3].z;
        float w = ( vector.x * matrix[0].w ) + ( vector.y * matrix[1].w ) + ( vector.z * matrix[2].w ) + matrix[3].w;

        return nyaVec3f( x, y, z ) / w;
    }
}

namespace nya
{
    namespace framework
    {
        static glm::mat4 CSMCreateGlobalShadowMatrix( const nyaVec3f& lightDirNormalized, const glm::mat4& viewProjection )
        {
            // Get the 8 points of the view frustum in world space
            nyaVec3f frustumCorners[8] = {
                nyaVec3f( -1.0f, +1.0f, +0.0f ),
                nyaVec3f( +1.0f, +1.0f, +0.0f ),
                nyaVec3f( +1.0f, -1.0f, +0.0f ),
                nyaVec3f( -1.0f, -1.0f, +0.0f ),
                nyaVec3f( -1.0f, +1.0f, +1.0f ),
                nyaVec3f( +1.0f, +1.0f, +1.0f ),
                nyaVec3f( +1.0f, -1.0f, +1.0f ),
                nyaVec3f( -1.0f, -1.0f, +1.0f ),
            };

            glm::mat4 invViewProjection = glm::inverse( viewProjection );

            nyaVec3f frustumCenter( 0.0f );
            for ( uint64_t i = 0; i < 8; ++i ) {
                frustumCorners[i] = TransformVec3( frustumCorners[i], invViewProjection );
                frustumCenter += frustumCorners[i];
            }

            frustumCenter /= 8.0f;

            // Pick the up vector to use for the light camera
            const nyaVec3f upDir = nyaVec3f( 0.0f, 1.0f, 0.0f );

            // Get position of the shadow camera
            nyaVec3f shadowCameraPos = frustumCenter + lightDirNormalized * -0.5f;

            // Create a new orthographic camera for the shadow caster
            glm::mat4 shadowCamera = glm::orthoLH( -0.5f, +0.5f, -0.5f, +0.5f, +0.0f, +1.0f );
            glm::mat4 shadowLookAt = glm::lookAtLH( shadowCameraPos, frustumCenter, upDir );
            glm::mat4 shadowMatrix = shadowCamera * shadowLookAt;

            // Use a 4x4 bias matrix for texel sampling
            const glm::mat4 texScaleBias = glm::mat4( 
                +0.5f, +0.0f, +0.0f, +0.0f,
                +0.0f, -0.5f, +0.0f, +0.0f,
                +0.0f, +0.0f, +1.0f, +0.0f,
                +0.5f, +0.5f, +0.0f, +1.0f );

            return ( texScaleBias * shadowMatrix );
        }

        void CSMComputeSliceData( DirectionalLight* light, const int cascadeIdx, CameraData& cameraData )
        {
            DirectionalLightData& lightData = light->getLightData();

            // Get the 8 points of the view frustum in world space
            nyaVec3f frustumCornersWS[8] = {
                nyaVec3f( -1.0f,  1.0f, 0.0f ),
                nyaVec3f( 1.0f,  1.0f, 0.0f ),
                nyaVec3f( 1.0f, -1.0f, 0.0f ),
                nyaVec3f( -1.0f, -1.0f, 0.0f ),
                nyaVec3f( -1.0f,  1.0f, 1.0f ),
                nyaVec3f( 1.0f,  1.0f, 1.0f ),
                nyaVec3f( 1.0f, -1.0f, 1.0f ),
                nyaVec3f( -1.0f, -1.0f, 1.0f ),
            };

            float prevSplitDist = ( cascadeIdx == 0 ) ? MinDistance : CascadeSplits[cascadeIdx - 1];
            float splitDist = CascadeSplits[cascadeIdx];

            auto inverseViewProjection = glm::inverse( cameraData.depthViewProjectionMatrix );

            for ( int i = 0; i < 8; ++i ) {
                frustumCornersWS[i] = TransformVec3( frustumCornersWS[i], inverseViewProjection );
            }

            // Get the corners of the current cascade slice of the view frustum
            for ( int i = 0; i < 4; ++i ) {
                nyaVec3f cornerRay = frustumCornersWS[i + 4] - frustumCornersWS[i];
                nyaVec3f nearCornerRay = cornerRay * prevSplitDist;
                nyaVec3f farCornerRay = cornerRay * splitDist;
                frustumCornersWS[i + 4] = frustumCornersWS[i] + farCornerRay;
                frustumCornersWS[i] = frustumCornersWS[i] + nearCornerRay;
            }

            // Calculate the centroid of the view frustum slice
            nyaVec3f frustumCenter( 0.0f );
            for ( int i = 0; i < 8; ++i ) {
                frustumCenter = frustumCenter + frustumCornersWS[i];
            }

            frustumCenter *= 1.0f / 8.0f;

            nyaVec3f minExtents;
            nyaVec3f maxExtents;

            // Pick the up vector to use for the light camera
            const nyaVec3f upDir = nyaVec3f( 0, 1, 0 );

            float sphereRadius = 0.0f;
            for ( int i = 0; i < 8; ++i ) {
                float dist = glm::length( frustumCornersWS[i] - frustumCenter );
                sphereRadius = glm::max( sphereRadius, dist );
            }

            sphereRadius = std::ceil( sphereRadius * 16.0f ) / 16.0f;

            maxExtents = nyaVec3f( sphereRadius, sphereRadius, sphereRadius );
            minExtents = -maxExtents;

            nyaVec3f cascadeExtents = maxExtents - minExtents;

            // Get position of the shadow camera
            nyaVec3f shadowCameraPos = frustumCenter + lightData.direction * -minExtents.z;

            // Come up with a new orthographic camera for the shadow caster
            glm::mat4 shadowCamera = glm::orthoLH( minExtents.x, maxExtents.x, minExtents.y,
                maxExtents.y, 0.0f, cascadeExtents.z );
            glm::mat4 shadowLookAt = glm::lookAtLH( shadowCameraPos, frustumCenter, upDir );
            glm::mat4 shadowMatrix = shadowCamera * shadowLookAt;

            // Create the rounding matrix, by projecting the world-space origin and determining
            // the fractional offset in texel space
            nyaVec3f shadowOrigin = nyaVec3f( 0.0f );
            shadowOrigin = TransformVec3( shadowOrigin, shadowMatrix );
            shadowOrigin = shadowOrigin * ( SHADOW_MAP_DIM / 2.0f );

            nyaVec3f roundedOrigin = glm::round( shadowOrigin );

            nyaVec3f roundOffset = roundedOrigin - shadowOrigin;
            roundOffset = roundOffset * ( 2.0f / SHADOW_MAP_DIM );
            roundOffset.z = 0.0f;

            shadowCamera[3].x += roundOffset.x;
            shadowCamera[3].y += roundOffset.y;
            shadowCamera[3].z += roundOffset.z;

            shadowMatrix = shadowCamera * shadowLookAt;
            cameraData.shadowViewMatrix[cascadeIdx] = glm::transpose( shadowMatrix );

            // Apply the scale/offset matrix, which transforms from [-1,1]
            // post-projection space to [0,1] UV space
            glm::mat4 texScaleBias( 0.5f, 0.0f, 0.0f, 0.0f,
                0.0f, -0.5f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.5f, 0.5f, 0.0f, 1.0f );

            shadowMatrix = texScaleBias * shadowMatrix;

            // Store the split distance in terms of view space depth
            cameraData.cascadeSplitDistances[cascadeIdx] = nearClip + splitDist * clipRange;

            // Calculate the position of the lower corner of the cascade partition, in the UV space
            // of the first cascade partition
            glm::mat4 invCascadeMat = glm::inverse( shadowMatrix );
            nyaVec3f cascadeCorner = TransformVec3( nyaVec3f( 0.0f ), invCascadeMat );
            cascadeCorner = TransformVec3( cascadeCorner, cameraData.globalShadowMatrix );

            // Do the same for the upper corner
            nyaVec3f otherCorner = TransformVec3( nyaVec3f( 1.0f ), invCascadeMat );
            otherCorner = TransformVec3( otherCorner, cameraData.globalShadowMatrix );

            // Calculate the scale and offset
            nyaVec3f cascadeScale = nyaVec3f( 1.0f, 1.0f, 1.0f ) / ( otherCorner - cascadeCorner );

            cameraData.cascadeOffsets[cascadeIdx] = nyaVec4f( -cascadeCorner, 0.0f );
            cameraData.cascadeScales[cascadeIdx] = nyaVec4f( cascadeScale, 1.0f );
        }
    }
}
