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
#include "RigidBody.h"

#include <Core/SerializationHelpers.h>

RigidBody::RigidBody( const float bodyMass, const glm::vec3& positionWorldSpace, const glm::quat& rotationWorldSpace )
    : nativeObject( nullptr )
    , bodyMotionState( nullptr )
    , mass( bodyMass )
{
    const auto btMotionStateRotation = btQuaternion( rotationWorldSpace.x, rotationWorldSpace.y, rotationWorldSpace.z, rotationWorldSpace.w );
    const auto btMotionStateTranslation = btVector3( positionWorldSpace.x, positionWorldSpace.y, positionWorldSpace.z );
    const auto btMotionStateTransform = btTransform( btMotionStateRotation, btMotionStateTranslation );

    bodyMotionState.reset( new btDefaultMotionState( btMotionStateTransform ) );

    btVector3 localInertia( 0, 0, 0 );

    auto colliderShape = new btSphereShape( 1.0f );
    if ( mass != 0.0f ) {
        colliderShape->calculateLocalInertia( mass, localInertia );
    }

    btRigidBody::btRigidBodyConstructionInfo rigidBodyConstructionInfos( mass, bodyMotionState.get(), colliderShape, localInertia );

    nativeObject.reset( new btRigidBody( rigidBodyConstructionInfos ) );
    nativeObject->setActivationState( ACTIVE_TAG );
    nativeObject->setRestitution( 0.0f );
    nativeObject->setFriction( 0.0f );
    nativeObject->setRollingFriction( 0.0f );
}

RigidBody::RigidBody( RigidBody& rigidBody )
    : nativeObject( nullptr )
    , mass( rigidBody.mass )
    , bodyMotionState( nullptr )
{
    const auto motionStateTransform = rigidBody.nativeObject->getWorldTransform();
    bodyMotionState.reset( new btDefaultMotionState( motionStateTransform ) );

    // Copy Collider Shape
    btTransform centerOfMassTransform;
    bodyMotionState->getWorldTransform( centerOfMassTransform );

    btCollisionShape* copiedCollisionShape = nullptr;

    int shapeType = rigidBody.nativeObject->getCollisionShape()->getShapeType();
    switch ( shapeType ) {
    case BOX_SHAPE_PROXYTYPE: {
        auto boxShape = static_cast< btBoxShape* >( rigidBody.nativeObject->getCollisionShape() );
        auto boxHalfSize = boxShape->getHalfExtentsWithMargin();

        copiedCollisionShape = new btBoxShape( boxHalfSize );
    } break;

    case SPHERE_SHAPE_PROXYTYPE: {
        auto boxShape = static_cast< btSphereShape* >( rigidBody.nativeObject->getCollisionShape() );
        auto sphereRadius = boxShape->getRadius();

        copiedCollisionShape = new btSphereShape( sphereRadius );
    } break;

    case CAPSULE_SHAPE_PROXYTYPE: {
        auto capsuleShape = static_cast< btCapsuleShape* >( rigidBody.nativeObject->getCollisionShape() );
        auto capsuleHalfHeight = capsuleShape->getHalfHeight();
        auto capsuleWidth = capsuleShape->getRadius();

        copiedCollisionShape = new btCapsuleShape( capsuleWidth, capsuleHalfHeight );
    } break;

    case CYLINDER_SHAPE_PROXYTYPE: {
        auto cylinderShape = static_cast< btCylinderShape* >( rigidBody.nativeObject->getCollisionShape() );
        auto cylinderDimensions = cylinderShape->getHalfExtentsWithoutMargin();

        copiedCollisionShape = new btCylinderShape( cylinderDimensions );
    } break;

    case CONE_SHAPE_PROXYTYPE: {
        auto coneShape = static_cast< btConeShape* >( rigidBody.nativeObject->getCollisionShape() );
        auto coneHeight = coneShape->getHeight();
        auto coneRadius = coneShape->getRadius();

        copiedCollisionShape = new btConeShape( coneRadius, coneHeight );
    } break;

    case STATIC_PLANE_PROXYTYPE: {
        auto planeShape = static_cast< btStaticPlaneShape* >( rigidBody.nativeObject->getCollisionShape() );
        auto planeHeight = planeShape->getPlaneConstant();
        auto planeNormal = planeShape->getPlaneNormal();

        copiedCollisionShape = new btStaticPlaneShape( planeNormal, planeHeight );
    } break;

    default: {
        copiedCollisionShape = new btEmptyShape();
    } break;
    };

    btVector3 localInertia( 0, 0, 0 );
    if ( mass != 0.0f ) {
        copiedCollisionShape->calculateLocalInertia( mass, localInertia );
    }

    // Update Motion State
    bodyMotionState->setWorldTransform( centerOfMassTransform );

    btRigidBody::btRigidBodyConstructionInfo rigidBodyConstructionInfos( mass, bodyMotionState.get(), copiedCollisionShape, localInertia );

    nativeObject.reset( new btRigidBody( rigidBodyConstructionInfos ) );
    nativeObject->setActivationState( rigidBody.nativeObject->getActivationState() );
    nativeObject->setRestitution( rigidBody.nativeObject->getRestitution() );
    nativeObject->setFriction( rigidBody.nativeObject->getFriction() );
    nativeObject->setRollingFriction( rigidBody.nativeObject->getRollingFriction() );
}

