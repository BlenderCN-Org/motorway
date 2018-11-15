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

#include <Core/Maths/MatrixTransformations.h>
#include <Core/Maths/Sampling.h>
//#include <imgui/imgui.h>

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
    , width( 1280.0f )
    , height( 720.0f )
    , fov( glm::radians( 90.0f ) )
    , nearPlane( 0.1f )
{
    data = {};
}

void FreeCamera::Update( const float frameTime )
{
    if ( yaw < -glm::pi<float>() ) {
        yaw += glm::two_pi<float>();
    } else if ( yaw > glm::pi<float>() ) {
        yaw -= glm::two_pi<float>();
    }

    pitch = glm::clamp( pitch, -glm::half_pi<float>(), glm::half_pi<float>() );

    eyeDirection = {
        cosf( pitch ) * sinf( yaw ),
        sinf( pitch ),
        cosf( pitch ) * cosf( yaw )
    };
    eyeDirection = glm::normalize( eyeDirection );

    const float yawResult = yaw - glm::half_pi<float>();

    rightVector = {
        sinf( yawResult + roll ),
        0.0f,
        cosf( yawResult + roll )
    };
    rightVector = glm::normalize( rightVector );

    glm::vec3 upVector = glm::cross( rightVector, eyeDirection );
    upVector = glm::normalize( upVector );

    // Update world position
    auto nextWorldPosition = data.worldPosition + eyeDirection * speedX;
    data.worldPosition = glm::mix( data.worldPosition, nextWorldPosition, frameTime );

    if ( speedX > 0.0f ) {
        speedX -= frameTime * moveSpeed * 0.50f;
        speedX = glm::max( speedX, 0.0f );
    } else if ( speedX < 0.0f ) {
        speedX += frameTime * moveSpeed * 0.50f;
        speedX = glm::min( speedX, 0.0f );
    }

    nextWorldPosition = data.worldPosition + rightVector * speedY;
    data.worldPosition = glm::mix( data.worldPosition, nextWorldPosition, frameTime );

    if ( speedY > 0.0f ) {
        speedY -= frameTime * moveSpeed * 0.50f;
        speedY = glm::max( speedY, 0.0f );
    } else if ( speedY < 0.0f ) {
        speedY += frameTime * moveSpeed * 0.50f;
        speedY = glm::min( speedY, 0.0f );
    }

    auto nextUpWorldPosition = data.worldPosition + glm::vec3( 0, 1, 0 ) * speedAltitude;
    data.worldPosition = glm::mix( data.worldPosition, nextUpWorldPosition, frameTime );

    if ( speedAltitude > 0.0f ) {
        speedAltitude -= frameTime * moveSpeed * 0.50f;
        speedAltitude = glm::max( speedAltitude, 0.0f );
    } else if ( speedAltitude < 0.0f ) {
        speedAltitude += frameTime * moveSpeed * 0.50f;
        speedAltitude = glm::min( speedAltitude, 0.0f );
    }

    // Build matrices
    const glm::vec3 lookAtVector = data.worldPosition + eyeDirection;

    // Save previous frame matrices (for temporal-based effects)
    data.previousViewProjectionMatrix = data.viewProjectionMatrix;
    data.previousViewMatrix = data.viewMatrix;

    data.viewProjectionMatrix = glm::transpose( data.projectionMatrix * data.viewMatrix );

    data.viewMatrix = glm::lookAtLH( data.worldPosition, lookAtVector, upVector );
    data.depthViewProjectionMatrix = data.depthProjectionMatrix * data.viewMatrix;

    FLAN_IMPORT_VAR_PTR( EnableTemporalAA, bool )
    if ( *EnableTemporalAA ) {
        const uint32_t samplingIndex = ( data.cameraFrameNumber % 16 );

        glm::vec2 projectionJittering = ( flan::core::Hammersley2D( samplingIndex, 16 ) - glm::vec2( 0.5f ) ) * TAAJitteringScale;

        const float offsetX = projectionJittering.x * ( 1.0f / width );
        const float offsetY = projectionJittering.y * ( 1.0f / height );

        // Apply jittering to projection matrix
        data.projectionMatrix = glm::translate( data.projectionMatrix, glm::vec3( offsetX, -offsetY, 0 ) );

        data.jitteringOffset = ( projectionJittering - data.previousJitteringOffset ) * 0.5f;
        data.previousJitteringOffset = projectionJittering;
    }
        
    data.viewProjectionMatrix = glm::transpose( data.projectionMatrix * data.viewMatrix );

    data.inverseViewMatrix = glm::inverse( data.viewMatrix );
    data.viewMatrix = glm::transpose( data.viewMatrix );

    data.inverseViewProjectionMatrix = glm::inverse( data.viewProjectionMatrix );

    // Update frustum with the latest view projection matrix
    flan::core::UpdateFrustumPlanes( data.depthViewProjectionMatrix, data.frustum );

    // Update camera frame number
    data.cameraFrameNumber++;
}

