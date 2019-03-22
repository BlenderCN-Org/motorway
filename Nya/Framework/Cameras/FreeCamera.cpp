/*
    Project Motorway Source Code
    Copyright (C) 2018 Prévost Baptiste

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

#include "Shared.h"
#include "FreeCamera.h"

#include <FileSystem/FileSystemObject.h>

#include <Maths/Helpers.h>
#include <Maths/Trigonometry.h>
#include <Maths/Sampling.h>
#include <Maths/MatrixTransformations.h>

#include <cmath>

using namespace nya::maths;

FreeCamera::FreeCamera( const float camSpeed )
	: rightVector{ 0.0f }
	, eyeDirection{ 1.0f, 0.0f, 0.0f }
    , speedX( 0.0f )
    , speedY( 0.0f )
    , speedAltitude( 0.0f )
	, yaw( 0.0f )
	, pitch( 0.0f )
	, roll( 0.0f )
	, moveSpeed( camSpeed )
    , moveDamping( 1.0f )
    , aspectRatio( 1.0f )
    , fov( radians( 90.0f ) )
    , nearPlane( 0.1f )
{
    data = {};
}

void FreeCamera::update( const float frameTime )
{
    if ( yaw < -PI<float>() ) {
        yaw += TWO_PI<float>();
    } else if ( yaw > PI<float>() ) {
        yaw -= TWO_PI<float>();
    }

    pitch = clamp( pitch, -HALF_PI<float>(), HALF_PI<float>() );

    eyeDirection = {
        cosf( pitch ) * sinf( yaw ),
        sinf( pitch ),
        cosf( pitch ) * cosf( yaw )
    };
    eyeDirection = eyeDirection.normalize();

    const float yawResult = yaw - HALF_PI<float>();

    rightVector = {
        sinf( yawResult + roll ),
        0.0f,
        cosf( yawResult + roll )
    };
    rightVector = rightVector.normalize();

    nyaVec3f upVector = nyaVec3f::cross( rightVector, eyeDirection );
    upVector = upVector.normalize();

    // Update world position
    nyaVec3f nextWorldPosition = data.worldPosition + eyeDirection * speedX;
    data.worldPosition = lerp( data.worldPosition, nextWorldPosition, frameTime );

    if ( speedX > 0.0f ) {
        speedX -= frameTime * moveSpeed * 0.50f;
        speedX = max( speedX, 0.0f );
    } else if ( speedX < 0.0f ) {
        speedX += frameTime * moveSpeed * 0.50f;
        speedX = min( speedX, 0.0f );
    }

    nextWorldPosition = data.worldPosition + rightVector * speedY;
    data.worldPosition = lerp( data.worldPosition, nextWorldPosition, frameTime );

    if ( speedY > 0.0f ) {
        speedY -= frameTime * moveSpeed * 0.50f;
        speedY = max( speedY, 0.0f );
    } else if ( speedY < 0.0f ) {
        speedY += frameTime * moveSpeed * 0.50f;
        speedY = min( speedY, 0.0f );
    }

    auto nextUpWorldPosition = data.worldPosition + nyaVec3f( 0, 1, 0 ) * speedAltitude;
    data.worldPosition = lerp( data.worldPosition, nextUpWorldPosition, frameTime );

    if ( speedAltitude > 0.0f ) {
        speedAltitude -= frameTime * moveSpeed * 0.50f;
        speedAltitude = max( speedAltitude, 0.0f );
    } else if ( speedAltitude < 0.0f ) {
        speedAltitude += frameTime * moveSpeed * 0.50f;
        speedAltitude = min( speedAltitude, 0.0f );
    }

    const nyaVec3f lookAtVector = data.worldPosition + eyeDirection;

    // Save previous frame matrices (for temporal-based effects)
    data.previousViewProjectionMatrix = data.viewProjectionMatrix;
    data.previousViewMatrix = data.viewMatrix;

    // Rebuild matrices
    data.viewMatrix = MakeLookAtMat( data.worldPosition, lookAtVector, upVector );
    data.inverseViewMatrix = data.viewMatrix.inverse();
    data.depthViewProjectionMatrix = data.depthProjectionMatrix * data.viewMatrix;
    data.viewProjectionMatrix = data.projectionMatrix * data.viewMatrix;

    if ( data.flags.enableTAA ) {
        const uint32_t samplingIndex = ( data.cameraFrameNumber % 16 );

        static constexpr float TAAJitteringScale = 0.01f;
        nyaVec2f projectionJittering = ( nya::maths::Hammersley2D( samplingIndex, 16 ) - nyaVec2f( 0.5f ) ) * TAAJitteringScale;

        const nyaVec2f offset = projectionJittering * data.inverseViewportSize;

        // Apply jittering to projection matrix
        data.projectionMatrix = MakeTranslationMat( nyaVec3f( offset.x, -offset.y, 0.0f ), data.projectionMatrix );

        data.jitteringOffset = ( projectionJittering - data.previousJitteringOffset ) * 0.5f;
        data.previousJitteringOffset = projectionJittering;
    }
    
    data.inverseViewProjectionMatrix = data.viewProjectionMatrix.inverse();

    // Update frustum with the latest view projection matrix
    UpdateFrustumPlanes( data.depthViewProjectionMatrix, data.frustum );

    // Update camera frame number
    data.cameraFrameNumber++;
}

void FreeCamera::save( FileSystemObject* stream )
{
    stream->write( data.worldPosition );
    stream->write( moveSpeed );
    stream->write( moveDamping );
    stream->write( nearPlane );
    stream->write( fov );
    stream->write( data.viewportSize );
}

void FreeCamera::restore( FileSystemObject* stream )
{
    stream->read( data.worldPosition );
    stream->read( moveSpeed );
    stream->read( moveDamping );
    stream->read( nearPlane );
    stream->read( fov );
    stream->read( data.viewportSize );

    setProjectionMatrix( degrees( fov ), data.viewportSize.x, data.viewportSize.y, nearPlane );
}

void FreeCamera::setProjectionMatrix( const float fieldOfView, const float screenWidth, float screenHeight, const float zNear )
{
    aspectRatio = ( screenWidth / screenHeight );
    fov = radians( fieldOfView );

    nearPlane = zNear;

    data.viewportSize = nyaVec2f( screenWidth, screenHeight );
    data.inverseViewportSize = 1.0f / data.viewportSize;

    data.projectionMatrix = MakeInfReversedZProj( fov, aspectRatio, nearPlane );
    data.inverseProjectionMatrix = data.projectionMatrix.inverse();
    data.depthProjectionMatrix = MakeFovProj( fov, aspectRatio, 0.250f, 250.0f );
}

void FreeCamera::updateMouse( const float frameTime, const double mouseDeltaX, const double mouseDeltaY ) noexcept
{
    pitch -= static_cast< float >( mouseDeltaY ) * frameTime * 4;
    yaw += static_cast< float >( mouseDeltaX ) * frameTime * 4;
}

void FreeCamera::moveForward( const float dt )
{
    speedX += ( dt * moveSpeed );
}

void FreeCamera::moveBackward( const float dt )
{
    speedX -= ( dt * moveSpeed );
}

void FreeCamera::moveLeft( const float dt )
{
    speedY += ( dt * moveSpeed );
}

void FreeCamera::moveRight( const float dt )
{
    speedY -= ( dt * moveSpeed );
}

void FreeCamera::takeAltitude( const float dt )
{
    speedAltitude += ( dt * moveSpeed );
}

void FreeCamera::lowerAltitude( const float dt )
{
    speedAltitude -= ( dt * moveSpeed );
}

void FreeCamera::setOrientation( const float yaw, const float pitch, const float roll )
{
    this->yaw = yaw;
    this->pitch = pitch;
    this->roll = roll;
}

CameraData& FreeCamera::getData()
{
    return data;
}

void FreeCamera::setMSAASamplerCount( const uint32_t samplerCount )
{
    data.msaaSamplerCount = max( 1u, samplerCount );
}

void FreeCamera::setImageQuality( const float imageQuality )
{
    data.imageQuality = max( 0.1f, imageQuality );
}
