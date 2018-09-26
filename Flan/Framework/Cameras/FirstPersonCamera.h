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

class FirstPersonCamera : public Camera
{
public:
					FirstPersonCamera();
					FirstPersonCamera( FirstPersonCamera& camera ) = default;
                    FirstPersonCamera& operator = ( FirstPersonCamera& camera ) = default;
					~FirstPersonCamera() = default;

    void            Update( const float frameTime ) override;
    void            Serialize( FileSystemObject* stream ) override;
    void            Deserialize(  FileSystemObject* stream ) override;
    void            DrawInEditor( const float frameTime ) override;

	void			setProjectionMatrix( const float fieldOfView, const float screenWidth, float screenHeight, const float zNear = 0.01f );

	void			onMouseUpdate( const float frameTime, const double mouseDeltaX, const double mouseDeltaY ) noexcept;
    
    void            setNextWorldPosition( const float frameTime, const glm::vec3& worldPosition );
    void            setNextWorldRotation( const float frameTime, const glm::quat& worldRotation );

    const glm::vec3& getRightVector() const;
    const glm::vec3& getEyeDirection() const;

    void            setRunningState();

private:
	glm::vec3		rightVector;
	glm::vec3		eyeDirection;

    glm::vec3		nextWorldPosition;
    glm::quat       nextWorldRotation;
    glm::quat       cameraTargetRotation;

	float			yaw;
	float			pitch;
	float			roll;
    float           aspectRatio;

    float           width;
    float           height;
    float           fov;
    float           nearPlane;

    float           cameraTime;
    bool            applyBobbing;
};
