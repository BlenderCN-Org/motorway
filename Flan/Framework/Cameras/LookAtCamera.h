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

#include <glm/gtc/quaternion.hpp>
#include "Camera.h"

class LookAtCamera : public Camera
{
public:
					LookAtCamera();
					LookAtCamera( LookAtCamera& camera ) = default;
                    LookAtCamera& operator = ( LookAtCamera& camera ) = default;
					~LookAtCamera() = default;

    void            Update( const float frameTime ) override;
    void            Serialize( FileSystemObject* stream ) override;
    void            Deserialize( FileSystemObject* stream ) override;
    void            DrawInEditor( const float frameTime ) override;

    void			setProjectionMatrix( const float fieldOfView, const float screenWidth, float screenHeight, const float zNear = 0.01f );

    void			onMouseUpdate( const float frameTime, const double mouseDeltaX, const double mouseDeltaY ) noexcept;

    void            setNextWorldPosition( const float frameTime, const glm::vec3& worldPosition );
    void            setNextWorldRotation( const float frameTime, const glm::quat& worldRotation );

    void            isMovingCamera( const bool state );

private:
    bool            isCameraMoving;

    float           cameraHeight;
    float           cameraRadius;
     
    float           cameraTheta;
    float           cameraPhi;

    glm::vec3		nextWorldPosition;
    glm::quat       nextWorldRotation;
    glm::vec3       cameraTargetTranslation;
    glm::quat       cameraTargetRotation;

    float           cameraActiveRadius;

    float           aspectRatio;
    float           width;
    float           height;
    float           fov;
    float           nearPlane;
};