RigidBody& RigidBody::operator = ( RigidBody& rigidBody )
{
    return *this;
}

RigidBody::~RigidBody()
{
    mass = 0.0f;
}

glm::vec3 RigidBody::getWorldPosition() const
{
    auto worldTransform = nativeObject->getWorldTransform();
    auto btWorldPosition = worldTransform.getOrigin();

    // Sub motion state offset to get the correct logic world translation
    return glm::vec3( 
        btWorldPosition.getX(), 
        btWorldPosition.getY(),
        btWorldPosition.getZ() );
}

glm::quat RigidBody::getWorldRotation() const
{
    auto worldTransform = nativeObject->getWorldTransform();
    auto btWorldRotation = worldTransform.getRotation();

    return glm::quat( btWorldRotation.getW(), btWorldRotation.getX(), btWorldRotation.getY(), btWorldRotation.getZ() );
}

void RigidBody::setMass( const float newMass )
{
    mass = std::max( newMass, 0.0f );
}

float RigidBody::getMass() const
{
    return mass;
}

void RigidBody::recomputeInertia()
{
    btVector3 inertia;
    nativeObject->getCollisionShape()->calculateLocalInertia( mass, inertia );
    nativeObject->setMassProps( mass, inertia );
}

void RigidBody::setWorldTransform( const glm::vec3& worldPosition, const glm::quat& worldRotation )
{
    // Undo logic world offset shift
    btVector3 btWorldPosition( 
        worldPosition.x, 
        worldPosition.y, 
        worldPosition.z );

    btQuaternion btWorldRotation( worldRotation.x, worldRotation.y, worldRotation.z, worldRotation.w );

    auto worldTransform = nativeObject->getWorldTransform();
    worldTransform.setRotation( btWorldRotation );
    worldTransform.setOrigin( btWorldPosition );

    nativeObject->setWorldTransform( worldTransform );
}

void RigidBody::setRestitution( const float restitution )
{
    nativeObject->setRestitution( restitution );
}

void RigidBody::setRollingFriction( const float rollingFriction )
{
    nativeObject->setRollingFriction( rollingFriction );
}

void RigidBody::setFriction( const float friction )
{
    nativeObject->setFriction( friction );
}

void RigidBody::setDamping( const float linearDamping, const float angularDamping )
{
    nativeObject->setDamping( linearDamping, angularDamping );
}

void RigidBody::setGravity( const glm::vec3& gravity )
{
    auto btGravity = btVector3( gravity.x, gravity.y, gravity.z );

    nativeObject->setGravity( btGravity );
}

