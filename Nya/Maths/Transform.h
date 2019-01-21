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

#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/quaternion.hpp>

class FileSystemObject;

class Transform
{
public:
                        Transform( const glm::vec3& worldTranslation = glm::vec3(), const glm::vec3& worldScale = glm::vec3( 1.0f ), const glm::quat& worldRotation = glm::quat( 1.0f, 0.0f, 0.0f, 0.0f ) );
                        Transform( Transform& transform ) = default;
                        Transform& operator = ( Transform& transform ) = default;
                        ~Transform() = default;

    bool                rebuildModelMatrix();

    void                setLocalTranslation( const glm::vec3& newTranslation );
    void                setLocalRotation( const glm::quat& newRotation );
    void                setLocalScale( const glm::vec3& newScale );

    void                setLocalModelMatrix( const glm::mat4& modelMat );

    void                setWorldTranslation( const glm::vec3& newTranslation );
    void                setWorldRotation( const glm::quat& newRotation );
    void                setWorldScale( const glm::vec3& newScale );

    void                setWorldModelMatrix( const glm::mat4& modelMat );

    void                translate( const glm::vec3& translation );

    void                propagateParentModelMatrix( const glm::mat4& parentModelMatrix );

    glm::mat4*          getWorldModelMatrix();
    const glm::mat4&    getWorldModelMatrix() const;

    glm::vec3           getWorldScale() const;
    glm::vec3           getWorldTranslation() const;
    glm::quat           getWorldRotation() const;
    float               getWorldBiggestScale() const;

    glm::mat4*          getLocalModelMatrix();
    const glm::mat4&    getLocalModelMatrix() const;

    glm::vec3           getLocalScale() const;
    glm::vec3           getLocalTranslation() const;
    glm::quat           getLocalRotation() const;
    float               getLocalBiggestScale() const;

    void                serialize( FileSystemObject* stream );
    void                deserialize( FileSystemObject* stream );

private:
    bool                isDirty;

    glm::vec3           worldTranslation;
    glm::vec3           worldScale;
    glm::quat           worldRotation;

    glm::vec3           localTranslation;
    glm::vec3           localScale;
    glm::quat           localRotation;

    glm::mat4           localModelMatrix;
    glm::mat4           worldModelMatrix;
};
