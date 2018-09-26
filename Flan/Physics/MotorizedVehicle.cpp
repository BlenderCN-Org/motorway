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
#include "MotorizedVehicle.h"

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/common.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <Physics/RigidBody.h>

MotorizedVehicle::MotorizedVehicle( RigidBody* rigidBody )
    : chassisRigidBody( rigidBody )
    , driveBalance( PROPULSION_TYPE_FWD )
    , brakeBalance( driveBalance )
    , chassisMass( 1600.0f )
    , decayControl( 0.1f )
    , decayThreshold( 9.0f )
    , slideControl( 0.5f )
    , spinControl( 0.2f )
    , sprintMultiplicator( 1.40f )
    , forceAccumulation( 0.0f, 0.0f, 0.0f )
    , torqueAccumulation( 0.0f, 0.0f, 0.0f )
    , steer( 0.0f )
    , frameInputSteer( 0.0f )
    , wheelCount( 4 )
    , torqueCurve{ 1.0f, 0.2f }
    , maxTorque( 4000.0f )
    , drag( 200.0f )
    , compressionDrag( 200.0f )
    , maxAngularVelocity( 120.0f )
    , limiter( 200.0f )
    , limiterTime( 0.1f )
    , gear( 0.1f )
    , currentLimitation( 0.0f )
    , gasInput( 0.0f )
    , brakeInput( 0.0f )
    , isFlipping( false )
{
    vehicleWheels = new std::unique_ptr<VehicleWheel>[4]{ nullptr, nullptr, nullptr, nullptr };

    static constexpr float front_axle_xpos = 1.45f;
    static constexpr float front_axle_width = 1.550f;
    static constexpr float rear_axle_xpos = -1.20f;
    static constexpr float rear_axle_width = 1.550f;
    static constexpr float tire_radius = 0.33f;
    static constexpr int WHEEL_COUNT = 4;

    glm::vec3 wheelWorldPosition[WHEEL_COUNT] = {
        glm::vec3( -front_axle_width * 0.5f, 0, front_axle_xpos ),
        glm::vec3( front_axle_width * 0.5f, 0, front_axle_xpos ),
        glm::vec3( -rear_axle_width * 0.5f, 0, rear_axle_xpos ),
        glm::vec3( rear_axle_width * 0.5f, 0, rear_axle_xpos )
    };

    for ( int i = 0; i < wheelCount; i++ ) {
        vehicleWheels[i].reset( new VehicleWheel );

        auto wheel = vehicleWheels[i].get();
        wheel->MaxBrakeTorque = 2000.0f;
        wheel->DragCoefficient = 0.4f;
        wheel->WheelRadius = tire_radius;
        wheel->MomentOfInertia = 8.0f;
        wheel->FricitionCoefficient = 1.0f;
        wheel->LostGripSlidingFactor = 0.8f;
        wheel->WheelTiltEdgeFactor = 0.2f;
        wheel->SteerRate = 1.5f;
        wheel->SteerLock = 0.0f;

        wheel->SuspensionRestLength = 0.22f;
        wheel->SuspensionForce = 15000.0f;
        wheel->SuspensionDampingFactor = 5000.0f;
        wheel->SuspensionExponent = 1.5f;

        wheel->PatchMass = 800.0f;
        wheel->PatchRadius = 0.1f;
        wheel->PatchSlipingFactor = 0.1f;

        wheel->PatchTensionTorque = 0.0f;
        wheel->FinalTorque = 0.0f;
        wheel->AngularVelocity = 0.0f;
        wheel->AxialAngle = 0.0f;
        wheel->GripState = 0.0f;
        wheel->SteerAngle = 0.0f;

        wheel->CurrentSuspensionLength = wheel->SuspensionRestLength;

        wheel->PatchSpringRate = wheel->PatchMass * 60.0f;
        wheel->PatchCriticalDampingCoefficient = pow( 4.0f * wheel->PatchMass * wheel->PatchSpringRate, 0.5f );
        wheel->PatchAveragePosition = glm::vec3( 0.0f );
        wheel->PatchAverageVelocity = glm::vec3( 0.0f );

        if ( i > 1 ) {
            vehicleWheels[i]->SteerLock = 0.6f;
        } else {
            vehicleWheels[i]->FricitionCoefficient = 1.0f;
            vehicleWheels[i]->SuspensionRestLength = 0.22f;
            vehicleWheels[i]->SuspensionForce = 9001.0f;
            vehicleWheels[i]->SuspensionDampingFactor = 3000.0f;
            vehicleWheels[i]->SuspensionExponent = 1.0f;
            vehicleWheels[i]->SteerRate = 1.8f;
            vehicleWheels[i]->PatchMass = 400.0f;
        }

        vehicleWheels[i]->BasePosition = wheelWorldPosition[i];
        vehicleWheels[i]->BaseOrientation = glm::quat( 1, 0, 0, 0 );
        vehicleWheels[i]->OrientationMatrix = glm::toMat3( vehicleWheels[i]->BaseOrientation );

        wheel->Position = vehicleWheels[i]->BasePosition;
        wheel->Orientation = vehicleWheels[i]->BaseOrientation;
    }
}

