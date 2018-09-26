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

#include <Framework/GameObject.h>

class MotorizedVehicle;
class Transform;

class VehicleLogic : public GameObject
{
public:
                        VehicleLogic( MotorizedVehicle* vehiclePhysics );
                        VehicleLogic( VehicleLogic& vehicleLogic ) = default;
                        ~VehicleLogic() = default;

    virtual void        Update( const float frameTime ) override;
    virtual void        Serialize( FileSystemObject* stream ) override;
    virtual void        Deserialize( FileSystemObject* stream ) override;

    void Steer( const float frameTime, const float axisValue );
    void Accelerate( const float frameTime );
    void Brake( const float frameTime );

private:
    MotorizedVehicle*   vehicleRigidBody;
};
