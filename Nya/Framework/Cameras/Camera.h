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

#include <Maths/Matrix.h>
#include <Maths/Vector.h>

struct CameraData
{
    nyaMat4x4f  viewMatrix;
    nyaMat4x4f  projectionMatrix;
    nyaMat4x4f  inverseViewMatrix;
    nyaMat4x4f  inverseProjectionMatrix;
    nyaMat4x4f  viewProjectionMatrix;

    nyaMat4x4f  inverseViewProjectionMatrix;
    nyaVec3f    worldPosition;
    int32_t     cameraFrameNumber;

    // Shadow mapping rendering specifics
    nyaMat4x4f  depthProjectionMatrix;
    nyaMat4x4f  depthViewProjectionMatrix;
    nyaVec4f    cascadeOffsets[CSM_SLICE_COUNT];
    nyaVec4f    cascadeScales[CSM_SLICE_COUNT];
    float       cascadeSplitDistances[CSM_SLICE_COUNT];
    nyaMat4x4f  shadowViewMatrix[CSM_SLICE_COUNT];
    nyaMat4x4f  globalShadowMatrix;

    // Temporal infos
    nyaMat4x4f  previousViewProjectionMatrix;
    nyaMat4x4f  previousViewMatrix;
    nyaVec2f    jitteringOffset;
    nyaVec2f    previousJitteringOffset;

    Frustum     frustum;
};
