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

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm/gtx/euler_angles.hpp>
#include <glm/glm/gtx/quaternion.hpp>
#include <glm/glm/gtx/matrix_decompose.hpp>

#include <FileSystem/FileSystemObject.h>

Transform::Transform( const glm::vec3& worldTranslation, const glm::vec3& worldScale, const glm::quat& worldRotation )
    : isDirty( true )
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

void Transform::translate( const glm::vec3& translation )
{
    worldTranslation += translation;
    isDirty = true;
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

bool Transform::needRebuild() const
{
    return isDirty;
}
