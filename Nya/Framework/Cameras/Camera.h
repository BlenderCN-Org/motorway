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

#include <Maths/Frustum.h>
#include <Shaders/ShadowMappingShared.h>

struct CameraData
{
    glm::mat4    viewMatrix;
    glm::mat4    projectionMatrix;
    glm::mat4    inverseViewMatrix;
    glm::mat4    inverseProjectionMatrix;
    glm::mat4    viewProjectionMatrix;

    glm::mat4    inverseViewProjectionMatrix;
    glm::vec3    worldPosition;
    int32_t      cameraFrameNumber;

    // Shadow mapping rendering specifics
    glm::mat4    depthProjectionMatrix;
    glm::mat4    depthViewProjectionMatrix;
    glm::vec4    cascadeOffsets[CSM_SLICE_COUNT];
    glm::vec4    cascadeScales[CSM_SLICE_COUNT];
    float        cascadeSplitDistances[CSM_SLICE_COUNT];
    glm::mat4    shadowViewMatrix[CSM_SLICE_COUNT];
    glm::mat4    globalShadowMatrix;

    // Temporal infos
    glm::mat4    previousViewProjectionMatrix;
    glm::mat4    previousViewMatrix;
    glm::vec2    jitteringOffset;
    glm::vec2    previousJitteringOffset;

    Frustum      frustum;
};
