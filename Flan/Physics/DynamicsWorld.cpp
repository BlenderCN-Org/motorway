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
#include "DynamicsWorld.h"

DynamicsWorld::DynamicsWorld()
    : broadphase( new btDbvtBroadphase() )
    , collisionConfiguration( new btDefaultCollisionConfiguration() )
    , dispatcher( new btCollisionDispatcher( collisionConfiguration.get() ) )
    , solver( new btSequentialImpulseConstraintSolver() )
    , dynamicsWorld( nullptr )
{

}

DynamicsWorld::~DynamicsWorld()
{
    // DynamicsWorld should NOT have the ownership of any registered physics entities
    rigidBodies.clear();
    constraints.clear();
}

void DynamicsWorld::create()
{
    dynamicsWorld.reset( new btDiscreteDynamicsWorld( dispatcher.get(), broadphase.get(), solver.get(), collisionConfiguration.get() ) );
    dynamicsWorld->setGravity( btVector3( 0.0f, -9.80665f, 0.0f ) );
}

void DynamicsWorld::update( const float frameTime )
{
    for ( auto motorizedVehicle : motorizedVehicles ) {
        motorizedVehicle->PreStepUpdate( frameTime, dynamicsWorld.get() );
    }

    dynamicsWorld->stepSimulation( frameTime, 0, frameTime );

    for ( auto motorizedVehicle : motorizedVehicles ) {
        motorizedVehicle->PostStepUpdate( frameTime );
    }
}

void DynamicsWorld::addRigidBody( RigidBody* rigidBody )
{
    dynamicsWorld->addRigidBody( rigidBody->getNativeObject() );

    rigidBody->keepAlive();
    rigidBodies.push_back( rigidBody );
}

void DynamicsWorld::addConstraint( btTypedConstraint* constraint, const bool disableCollisionsBetweenLinkedBodies )
{
    dynamicsWorld->addConstraint( constraint, disableCollisionsBetweenLinkedBodies );

    constraints.push_back( constraint );
}

void DynamicsWorld::removeRigidBody( RigidBody* rigidBody )
{
    dynamicsWorld->removeRigidBody( rigidBody->getNativeObject() );
}

void DynamicsWorld::addMotorizedVehicle( MotorizedVehicle* motorizedVehicle )
{
    addRigidBody( motorizedVehicle->GetChassisRigidBody() );

    motorizedVehicles.push_back( motorizedVehicle );
}
