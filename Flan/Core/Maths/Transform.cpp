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
#include "Transform.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <Framework/TransactionHandler/TransactionHandler.h>

#include <Framework/TransactionHandler/TranslateCommand.h>
#include <Framework/TransactionHandler/RotateCommand.h>
#include <Framework/TransactionHandler/ScaleCommand.h>

Transform::Transform( const glm::vec3& worldTranslation, const glm::vec3& worldScale, const glm::quat& worldRotation )
    : isDirty( true )
    , isManipulating( false )
    , worldTranslation( worldTranslation )
    , worldScale( worldScale )
    , worldRotation( worldRotation )
    , localTranslation( worldTranslation )
    , localScale( worldScale )
    , localRotation( worldRotation )
    , localModelMatrix( 1.0f )
    , worldModelMatrix( 1.0f )
{
   rebuildModelMatrix();
}

bool Transform::rebuildModelMatrix()
{
    bool hasChanged = isDirty;

    // Rebuild model matrix if anything has changed
    if ( isDirty ) {
        glm::mat4 translationMatrix = glm::translate( glm::mat4( 1.0 ), localTranslation );
        glm::mat4 rotationMatrix = glm::mat4_cast( localRotation );
        glm::mat4 scaleMatrix = glm::scale( glm::mat4( 1.0 ), localScale );

        localModelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

        propagateParentModelMatrix( glm::mat4x4( 1.0f ) );

        isDirty = false;
    }

    return hasChanged;
}

void Transform::serialize( FileSystemObject* stream )
{
    stream->write( (uint8_t*)&localModelMatrix[0][0], sizeof( glm::mat4 ) );
}

void Transform::deserialize( FileSystemObject* stream )
{
    stream->read( ( uint8_t* )&localModelMatrix[0][0], sizeof( glm::mat4 ) );

    glm::vec3 skewDecomposed;
    glm::vec4 perspectiveDecomposed;
    glm::decompose( localModelMatrix, localScale, localRotation, localTranslation, skewDecomposed, perspectiveDecomposed );

    isDirty = true;

    rebuildModelMatrix();
}

#if FLAN_DEVBUILD
#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
void Transform::drawInEditor( const float frameTime, TransactionHandler* transactionHandler )
{
    if ( ImGui::TreeNode( "Transform" ) ) {
        FLAN_IMPORT_VAR_PTR( dev_GuizmoViewMatrix, float* )
        FLAN_IMPORT_VAR_PTR( dev_GuizmoProjMatrix, float* )

        static ImGuizmo::OPERATION mCurrentGizmoOperation( ImGuizmo::ROTATE );
        static int activeManipulationMode = 0;
        static bool useSnap = false;
        static float snap[3] = { 1.f, 1.f, 1.f };
        static ImGuizmo::MODE mCurrentGizmoMode( ImGuizmo::LOCAL );

        glm::mat4x4* modelMatrix = ( mCurrentGizmoMode == ImGuizmo::LOCAL ) ? &localModelMatrix : &worldModelMatrix;

        if ( mCurrentGizmoOperation != ImGuizmo::SCALE ) {
            if ( ImGui::RadioButton( "Local", mCurrentGizmoMode == ImGuizmo::LOCAL ) ) {
                mCurrentGizmoMode = ImGuizmo::LOCAL;
                modelMatrix = &localModelMatrix;
            }

            ImGui::SameLine();

            if ( ImGui::RadioButton( "World", mCurrentGizmoMode == ImGuizmo::WORLD ) ) {
                mCurrentGizmoMode = ImGuizmo::WORLD;
                modelMatrix = &worldModelMatrix;
            }
        }

        ImGui::Checkbox( "", &useSnap );
        ImGui::SameLine();

        switch ( mCurrentGizmoOperation ) {
        case ImGuizmo::TRANSLATE:
            ImGui::InputFloat3( "Snap", snap );
            break;
        case ImGuizmo::ROTATE:
            ImGui::InputFloat( "Angle Snap", snap );
            break;
        case ImGuizmo::SCALE:
            ImGui::InputFloat( "Scale Snap", snap );
            break;
        }

        ImGui::RadioButton( "Translate", &activeManipulationMode, 0 );
        ImGui::SameLine();
        ImGui::RadioButton( "Rotate", &activeManipulationMode, 1 );
        ImGui::SameLine();
        ImGui::RadioButton( "Scale", &activeManipulationMode, 2 );

        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect( 0, 0, io.DisplaySize.x, io.DisplaySize.y );
        ImGuizmo::Manipulate( *dev_GuizmoViewMatrix, *dev_GuizmoProjMatrix, static_cast< ImGuizmo::OPERATION >( activeManipulationMode ), mCurrentGizmoMode, ( float* )modelMatrix, NULL, useSnap ? &snap[activeManipulationMode] : NULL );

        glm::vec3 scaleDecomposed;
        glm::quat rotationDecomposed;
        glm::vec3 skewDecomposed;
        glm::vec3 translationDecomposed;
        glm::vec4 perspectiveDecomposed;
        glm::decompose( *modelMatrix, scaleDecomposed, rotationDecomposed, translationDecomposed, skewDecomposed, perspectiveDecomposed );

        ImGui::InputFloat3( "Translation", ( float* )&translationDecomposed, 3 );

        auto euler = glm::eulerAngles( rotationDecomposed );

        glm::quat nextRotation = localRotation;

        // TODO FIXME Why the fuck rotation sperg out when it's enabled (loss of precision when converting euler angles to quaternion?)
        if ( ImGui::InputFloat3( "Rotation", ( float* )&euler, 3 ) ) {
            nextRotation = glm::toQuat( glm::eulerAngleXYZ( euler.x, euler.y, euler.z ) );
        } else {
            nextRotation = rotationDecomposed;
        }

        ImGui::InputFloat3( "Scale", ( float* )&scaleDecomposed, 3 );

        isManipulating = ImGuizmo::IsUsing();

        if ( !isManipulating ) {
            if ( translationDecomposed != localTranslation ) {
                if ( mCurrentGizmoMode == ImGuizmo::LOCAL )
                    transactionHandler->commit( new LocalTranslateCommand( this, translationDecomposed ) );
                else
                    transactionHandler->commit( new WorldTranslateCommand( this, translationDecomposed ) );
            }

            if ( scaleDecomposed != localScale ) {
                if ( mCurrentGizmoMode == ImGuizmo::LOCAL )
                    transactionHandler->commit( new LocalScaleCommand( this, scaleDecomposed ) );
                else
                    transactionHandler->commit( new WorldScaleCommand( this, scaleDecomposed ) );
            }

            if ( nextRotation != localRotation ) {
                if ( mCurrentGizmoMode == ImGuizmo::LOCAL )
                    transactionHandler->commit( new LocalRotateCommand( this, nextRotation ) );
                else
                    transactionHandler->commit( new WorldRotateCommand( this, nextRotation ) );
            }
        }
    
        ImGui::TreePop();
    }
}
#endif

