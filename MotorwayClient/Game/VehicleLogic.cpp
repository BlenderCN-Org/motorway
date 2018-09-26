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
#include <Shared.h>
#include "VehicleLogic.h"

#include <Physics/MotorizedVehicle.h>
#include <Core/Maths/Transform.h>

VehicleLogic::VehicleLogic( MotorizedVehicle* vehiclePhysics )
    : vehicleRigidBody( vehiclePhysics )
{

}

void VehicleLogic::Update( const float frameTime )
{
    //static constexpr float WHEEL_EPSILON = 0.01f;

    //for ( int i = 0; i < motorizedVehicle->GetWheelCount(); i++ ) {
    //    auto wheelTransformComponent = wheelGameObjects[i]->GetComponent<Transform>(); //static_cast<Transform*>( ->GetComponent( PH_STRING_HASH( "Transform" ) ) );
    //    auto wheelInfos = motorizedVehicle->GetWheelByIndex( i );

    //    auto wheelPos = wheelInfos->Position;
    //    wheelPos.y -= wheelInfos->CurrentSuspensionLength + WHEEL_EPSILON;

    //    wheelTransformComponent->SetTranslation( wheelPos );
    //    wheelTransformComponent->SetRotation( ( i == 1 || i == 3 ) ? glm::rotate( wheelInfos->Orientation, glm::radians( 180.0f ), glm::vec3( 0, 1, 0 ) ) : wheelInfos->Orientation );
    //}
}

void VehicleLogic::Serialize( FileSystemObject* stream )
{
    GameObject::Serialize( stream );
}

void VehicleLogic::Deserialize( FileSystemObject* stream )
{
    GameObject::Deserialize( stream );
}

void VehicleLogic::Steer( const float frameTime, const float axisValue )
{
    vehicleRigidBody->Steer( axisValue );
}

void VehicleLogic::Accelerate( const float frameTime )
{
    vehicleRigidBody->SetGasState( 1.0f );
}

void VehicleLogic::Brake( const float frameTime )
{
    vehicleRigidBody->SetBrakeState( 1.0f );
}