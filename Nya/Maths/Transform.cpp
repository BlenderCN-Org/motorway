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

#include <FileSystem/FileSystemObject.h>

#include "MatrixTransformations.h"

using namespace nya::maths;

Transform::Transform( const nyaVec3f& worldTranslation, const nyaVec3f& worldScale, const nyaQuatf& worldRotation )
    : isDirty( true )
    , worldTranslation( worldTranslation )
    , worldScale( worldScale )
    , worldRotation( worldRotation )
    , localTranslation( worldTranslation )
    , localScale( worldScale )
    , localRotation( worldRotation )
    , localModelMatrix( nyaMat4x4f::Identity )
    , worldModelMatrix( nyaMat4x4f::Identity )
{
    rebuildModelMatrix();
}

bool Transform::rebuildModelMatrix()
{
    bool hasChanged = isDirty;

    // Rebuild model matrix if anything has changed
    if ( isDirty ) {
        nyaMat4x4f translationMatrix = MakeTranslationMat( localTranslation );
        nyaMat4x4f rotationMatrix = localRotation.toMat4x4();
        nyaMat4x4f scaleMatrix = MakeScaleMat( localScale );

        localModelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

        propagateParentModelMatrix( nyaMat4x4f::Identity );

        isDirty = false;
    }

    return hasChanged;
}

void Transform::serialize( FileSystemObject* stream )
{
    stream->write( (uint8_t*)&localModelMatrix[0][0], sizeof( nyaMat4x4f ) );
}

void Transform::deserialize( FileSystemObject* stream )
{
    stream->read( ( uint8_t* )&localModelMatrix[0][0], sizeof( nyaMat4x4f ) );

    localTranslation = ExtractTranslation( localModelMatrix );
    localScale = ExtractScale( localModelMatrix );
    localRotation = nyaQuatf( ExtractRotation( localModelMatrix, localScale ) );

    isDirty = true;

    rebuildModelMatrix();
}

void Transform::setLocalTranslation( const nyaVec3f& newTranslation )
{
    localTranslation = newTranslation;
    isDirty = true;
}

void Transform::setLocalRotation( const nyaQuatf& newRotation )
{
    localRotation = newRotation;
    isDirty = true;
}

void Transform::setLocalScale( const nyaVec3f& newScale )
{
    localScale = newScale;
    isDirty = true;
}

void Transform::setLocalModelMatrix( const nyaMat4x4f& modelMat )
{
    localModelMatrix = modelMat;

    localTranslation = ExtractTranslation( localModelMatrix );
    localScale = ExtractScale( localModelMatrix );
    localRotation = nyaQuatf( ExtractRotation( localModelMatrix, localScale ) );
}

void Transform::setWorldTranslation( const nyaVec3f& newTranslation )
{
    worldTranslation = newTranslation;
    isDirty = true;
}

void Transform::setWorldRotation( const nyaQuatf& newRotation )
{
    worldRotation = newRotation;
    isDirty = true;
}

void Transform::setWorldScale( const nyaVec3f& newScale )
{
    worldScale = newScale;
    isDirty = true;
}

void Transform::setWorldModelMatrix( const nyaMat4x4f& modelMat )
{
    worldModelMatrix = modelMat;
    
    worldTranslation = ExtractTranslation( worldModelMatrix );
    worldScale = ExtractScale( worldModelMatrix );
    worldRotation = nyaQuatf( ExtractRotation( worldModelMatrix, worldScale ) );
}

void Transform::translate( const nyaVec3f& translation )
{
    localTranslation += translation;
    isDirty = true;
}

void Transform::propagateParentModelMatrix( const nyaMat4x4f& parentModelMatrix )
{
    worldModelMatrix = ( localModelMatrix * parentModelMatrix ).transpose();

    worldTranslation = ExtractTranslation( worldModelMatrix );
    worldScale = ExtractScale( worldModelMatrix );
    worldRotation = nyaQuatf( ExtractRotation( worldModelMatrix, worldScale ) );
}

nyaMat4x4f* Transform::getWorldModelMatrix()
{
    return &worldModelMatrix;
}

const nyaMat4x4f& Transform::getWorldModelMatrix() const
{
    return worldModelMatrix;
}

const nyaVec3f& Transform::getWorldScale() const
{
    return worldScale;
}

const nyaVec3f& Transform::getWorldTranslation() const
{
    return worldTranslation;
}

const nyaQuatf& Transform::getWorldRotation() const
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

nyaMat4x4f* Transform::getLocalModelMatrix()
{
    return &localModelMatrix;
}

const nyaMat4x4f& Transform::getLocalModelMatrix() const
{
    return localModelMatrix;
}

const nyaVec3f& Transform::getLocalScale() const
{
    return localScale;
}

const nyaVec3f& Transform::getLocalTranslation() const
{
    return localTranslation;
}

const nyaQuatf& Transform::getLocalRotation() const
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
