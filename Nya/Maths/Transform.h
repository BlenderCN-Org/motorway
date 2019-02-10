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
#pragma once

#include "Matrix.h"
#include "Vector.h"
#include "Quaternion.h"

class FileSystemObject;

class Transform
{
public:
                        Transform( const nyaVec3f& worldTranslation = nyaVec3f::Zero, const nyaVec3f& worldScale = nyaVec3f( 1.0f, 1.0f, 1.0f ), const nyaQuatf& worldRotation = nyaQuatf::Identity );
                        Transform( Transform& transform ) = default;
                        Transform& operator = ( Transform& transform ) = default;
                        ~Transform() = default;

    bool                rebuildModelMatrix();

    void                setLocalTranslation( const nyaVec3f& newTranslation );
    void                setLocalRotation( const nyaQuatf& newRotation );
    void                setLocalScale( const nyaVec3f& newScale );

    void                setLocalModelMatrix( const nyaMat4x4f& modelMat );

    void                setWorldTranslation( const nyaVec3f& newTranslation );
    void                setWorldRotation( const nyaQuatf& newRotation );
    void                setWorldScale( const nyaVec3f& newScale );

    void                setWorldModelMatrix( const nyaMat4x4f& modelMat );

    void                translate( const nyaVec3f& translation );

    void                propagateParentModelMatrix( const nyaMat4x4f& parentModelMatrix );

    nyaMat4x4f*         getWorldModelMatrix();
    const nyaMat4x4f&   getWorldModelMatrix() const;

    const nyaVec3f&     getWorldScale() const;
    const nyaVec3f&     getWorldTranslation() const;
    const nyaQuatf&     getWorldRotation() const;
    float               getWorldBiggestScale() const;

    nyaMat4x4f*         getLocalModelMatrix();
    const nyaMat4x4f&   getLocalModelMatrix() const;

    const nyaVec3f&     getLocalScale() const;
    const nyaVec3f&     getLocalTranslation() const;
    const nyaQuatf&     getLocalRotation() const;
    float               getLocalBiggestScale() const;

    void                serialize( FileSystemObject* stream );
    void                deserialize( FileSystemObject* stream );

    bool                needRebuild() const;

private:
    bool                isDirty;

    nyaVec3f            worldTranslation;
    nyaVec3f            worldScale;
    nyaQuatf            worldRotation;

    nyaVec3f            localTranslation;
    nyaVec3f            localScale;
    nyaQuatf            localRotation;

    nyaMat4x4f          localModelMatrix;
    nyaMat4x4f          worldModelMatrix;
};
