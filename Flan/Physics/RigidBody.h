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

#include <bullet3/src/btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class RigidBody
{
public:
                    RigidBody( 
                        const float bodyMass, 
                        const glm::vec3& positionWorldSpace = glm::vec3( 0.0f, 0.0f, 0.0f ),
                        const glm::quat& rotationWorldSpace = glm::quat( 1.0f, 0.0f, 0.0f, 0.0f ) );
                    RigidBody( RigidBody& rigidBody );
                    RigidBody& operator = ( RigidBody& rigidBody );
                    ~RigidBody();

    glm::vec3       getWorldPosition() const ;
    glm::quat       getWorldRotation() const;

    void            setMass( const float newMass );
    float           getMass() const;

    void            recomputeInertia();

    void            setWorldTransform( const glm::vec3& worldPosition, const glm::quat& worldRotation );

    void            setRestitution( const float restitution );
    void            setRollingFriction( const float rollingFriction );
    void            setFriction( const float friction );

    void            setDamping( const float linearDamping, const float angularDamping );
    void            setGravity( const glm::vec3& gravity );
    void            setActivationState( const bool activationState );
    void            keepAlive();

    void            applyCentralImpulse( const glm::vec3& impulse );

    void            serialize( FileSystemObject* file );
    void            deserialize( FileSystemObject* file );

    btRigidBody*    getNativeObject() const;

private:
    std::unique_ptr<btRigidBody>            nativeObject;
    std::unique_ptr<btDefaultMotionState>   bodyMotionState;
    float                                   mass;
};