void MotorizedVehicle::PreStepUpdate( const float frameTime, btDynamicsWorld* dynamicsWorld )
{
    // Flip the vehicle around Z axis (basic recover)
    if ( isFlipping ) {
        chassisRigidBody->getNativeObject()->setAngularVelocity( btVector3( 0.0f, 0.0f, 2.0f ) );
        isFlipping = false;
    }

    // Update chassis simulation
    auto angularVelocity = chassisRigidBody->getNativeObject()->getAngularVelocity();
    auto linearVelocity = chassisRigidBody->getNativeObject()->getLinearVelocity();

    float inputSteer = frameInputSteer /
        ( 1 + ( ( glm::pow( glm::max( abs( linearVelocity.getZ() ) - decayThreshold, 0.0f ), 0.9f ) ) * decayControl ) );

    float slideSteer = 0.0f;
    glm::vec2 slideVector = glm::vec2( linearVelocity.getX(), linearVelocity.getZ() );
    float slideVelocity = glm::length( slideVector );

    if ( slideVector.y > 0.0f ) {
        slideSteer = glm::angle( slideVector, glm::vec2( 0, 1 ) ) * ( static_cast< float >( slideVector.x > 0.0f ) - 0.5f ) * 2.0f;
    } else if ( slideVector.y < 0.0f ) {
        slideSteer = glm::angle( slideVector, glm::vec2( 0, -1 ) ) * ( static_cast< float >( slideVector.x > 0.0f ) - 0.5f ) * 2.0f;
    }

    slideSteer = slideSteer * ( ( 1 - ( 1 / ( 1 + slideVelocity ) ) ) * slideControl );
    slideSteer *= static_cast<float>( abs( slideVector.y ) > 6.0f );

    float spinSteer = angularVelocity.getY() * slideControl;
    steer = glm::clamp( inputSteer + slideSteer + spinSteer, -1.0f, 1.0f );

    auto Towards = [&]( float current, float target, float amount ) {
        if ( current > target ) {
            return current - glm::min( abs( target - current ), amount );
        } else if ( current < target ) {
            return current + glm::min( abs( target - current ), amount );
        }

        return current;
    };

    auto VecTrack = [&]( const glm::vec3& v0, const glm::vec3& v1, int lockAxis = 1, int trackAxis = 2 ) {
        if ( lockAxis == trackAxis ) {
            return glm::mat3( 1.0f );
        }

        auto lock = glm::normalize( v0 );
        auto track = glm::normalize( v1 );

        if ( abs( glm::dot( lock, track ) ) == 1.0f ) {
            return glm::mat3( 1.0f );
        }

        auto other = glm::normalize( glm::cross( track, lock ) );
        track = glm::cross( lock, other );

        auto otherAxis = 3 - ( lockAxis + trackAxis );

        if ( !( ( lockAxis - 1 ) % 3 == trackAxis ) ) {
            other = -other;
        }

        glm::mat3 r = glm::mat3( 0.0f );
        r[lockAxis] = lock;
        r[trackAxis] = track;
        r[otherAxis] = other;

        return r;
    };

    auto veclineproject = [&]( const glm::vec3& vec, const glm::vec3& nor ) {
        glm::vec3 n = glm::normalize( nor );
        return nor * glm::dot( nor, vec );
    };

    auto vecplaneproject = [&]( const glm::vec3& vec, const glm::vec3& nor ) {
        glm::vec3 n = glm::normalize( nor );
        return vec - veclineproject( vec, n );
    };

    auto vecToLine = [&]( glm::vec3& vec, glm::vec3& nor ) {
        return veclineproject( vec, nor ) - vec;
    };

    auto limit = [&]( float value, float lower, float upper ) {
        return glm::max( glm::min( value, upper ), lower );
    };

    auto lerp = [&]( float p0, float p1, float t ) {
        return p0 + ( p1 - p0 ) * limit( t, 0.0, 1.0 );
    };

    auto SafeNormalize = [&]( glm::vec3& vectorToNormalize ) {
        // For some reason, glm allows null vector normalization (which leads to a division by zero)
        // So we need to check if the vector length is null to avoid evilish NaN
        vectorToNormalize = ( glm::length( vectorToNormalize ) == 0.0f ) ? glm::vec3( 0.0f ) : glm::normalize( vectorToNormalize );
        return vectorToNormalize;
    };

    auto bpos = chassisRigidBody->getWorldPosition();
    auto bmat = glm::mat3_cast( chassisRigidBody->getWorldRotation() );

    for ( int i = 0; i < wheelCount; i++ ) {
        auto wheel = vehicleWheels[i].get();

        // Do steering
        wheel->SteerAngle = Towards( wheel->SteerAngle, -wheel->SteerLock * steer, wheel->SteerRate * frameTime );

        // Generate wheel matrix and apply steering
        auto wmat = bmat * ( wheel->OrientationMatrix * glm::mat3_cast( glm::rotate( glm::quat( 1, 0, 0, 0 ), wheel->SteerAngle, glm::vec3( 0, 1, 0 ) ) ) );

        // Ray coordinates
        auto p0 = wheel->Position;
        auto depth = ( glm::column( ( bmat * wheel->OrientationMatrix ), 1 ) * ( wheel->SuspensionRestLength + wheel->WheelRadius ) );
        auto p1 = p0 - depth;

        btVector3 btP0 = btVector3( p0.x, p0.y, p0.z );
        btVector3 btP1 = btVector3( p1.x, p1.y, p1.z );

        btCollisionWorld::ClosestRayResultCallback rayResult( btP0, btP1 );
        dynamicsWorld->rayTest( btP0, btP1, rayResult );

        if ( rayResult.hasHit() ) {
            // Retrieve hit infos and compute common terms for the rest of the update
            auto hob = rayResult.m_collisionObject;
            
            auto hitNormal = glm::normalize( glm::vec3( rayResult.m_hitNormalWorld.getX(), rayResult.m_hitNormalWorld.getY(), rayResult.m_hitNormalWorld.getZ() ) );
            auto hmat = VecTrack( hitNormal, glm::column( wmat, 2 ) );

            auto hpos = glm::vec3( rayResult.m_hitPointWorld.getX(), rayResult.m_hitPointWorld.getY(), rayResult.m_hitPointWorld.getZ() );

            float flatness = glm::dot( glm::column( hmat, 1 ), glm::column( wmat, 1 ) );

            auto hitPositionBody = hpos - bpos;

            auto btHitObjectWorldPosition = hob->getWorldTransform().getOrigin();
            auto hitObjectWorldPosition = glm::vec3( btHitObjectWorldPosition.getX(), btHitObjectWorldPosition.getY(), btHitObjectWorldPosition.getZ() );
            auto hitOriginBody = hpos - hitObjectWorldPosition;

            btVector3 hpos_body = btVector3( hitPositionBody.x, hitPositionBody.y, hitPositionBody.z );
            btVector3 hpos_hob = btVector3( hitOriginBody.x, hitOriginBody.y, hitOriginBody.z );

            auto rigidBodyHitObject = btRigidBody::upcast( hob );

            if ( rigidBodyHitObject && !rigidBodyHitObject->hasContactResponse() ) {
                continue;
            }

            auto hitPointVelocity = chassisRigidBody->getNativeObject()->getVelocityInLocalPoint( hpos_body ) - rigidBodyHitObject->getVelocityInLocalPoint( hpos_hob );
            auto hvel = glm::vec3( hitPointVelocity.getX(), hitPointVelocity.getY(), hitPointVelocity.getZ() );

            // Update suspension length based on raycast's result with the rest of the world
            wheel->CurrentSuspensionLength = glm::max( glm::length( p0 - hpos ) - wheel->WheelRadius, 0.0f );

            float suspensionFac = 1.0f - wheel->CurrentSuspensionLength / wheel->SuspensionRestLength;
            float suspensionSpringForce = glm::pow( suspensionFac, wheel->SuspensionExponent ) * wheel->SuspensionForce;
            suspensionSpringForce -= glm::dot( hvel, glm::column( hmat, 1 ) ) * wheel->SuspensionDampingFactor;

            // Update per-wheel traction
            auto tp_pos = wheel->PatchAveragePosition;
            tp_pos -= hvel * frameTime;
            float rollDistance = wheel->AngularVelocity * wheel->WheelRadius * frameTime;
            float rollFactor = abs( rollDistance ) / ( wheel->PatchRadius * 2.0f );
            tp_pos += rollDistance * glm::column( hmat, 2 );

            tp_pos = vecplaneproject( tp_pos, glm::column( hmat, 1 ) );
            tp_pos += vecToLine( tp_pos, glm::column( hmat, 2 ) ) * ( 1.0f - 1.0f / ( 1.0f + rollFactor ) ) * wheel->PatchSlipingFactor;

            auto fNormal = glm::max( suspensionSpringForce, 0.0f );
            auto fTraction = wheel->FricitionCoefficient * fNormal * lerp( wheel->WheelTiltEdgeFactor, 1.0f, flatness );

            auto tp_max = fTraction / wheel->PatchSpringRate;
            tp_pos = SafeNormalize( tp_pos ) * ( glm::min( tp_max, glm::length( tp_pos ) ) );

            wheel->PatchAveragePosition = SafeNormalize( wheel->PatchAveragePosition ) * glm::min( tp_max, glm::length( wheel->PatchAveragePosition ) );

            wheel->PatchAverageVelocity = ( tp_pos - wheel->PatchAveragePosition ) / frameTime;
            wheel->PatchAveragePosition = tp_pos;

            auto fPatch = wheel->PatchAveragePosition * wheel->PatchSpringRate + wheel->PatchAverageVelocity * wheel->PatchCriticalDampingCoefficient / ( 1.0f + rollFactor );

            if ( glm::length( fPatch ) > fTraction ) {
                fPatch = glm::normalize( fPatch ) * ( fTraction * wheel->LostGripSlidingFactor );
            }

            if ( fTraction != 0.0f ) {
                wheel->GripState = limit( 1.0f - ( glm::length( wheel->PatchAveragePosition ) * wheel->PatchSpringRate ) / fTraction, 0.0f, 1.0f );
            } else {
                wheel->GripState = 0.0f;
            }

            wheel->PatchTensionTorque = -( glm::dot( fPatch, glm::column( hmat, 2 ) ) * wheel->WheelRadius );

            auto force = fNormal * glm::column( hmat, 1 ) + fPatch;

            forceAccumulation += force;
            torqueAccumulation += glm::cross( hitPositionBody, force );
        } else {
            wheel->CurrentSuspensionLength = wheel->SuspensionRestLength;
            wheel->PatchAveragePosition = { 0, 0, 0 };
            wheel->PatchAverageVelocity = { 0, 0, 0 };
            wheel->PatchTensionTorque = 0.0f;
        }
    }

    // Front Axle
    for ( int i = 0; i < wheelCount; i++ ) {
        auto wheel = vehicleWheels[i].get();

        auto torque = wheel->PatchTensionTorque;
        torque -= wheel->AngularVelocity * wheel->DragCoefficient;

        torque += GetTorque( wheel->AngularVelocity ) / wheelCount;
        torque = Towards( torque, -( wheel->AngularVelocity * wheel->MomentOfInertia ) / frameTime, wheel->MaxBrakeTorque * brakeInput );

        wheel->FinalTorque = torque;
    }

    // Rear Axle
    float torque = 0.0f;
    float patchDragSum = 0.0f;
    for ( int i = 0; i < wheelCount; i++ ) {
        torque += vehicleWheels[i]->PatchTensionTorque;
        patchDragSum += ( vehicleWheels[i]->AngularVelocity * vehicleWheels[i]->DragCoefficient );
    }

    torque -= patchDragSum;
    torque += GetTorque( vehicleWheels[1]->AngularVelocity );

    float brake = 0.0f;
    float momentum = 0.0f;
    float momenSum = 0.0f;
    for ( int i = 0; i < wheelCount; i++ ) {
        brake += vehicleWheels[i]->MaxBrakeTorque * brakeInput;
        momentum += vehicleWheels[i]->AngularVelocity * vehicleWheels[i]->MomentOfInertia;
        momenSum += vehicleWheels[i]->MomentOfInertia;
    }

    momentum = -momentum;
    torque = Towards( torque, momentum / frameTime, brake );

    for ( int i = 0; i < wheelCount; i++ ) {
        vehicleWheels[i]->FinalTorque = torque * ( vehicleWheels[i]->MomentOfInertia / momenSum );
    }
}