void RigidBody::setActivationState( const bool activationState )
{
    nativeObject->setActivationState( static_cast<int>( activationState ) );
}

void RigidBody::keepAlive()
{
    nativeObject->setActivationState( DISABLE_DEACTIVATION );
}

void RigidBody::applyCentralImpulse( const glm::vec3& impulse )
{
    auto btImpulse = btVector3( impulse.x, impulse.y, impulse.z );

    nativeObject->applyCentralImpulse( btImpulse );
}

void RigidBody::serialize( FileSystemObject* file )
{
    FLAN_SERIALIZE_VARIABLE( file, nativeObject->getActivationState() );
    FLAN_SERIALIZE_VARIABLE( file, mass );
    FLAN_SERIALIZE_VARIABLE( file, nativeObject->getFriction() );
    FLAN_SERIALIZE_VARIABLE( file, nativeObject->getRestitution() );

    // Serialize Collider at frame N only (will be used as initial motion state transform at deserialization)
    auto collider = nativeObject->getWorldTransform();

    auto colliderPos = collider.getOrigin();
    auto colliderRot = collider.getRotation();

    FLAN_SERIALIZE_VARIABLE( file, colliderPos.getX() );
    FLAN_SERIALIZE_VARIABLE( file, colliderPos.getY() );
    FLAN_SERIALIZE_VARIABLE( file, colliderPos.getZ() );

    FLAN_SERIALIZE_VARIABLE( file, colliderRot.getW() );
    FLAN_SERIALIZE_VARIABLE( file, colliderRot.getX() );
    FLAN_SERIALIZE_VARIABLE( file, colliderRot.getY() );
    FLAN_SERIALIZE_VARIABLE( file, colliderRot.getZ() );

    int shapeType = nativeObject->getCollisionShape()->getShapeType();
    FLAN_SERIALIZE_VARIABLE( file, shapeType );

    switch ( shapeType ) {
    case BOX_SHAPE_PROXYTYPE: {
        btBoxShape* shapeBox = ( btBoxShape* )nativeObject->getCollisionShape();

        auto halfExtents = shapeBox->getImplicitShapeDimensions();
        FLAN_SERIALIZE_VARIABLE( file, halfExtents.getX() );
        FLAN_SERIALIZE_VARIABLE( file, halfExtents.getY() );
        FLAN_SERIALIZE_VARIABLE( file, halfExtents.getZ() );
    } break;

    case SPHERE_SHAPE_PROXYTYPE: {
        btSphereShape* shapeSphere = ( btSphereShape* )nativeObject->getCollisionShape();

        auto radius = shapeSphere->getRadius();
        FLAN_SERIALIZE_VARIABLE( file, radius );
    } break;

    case STATIC_PLANE_PROXYTYPE: {
        btStaticPlaneShape* staticPlaneShape = ( btStaticPlaneShape* )nativeObject->getCollisionShape();

        auto normal = staticPlaneShape->getPlaneNormal();
        FLAN_SERIALIZE_VARIABLE( file, normal.getX() );
        FLAN_SERIALIZE_VARIABLE( file, normal.getY() );
        FLAN_SERIALIZE_VARIABLE( file, normal.getZ() );

        auto height = staticPlaneShape->getPlaneConstant();
        FLAN_SERIALIZE_VARIABLE( file, height );
    } break;

    case CYLINDER_SHAPE_PROXYTYPE: {
        btCylinderShapeX* cylinderShape = ( btCylinderShapeX* )nativeObject->getCollisionShape();

        auto shapeDimensions = cylinderShape->getImplicitShapeDimensions();
        FLAN_SERIALIZE_VARIABLE( file, shapeDimensions.getX() );
        FLAN_SERIALIZE_VARIABLE( file, shapeDimensions.getY() );
    } break;

    case CAPSULE_SHAPE_PROXYTYPE: {
        btCapsuleShapeX* cylinderShape = ( btCapsuleShapeX* )nativeObject->getCollisionShape();

        float radius = cylinderShape->getRadius();
        float height = cylinderShape->getHalfHeight();

        FLAN_SERIALIZE_VARIABLE( file, radius );
        FLAN_SERIALIZE_VARIABLE( file, height );
    } break;
    }
}

