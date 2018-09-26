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
#include "LookAtCamera.h"

#include <Core/Maths/MatrixTransformations.h>
#include <Core/Maths/Sampling.h>

LookAtCamera::LookAtCamera()
    : isCameraMoving( false )
    , cameraHeight( 2.50f )
    , cameraRadius( 4.0f )
    , cameraTheta( 0 )
    , cameraPhi( 0 )
    , aspectRatio( 1.0f )
    , width( 1280.0f )
    , height( 720.0f )
    , fov( glm::radians( 90.0f ) )
    , nearPlane( 0.1f )
    , nextWorldRotation( 1, 0, 0, 0 )
    , nextWorldPosition( 0, 0, 0 )
    , cameraTargetRotation( 1, 0, 0, 0 )
    , cameraTargetTranslation( 0, 0, 0 )
    , cameraActiveRadius( cameraRadius )
{
    data = {};
}

void LookAtCamera::Update( const float frameTime )
{
    // If the camera button is released, smoothly return to the origin
    if ( !isCameraMoving ) {
        cameraTheta = glm::mix( cameraTheta, 0.0f, frameTime * 4.0f );
        cameraPhi = glm::mix( cameraPhi, 0.0f, frameTime * 4.0f );
    }

    // Compute distance between current and previous rotation quaternion
    auto targetRotationDelta = 1.0f - glm::pow( glm::dot( nextWorldRotation, cameraTargetRotation ), 2 );

    auto recoveryFactor = ( ( targetRotationDelta ) < 0.1f ) ? 4 : 8;
    cameraTargetRotation = glm::slerp( cameraTargetRotation, nextWorldRotation, frameTime * recoveryFactor );

    // Camera to nextWorldPosition distance based on object velocity
    // This is (obviously) super approximate but doesn't require any coupling with Physics subsystem
    auto targetTranslationAlpha = glm::distance( nextWorldPosition, cameraTargetTranslation ) * 2;
    cameraActiveRadius = cameraRadius + cameraRadius * targetTranslationAlpha;

    cameraTargetTranslation = nextWorldPosition;

    // Combine Pitch and Yaw as a single quaternion
    auto cameraMouseRotationPhi = glm::angleAxis( cameraPhi, glm::vec3( 1, 0, 0 ) );
    auto cameraMouseRotationTheta = glm::angleAxis( cameraTheta, glm::vec3( 0, 1, 0 ) );
    auto cameraMouseRotation = glm::normalize( cameraMouseRotationTheta * cameraMouseRotationPhi );

    // Compute camera position (in world space)
    auto cameraTranslation = cameraMouseRotation * cameraTargetRotation * glm::vec4( 0, cameraHeight, cameraActiveRadius, 1 );
    data.worldPosition = cameraTargetTranslation + glm::vec3( cameraTranslation );

    // Save previous frame matrices (for temporal-based effects)
    data.previousViewProjectionMatrix = data.viewProjectionMatrix;
    data.previousViewMatrix = data.viewMatrix;

    data.viewProjectionMatrix = glm::transpose( data.projectionMatrix * data.viewMatrix );

    data.viewMatrix = glm::lookAtLH( data.worldPosition, nextWorldPosition, glm::vec3( 0, 1, 0 ) );
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

void LookAtCamera::Serialize( FileSystemObject* stream )
{
    stream->write( data.worldPosition );
    stream->write( cameraHeight );
    stream->write( cameraRadius );
    stream->write( nearPlane );
    stream->write( fov );
}

void LookAtCamera::Deserialize(  FileSystemObject* stream )
{
    stream->read( data.worldPosition );
    stream->read( cameraHeight );
    stream->read( cameraRadius );
    stream->read( nearPlane );
    stream->read( fov );
}

#if FLAN_DEVBUILD
#include <imgui/imgui.h>
void LookAtCamera::DrawInEditor( const float frameTime )
{
    auto fovDeg = glm::degrees( fov );

    ImGui::LabelText( "##LookAtCameraName", "Look At Camera" );
    ImGui::DragFloat3( "Position (world)", &data.worldPosition[0] );

    ImGui::DragFloat( "Camera To Target Height", &cameraHeight, 0.01f, 0.0001f, 100.0f );
    ImGui::DragFloat( "Camera To Target Radius", &cameraRadius, 0.01f, 0.0001f, 100.0f );

    if ( ImGui::DragFloat( "Camera Near Plane", &nearPlane, 0.01f, 0.0001f, 10.0f ) ) {
        setProjectionMatrix( fovDeg, width, height, nearPlane );
    }

    if ( ImGui::DragFloat( "Field Of View (deg)", &fovDeg, 0.01f, 0.001f, 180.0f ) ) {
        setProjectionMatrix( fovDeg, width, height, nearPlane );
    }
}
#endif

void LookAtCamera::setProjectionMatrix( const float fieldOfView, const float screenWidth, float screenHeight, const float zNear )
{
    aspectRatio = ( screenWidth / screenHeight );

    width = screenWidth;
    height = screenHeight;

    fov = glm::radians( fieldOfView );

    nearPlane = zNear;

    data.projectionMatrix = flan::core::MakeInfReversedZProj( glm::radians( fieldOfView ), aspectRatio, zNear );
    data.inverseProjectionMatrix = glm::transpose( glm::inverse( data.projectionMatrix ) );
    data.depthProjectionMatrix = glm::perspectiveFovLH( fov, width, height, 1.0f, 125.0f );
}

void LookAtCamera::onMouseUpdate( const float frameTime, const double mouseDeltaX, const double mouseDeltaY ) noexcept
{
    // Update the camera only when the camera button is pressed
    if ( !isCameraMoving ) {
        return;
    }

    cameraTheta -= static_cast<float>( mouseDeltaX ) * frameTime * 8.0f;
    cameraPhi += static_cast<float>( mouseDeltaY ) * frameTime * 8.0f;

    // Clamp to avoid 360 rotation
    cameraTheta = glm::clamp( cameraTheta, -glm::pi<float>(), glm::pi<float>() );
    cameraPhi = glm::clamp( cameraPhi, -( glm::quarter_pi<float>() / 2.0f ), glm::quarter_pi<float>() / 2.0f );
}

void LookAtCamera::setNextWorldPosition( const float frameTime, const glm::vec3& worldPosition )
{
    nextWorldPosition = worldPosition;
}

void LookAtCamera::setNextWorldRotation( const float frameTime, const glm::quat& worldRotation )
{
    nextWorldRotation = worldRotation;
}

void LookAtCamera::isMovingCamera( const bool state )
{
    isCameraMoving = state;
}