void FreeCamera::Serialize( FileSystemObject* stream )
{
    stream->write( data.worldPosition );
    stream->write( moveSpeed );
    stream->write( moveDamping );
    stream->write( nearPlane );
    stream->write( fov );
    stream->write( width );
    stream->write( height );
}

void FreeCamera::Deserialize( FileSystemObject* stream )
{
    stream->read( data.worldPosition );
    stream->read( moveSpeed );
    stream->read( moveDamping );
    stream->read( nearPlane );
    stream->read( fov );
    stream->read( width );
    stream->read( height );

    auto fovDeg = glm::degrees( fov );
    SetProjectionMatrix( fovDeg, width, height, nearPlane );
}

#if FLAN_DEVBUILD
#include <imgui/imgui.h>
void FreeCamera::DrawInEditor( const float frameTime )
{
    auto fovDeg = glm::degrees( fov );

    ImGui::LabelText( "##FreeCameraName", "Free Camera" );
    ImGui::DragFloat3( "Position (world)", &data.worldPosition[0] );
    ImGui::DragFloat( "Move Speed", &moveSpeed, 0.01f, 0.0f, 64.0f );
    ImGui::DragFloat( "Move Damping", &moveDamping, 0.01f, 0.0f, 64.0f );

    if ( ImGui::DragFloat( "Camera Near Plane", &nearPlane, 0.01f, 0.0001f, 10.0f ) ) {
        SetProjectionMatrix( fovDeg, width, height, nearPlane );
    }

    if ( ImGui::DragFloat( "Field Of View (deg)", &fovDeg, 0.01f, 0.001f, 180.0f ) ) {
        SetProjectionMatrix( fovDeg, width, height, nearPlane );
    }
}
#endif

void FreeCamera::SetProjectionMatrix( const float fieldOfView, const float screenWidth, float screenHeight, const float zNear )
{
    aspectRatio = ( screenWidth / screenHeight );

    width = screenWidth;
    height = screenHeight;

    fov = glm::radians( fieldOfView );

    nearPlane = zNear;

    data.projectionMatrix = flan::core::MakeInfReversedZProj( fov, aspectRatio, nearPlane );
    data.inverseProjectionMatrix = glm::transpose( glm::inverse( data.projectionMatrix ) );
    data.depthProjectionMatrix = glm::perspectiveFovLH( fov, width, height, 1.0f, 512.0f );
}

void FreeCamera::OnMouseUpdate( const float frameTime, const double mouseDeltaX, const double mouseDeltaY ) noexcept
{
    pitch -= static_cast< float >( mouseDeltaY ) * frameTime * 4; // *0.01f;
    yaw += static_cast< float >( mouseDeltaX ) * frameTime * 4; // *0.1f;
}

void FreeCamera::MoveForward( const float dt )
{
    speedX += ( dt * moveSpeed );
}

void FreeCamera::MoveBackward( const float dt )
{
    speedX -= ( dt * moveSpeed );
}

void FreeCamera::MoveLeft( const float dt )
{
    speedY += ( dt * moveSpeed );
}

void FreeCamera::MoveRight( const float dt )
{
    speedY -= ( dt * moveSpeed );
}

void FreeCamera::TakeAltitude( const float dt )
{
    speedAltitude += ( dt * moveSpeed );
}

void FreeCamera::LowerAltitude( const float dt )
{
    speedAltitude -= ( dt * moveSpeed );
}
