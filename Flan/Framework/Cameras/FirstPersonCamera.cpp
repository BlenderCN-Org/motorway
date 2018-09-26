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
#include "FirstPersonCamera.h"

#include <Core/Maths/MatrixTransformations.h>
#include <Core/Maths/Sampling.h>

FirstPersonCamera::FirstPersonCamera()
	: rightVector{ 0.0f }
	, eyeDirection{ 1.0f, 0.0f, 0.0f }
    , nextWorldPosition( 0 )
    , nextWorldRotation( 1, 0, 0, 0 )
    , cameraTargetRotation( 1, 0, 0, 0 )
	, yaw( 0.0f )
	, pitch( 0.0f )
	, roll( 0.0f )
    , aspectRatio( 1.0f )
    , width( 1280.0f )
    , height( 720.0f )
    , fov( glm::radians( 90.0f ) )
    , nearPlane( 0.1f )
    , cameraTime( 0.0f )
    , applyBobbing( false )
{
    data = {};
}

void FirstPersonCamera::Update( const float frameTime )
{
    auto targetRotationDelta = 1.0f - glm::pow( glm::dot( cameraTargetRotation, nextWorldRotation ), 2 );
    cameraTargetRotation = glm::slerp( cameraTargetRotation, nextWorldRotation, frameTime * 4 );

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

    const float yawresult = yaw - glm::half_pi<float>();

    rightVector = {
        sinf( yawresult + roll ),
        0.0f,
        cosf( yawresult + roll )
    };
    rightVector = glm::normalize( rightVector );

    glm::vec3 upVector = glm::cross( rightVector, eyeDirection );
    upVector = glm::normalize( upVector );

    // Update world position
    cameraTime += frameTime;
    auto offsetFactor = sin( cameraTime * 2.0f * glm::pi<float>() ) * 0.15f;
    auto bobbingOffset = ( applyBobbing ) ? rightVector * offsetFactor : glm::vec3( 0.0f );

    // Reset Bobbing flag
    applyBobbing = false;

    data.worldPosition = glm::mix( data.worldPosition, nextWorldPosition + bobbingOffset, frameTime * 16 );

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
    flan::core::UpdateFrustumPlanes( data.depthViewProjectionMatrix, frustum );

    // Update camera frame number
    data.cameraFrameNumber++;
}

void FirstPersonCamera::Serialize( FileSystemObject* stream )
{
    stream->write( data.worldPosition );
    stream->write( nearPlane );
    stream->write( fov );
    stream->write( width );
    stream->write( height );
}

void FirstPersonCamera::Deserialize( FileSystemObject* stream )
{
    stream->read( data.worldPosition );
    stream->read( nearPlane );
    stream->read( fov );
    stream->read( width );
    stream->read( height );

    auto fovDeg = glm::degrees( fov );
    setProjectionMatrix( fovDeg, width, height, nearPlane );
}

#if FLAN_DEVBUILD
#include <imgui/imgui.h>
void FirstPersonCamera::DrawInEditor( const float frameTime )
{
    auto fovDeg = glm::degrees( fov );

    ImGui::LabelText( "##FirstPersonCameraName", "First Person Camera" );
    ImGui::DragFloat3( "Position (world)", &data.worldPosition[0] );

    if ( ImGui::DragFloat( "Camera Near Plane", &nearPlane, 0.01f, 0.0001f, 10.0f ) ) {
        setProjectionMatrix( fovDeg, width, height, nearPlane );
    }

    if ( ImGui::DragFloat( "Field Of View (deg)", &fovDeg, 0.01f, 0.001f, 180.0f ) ) {
        setProjectionMatrix( fovDeg, width, height, nearPlane );
    }
}
#endif

void FirstPersonCamera::setProjectionMatrix( const float fieldOfView, const float screenWidth, float screenHeight, const float zNear )
{
    aspectRatio = ( screenWidth / screenHeight );

    width = screenWidth;
    height = screenHeight;

    fov = glm::radians( fieldOfView );

    nearPlane = zNear;

    data.projectionMatrix = flan::core::MakeInfReversedZProj( fov, aspectRatio, nearPlane );
    data.inverseProjectionMatrix = glm::transpose( glm::inverse( data.projectionMatrix ) );
    data.depthProjectionMatrix = glm::perspectiveFovLH( fov, width, height, 1.0f, 125.0f );
}

void FirstPersonCamera::setNextWorldPosition( const float frameTime, const glm::vec3& worldPosition )
{
    nextWorldPosition = worldPosition;
}

void FirstPersonCamera::setNextWorldRotation( const float frameTime, const glm::quat& worldRotation )
{
    nextWorldRotation = worldRotation;
}

void FirstPersonCamera::onMouseUpdate( const float frameTime, const double mouseDeltaX, const double mouseDeltaY ) noexcept
{
    pitch -= static_cast< float >( mouseDeltaY ) * frameTime * 4; // *0.01f;
    yaw += static_cast< float >( mouseDeltaX ) * frameTime * 4; // *0.1f;
}

const glm::vec3& FirstPersonCamera::getRightVector() const
{
    return rightVector;
}

const glm::vec3& FirstPersonCamera::getEyeDirection() const
{
    return eyeDirection;
}

void FirstPersonCamera::setRunningState()
{
    applyBobbing = true;
}
