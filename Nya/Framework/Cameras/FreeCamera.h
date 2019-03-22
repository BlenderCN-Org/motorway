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

class FileSystemObject;

#include "Camera.h"

class FreeCamera
{
public:
                    FreeCamera( const float camSpeed = 50.0f );
                    FreeCamera( FreeCamera& camera ) = default;
                    ~FreeCamera() = default;

    void            update( const float frameTime );
    void            updateMouse( const float frameTime, const double mouseDeltaX, const double mouseDeltaY ) noexcept;

    void            save( FileSystemObject* stream );
    void            restore( FileSystemObject* stream );

    void            setProjectionMatrix( const float fieldOfView, const float screenWidth, float screenHeight, const float zNear = 0.01f );

    void            moveForward( const float dt );
    void            moveBackward( const float dt );
    void            moveLeft( const float dt );
    void            moveRight( const float dt );

    void            takeAltitude( const float dt );
    void            lowerAltitude( const float dt );

    // NOTE Override current camera state
    void            setOrientation( const float yaw, const float pitch, const float roll );
    nyaVec3f        getOrientation() const { return nyaVec3f( yaw, pitch, roll ); }

    CameraData&     getData();

    decltype( CameraData::flags )& getUpdatableFlagset()
    {
        return data.flags;
    }

    void                  setMSAASamplerCount( const uint32_t samplerCount = 1 );
    void                  setImageQuality( const float imageQuality = 1.0f );

private:
    CameraData      data;

    nyaVec3f        rightVector;
    nyaVec3f        eyeDirection;

    float           speedX;
    float           speedY;
    float           speedAltitude;

    float           yaw;
    float           pitch;
    float           roll;

    float           moveSpeed;
    float           moveDamping;

    float           aspectRatio;
    float           fov;
    float           nearPlane;
};
