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

#include <vector>

#include "RigidBody.h"
#include "MotorizedVehicle.h"

class DynamicsWorld
{
public:
    inline btDiscreteDynamicsWorld*                         getNativeDynamicsWorld() { return dynamicsWorld.get(); }

public:
                                                            DynamicsWorld();
                                                            DynamicsWorld( DynamicsWorld& ) = delete;
                                                            ~DynamicsWorld();

    void                                                    create();
    void                                                    update( const float frameTime );

    void                                                    addRigidBody( RigidBody* rigidBody );
    void                                                    addConstraint( btTypedConstraint* constraint, const bool disableCollisionsBetweenLinkedBodies = false );
    void                                                    addMotorizedVehicle( MotorizedVehicle* motorizedVehicle );

    void                                                    removeRigidBody( RigidBody* rigidBody );

private:
    std::unique_ptr<btBroadphaseInterface>                  broadphase;
    std::unique_ptr<btDefaultCollisionConfiguration>        collisionConfiguration;
    std::unique_ptr<btCollisionDispatcher>                  dispatcher;
    std::unique_ptr<btSequentialImpulseConstraintSolver>    solver;
    std::unique_ptr<btDiscreteDynamicsWorld>                dynamicsWorld;

    std::vector<MotorizedVehicle*>                          motorizedVehicles;
    std::vector<RigidBody*>                                 rigidBodies;
    std::vector<btTypedConstraint*>                         constraints;
};