void RigidBody::deserialize( FileSystemObject* file )
{
    int activationState;
    FLAN_DESERIALIZE_VARIABLE( file, activationState );
    FLAN_DESERIALIZE_VARIABLE( file, mass );

    btScalar friction, restitution;
    FLAN_DESERIALIZE_VARIABLE( file, friction );
    FLAN_DESERIALIZE_VARIABLE( file, restitution );

    glm::vec3 colliderPos;
    glm::quat colliderRot;
    FLAN_DESERIALIZE_VECTOR3( file, colliderPos );
    FLAN_DESERIALIZE_QUATERNION( file, colliderRot );

    int shapeType;
    FLAN_DESERIALIZE_VARIABLE( file, shapeType );

    btCollisionShape* collisionShape = nullptr;
    switch ( shapeType ) {
    case BOX_SHAPE_PROXYTYPE: {
        glm::vec3 halfExtents;
        FLAN_DESERIALIZE_VECTOR3( file, halfExtents );
        auto btHalfExtents = btVector3( halfExtents.x, halfExtents.y, halfExtents.z );

        collisionShape = new btBoxShape( btHalfExtents );
    } break;

    case SPHERE_SHAPE_PROXYTYPE: {
        float radius = 0.0f;
        FLAN_SERIALIZE_VARIABLE( file, radius );

        collisionShape = new btSphereShape( radius );
    } break;

    case STATIC_PLANE_PROXYTYPE: {
        glm::vec3 normal;
        FLAN_DESERIALIZE_VECTOR3( file, normal );

        float height = 0.0f;
        FLAN_DESERIALIZE_VARIABLE( file, height );

        collisionShape = new btStaticPlaneShape( btVector3( normal.x, normal.y, normal.z ), height );
    } break;

    case CYLINDER_SHAPE_PROXYTYPE: {
        float radius, depth;
        FLAN_DESERIALIZE_VARIABLE( file, radius );
        FLAN_DESERIALIZE_VARIABLE( file, depth );

        collisionShape = new btCylinderShape( btVector3( radius, depth, radius ) );
    } break;

    case CAPSULE_SHAPE_PROXYTYPE: {
        float radius, height;
        FLAN_DESERIALIZE_VARIABLE( file, radius );
        FLAN_DESERIALIZE_VARIABLE( file, height );

        collisionShape = new btCapsuleShape( radius, height );
    } break;
    }

    const auto btMotionStateRotation = btQuaternion( colliderRot.x, colliderRot.y, colliderRot.z, colliderRot.w );
    const auto btMotionStateTranslation = btVector3( colliderPos.x, colliderPos.y, colliderPos.z );
    const auto btMotionStateTransform = btTransform( btMotionStateRotation, btMotionStateTranslation );

    bodyMotionState.reset( new btDefaultMotionState( btMotionStateTransform ) );

    btVector3 localInertia( 0, 0, 0 );
    if ( mass != 0.0f ) {
        collisionShape->calculateLocalInertia( mass, localInertia );
    }

    btRigidBody::btRigidBodyConstructionInfo rigidBodyConstructionInfos( mass, bodyMotionState.get(), collisionShape, localInertia );

    nativeObject.reset( new btRigidBody( rigidBodyConstructionInfos ) );
    nativeObject->setActivationState( ACTIVE_TAG );
    nativeObject->setRestitution( 0.0f );
    nativeObject->setFriction( 0.0f );
    nativeObject->setRollingFriction( 0.0f );
}

btRigidBody* RigidBody::getNativeObject() const
{
    return nativeObject.get();
}