void Transform::setLocalTranslation( const glm::vec3& newTranslation )
{
    localTranslation = newTranslation;
    isDirty = true;
}

void Transform::setLocalRotation( const glm::quat& newRotation )
{
    localRotation = newRotation;
    isDirty = true;
}

void Transform::setLocalScale( const glm::vec3& newScale )
{
    localScale = newScale;
    isDirty = true;
}

void Transform::setLocalModelMatrix( const glm::mat4& modelMat )
{
    localModelMatrix = modelMat;

    glm::vec3 skewDecomposed;
    glm::vec4 perspectiveDecomposed;
    glm::decompose( localModelMatrix, localScale, localRotation, localTranslation, skewDecomposed, perspectiveDecomposed );
}

void Transform::setWorldTranslation( const glm::vec3& newTranslation )
{
    worldTranslation = newTranslation;
    isDirty = true;
}

void Transform::setWorldRotation( const glm::quat& newRotation )
{
    worldRotation = newRotation;
    isDirty = true;
}

void Transform::setWorldScale( const glm::vec3& newScale )
{
    worldScale = newScale;
    isDirty = true;
}

void Transform::setWorldModelMatrix( const glm::mat4& modelMat )
{
    worldModelMatrix = modelMat;

    glm::vec3 skewDecomposed;
    glm::vec4 perspectiveDecomposed;
    glm::decompose( worldModelMatrix, worldScale, worldRotation, worldTranslation, skewDecomposed, perspectiveDecomposed );
}

void Transform::propagateParentModelMatrix( const glm::mat4& parentModelMatrix )
{
    worldModelMatrix = localModelMatrix * parentModelMatrix;

    glm::vec3 skewDecomposed;
    glm::vec4 perspectiveDecomposed;
    glm::decompose( worldModelMatrix, worldScale, worldRotation, worldTranslation, skewDecomposed, perspectiveDecomposed );
}

glm::mat4* Transform::getWorldModelMatrix()
{
    return &worldModelMatrix;
}

const glm::mat4& Transform::getWorldModelMatrix() const
{
    return worldModelMatrix;
}

glm::vec3 Transform::getWorldScale() const
{
    return worldScale;
}

glm::vec3 Transform::getWorldTranslation() const
{
    return worldTranslation;
}

glm::quat Transform::getWorldRotation() const
{
    return worldRotation;
}

float Transform::getWorldBiggestScale() const
{
    auto biggestScale = worldScale.x;

    if ( biggestScale < worldScale.y ) {
        biggestScale = worldScale.y;
    }

    if ( biggestScale < worldScale.z ) {
        biggestScale = worldScale.z;
    }

    return biggestScale;
}

glm::mat4* Transform::getLocalModelMatrix()
{
    return &localModelMatrix;
}

const glm::mat4& Transform::getLocalModelMatrix() const
{
    return localModelMatrix;
}

glm::vec3 Transform::getLocalScale() const
{
    return localScale;
}

glm::vec3 Transform::getLocalTranslation() const
{
    return localTranslation;
}

glm::quat Transform::getLocalRotation() const
{
    return localRotation;
}

float Transform::getLocalBiggestScale() const
{
    auto biggestScale = localScale.x;

    if ( biggestScale < localScale.y ) {
        biggestScale = localScale.y;
    }

    if ( biggestScale < localScale.z ) {
        biggestScale = localScale.z;
    }

    return biggestScale;
}
