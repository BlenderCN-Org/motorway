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

#include <vector>
#include "RigidBody.h"

enum ePropulsionType
{
    PROPULSION_TYPE_RWD = 0,
    PROPULSION_TYPE_AWD = 1,
    PROPULSION_TYPE_FWD = 2,
};

// TODO Should probably be stored in its own class
struct VehicleWheel
{
    float   SpeedInKmh;

    float   MaxBrakeTorque;
    float   DragCoefficient;
    float   WheelRadius;
    float   MomentOfInertia;

    float   FricitionCoefficient;

    float   LostGripSlidingFactor;
    float   WheelTiltEdgeFactor;

    float   SteerRate;
    float   SteerLock;

    float   SuspensionRestLength;
    float   SuspensionForce;
    float   SuspensionDampingFactor;
    float   SuspensionExponent;

    float   PatchMass;
    float   PatchRadius;
    float   PatchSlipingFactor;

    float   PatchTensionTorque;
    float   FinalTorque;
    float   AngularVelocity;
    float   AxialAngle;
    float   GripState;
    float   SteerAngle;
    float   CurrentSuspensionLength;

    float       PatchSpringRate;
    float       PatchCriticalDampingCoefficient;
    glm::vec3   PatchAveragePosition;
    glm::vec3   PatchAverageVelocity;

    glm::mat3   OrientationMatrix;

    glm::vec3   HeightVelocity;

    glm::quat   BaseOrientation;
    glm::vec3   BasePosition;

    glm::quat   Orientation;
    glm::vec3   Position;
};

class MotorizedVehicle
{
public:
    RigidBody*      GetChassisRigidBody() { return chassisRigidBody.get(); }
    int             GetWheelCount() { return wheelCount; }
    VehicleWheel*   GetWheelByIndex( const int wheelIndex ) { return vehicleWheels[wheelIndex].get(); }

public:
                    MotorizedVehicle( RigidBody* rigidBody );
                    MotorizedVehicle( MotorizedVehicle& vehicle ) = default;
                    ~MotorizedVehicle() = default;

    void            PreStepUpdate( const float frameTime, btDynamicsWorld* dynamicsWorld );
    void            PostStepUpdate( const float frameTime );

    void            Steer( const float steerValue );
    void            SetGasState( const float gasValue );
    void            SetBrakeState( const float brakeValue );
    void            Flip();

private:
    // Vehicle
    ePropulsionType driveBalance;
    ePropulsionType brakeBalance;

    // Lessen steering as speed increases (0 = no effect)
    float           decayControl;
    // Start to lessen steering at this speed (m/s)
    float           decayThreshold;
    // Steer against sideways sliding
    float           slideControl;
    // Steer against spinning
    float           spinControl;
    // Drive torque multiplier when sprinting
    float           sprintMultiplicator;

    // Chassis
    std::unique_ptr<RigidBody>      chassisRigidBody;

    // Wheels
    int                             wheelCount;
    std::unique_ptr<VehicleWheel>*  vehicleWheels;

    glm::vec3       forceAccumulation;
    glm::vec3       torqueAccumulation;
    float           steer;

    float           frameInputSteer;
    float           gasInput;
    float           brakeInput;
    bool            isFlipping;

    float           chassisMass;

    // Engine
    std::vector<float>  torqueCurve;
    float               maxTorque;
    float               drag;
    float               compressionDrag;
    float               maxAngularVelocity;
    float               limiter;
    float               limiterTime;
    float               gear;

    float               currentLimitation;

private:
    float               GetTorque( const float angularVelocity );
};