void MotorizedVehicle::PostStepUpdate( const float frameTime )
{
    auto bpos = chassisRigidBody->getWorldPosition();
    auto bmat = glm::mat3_cast( chassisRigidBody->getWorldRotation() );

    // Physics Steppin'
    // Apply wheel forces to hull
    auto btForceAccumulation = btVector3( forceAccumulation.x, forceAccumulation.y, forceAccumulation.z );
    auto btTorqueAccumulation = btVector3( torqueAccumulation.x, torqueAccumulation.y, torqueAccumulation.z );

    chassisRigidBody->getNativeObject()->applyCentralForce( btForceAccumulation );
    chassisRigidBody->getNativeObject()->applyTorque( btTorqueAccumulation );

    forceAccumulation = { 0, 0, 0 };
    torqueAccumulation = { 0, 0, 0 };

    for ( int i = 0; i < wheelCount; i++ ) {
        auto wheel = vehicleWheels[i].get();

        wheel->AngularVelocity += ( wheel->FinalTorque / wheel->MomentOfInertia ) * frameTime;
        wheel->AxialAngle += wheel->AngularVelocity * frameTime;

        // Update wheel position according to suspension length
        auto updatedWheelPosition = wheel->BasePosition - glm::column( wheel->OrientationMatrix, 1 ) * wheel->CurrentSuspensionLength;
        updatedWheelPosition = bpos + bmat * updatedWheelPosition;

        // Update wheel rotation
        wheel->AxialAngle = glm::fmod( wheel->AxialAngle, 2.0f * glm::pi<float>() );
        auto wheelSteer = glm::mat3_cast( glm::rotate( glm::quat( 1, 0, 0, 0 ), wheel->SteerAngle, glm::vec3( 0, 1, 0 ) ) );
        auto wheelSpin = glm::mat3_cast( glm::rotate( glm::quat( 1, 0, 0, 0 ), -wheel->AxialAngle, glm::vec3( 1, 0, 0 ) ) );

        auto updatedWheelOrientation = wheel->OrientationMatrix * wheelSteer * wheelSpin;

        // Update wheel transform
        wheel->Position = glm::vec3( updatedWheelPosition.x, updatedWheelPosition.y, updatedWheelPosition.z );
        wheel->Orientation = glm::quat( bmat * updatedWheelOrientation );

        wheel->SpeedInKmh = wheel->AngularVelocity * wheel->WheelRadius * -3.60f;
        
        //float mph = wheel->SpeedInKmh * 0.62137119f;
    }

    if ( currentLimitation > 0.0f ) {
        currentLimitation -= frameTime;
    }

    frameInputSteer = 0.0f;
    brakeInput = 0.0f;
    gasInput = 0.0f;
}

void MotorizedVehicle::Steer( const float steerValue )
{
    frameInputSteer = -steerValue;
}

void MotorizedVehicle::SetGasState( const float gasValue )
{
    gasInput = -gasValue;
}

void MotorizedVehicle::SetBrakeState( const float brakeValue )
{
    brakeInput = brakeValue;
}

void MotorizedVehicle::Flip()
{
    isFlipping = true;
}

float MotorizedVehicle::GetTorque( const float angularVelocity )
{
    if ( abs( angularVelocity ) > limiter ) {
        currentLimitation = limiterTime;
    }

    float motorSpeed = angularVelocity / maxAngularVelocity;

    auto lerp1 = [&]( const float p0, const float p1, const float t ) { return p0 + ( p1 - p0 ) * t; };
    auto multilerp = [&]( const std::vector<float>& torqueCurve, const float time ) {
        if ( torqueCurve.size() == 0 ) {
            return 0.0f;
        } else if ( torqueCurve.size() == 1 ) {
            return torqueCurve.at( 0 );
        }

        float t = glm::clamp( abs( time ), 0.0f, 1.0f );
        if ( t == 1.0f ) {
            return torqueCurve.back();
        } else if ( t == 0.0f ) {
            return torqueCurve.front();
        }

        t *= ( torqueCurve.size() - 1 );
        int i0 = static_cast< int >( t );
        int i1 = i0 + 1;

        return lerp1( torqueCurve[i0], torqueCurve[i1], t - i0 );
    };

    float torque = 0.0f;
    if ( currentLimitation <= 0.0f ) {
        torque = multilerp( torqueCurve, abs( motorSpeed ) ) * maxTorque * gasInput;
        torque -= drag * motorSpeed;
        torque -= compressionDrag * motorSpeed * ( 1.0f - abs( gasInput ) );
    }

    return torque;
}
