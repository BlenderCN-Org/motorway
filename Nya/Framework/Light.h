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

#include <glm/glm/glm.hpp>

// Punctual Lights
struct PointLightData
{
    glm::vec3   worldPosition;
    float       radius;

    glm::vec3   colorRGB;
    float       lightPower;
};

struct SpotLightData
{
    glm::vec3   worldPosition;
    float       cutoff;

    glm::vec3   colorRGB;
    float       lightPower;

    glm::vec3   lightDirection;
    float       outerCutoff;

    float       radius;
    float       inverseCosCone;
    uint32_t    __PADDING__[2];
};

// Area Light
// NOTE Sphere Lights share the same data as a pointlight
struct DiskLightData
{
    glm::vec3   worldPosition;
    float       radius;

    glm::vec3   colorRGB;
    float       lightPower;

    glm::vec3   planeNormal;
    int32_t     __PADDING__;
};

struct RectangleLightData
{
    glm::vec3   worldPosition;
    float       radius;

    glm::vec3   colorRGB;
    float       lightPower;

    glm::vec3   planeNormal;
    float       width;

    glm::vec3   upVector;
    float       height;

    glm::vec3   leftVector;
    uint32_t    isTubeLight;
};

// Directional Light
struct DirectionalLightData
{
    glm::vec3   colorRGB;
    float       angularRadius;

    glm::vec3   direction;
    float       illuminanceInLux;

    glm::vec2   sphericalCoordinates;
    float       intensityInLux;
    bool        isSunLight;
    bool        enableShadow;
    bool        __PADDING__[2];
};

// Environment Probe
struct EnvironmentProbeData
{
    glm::vec3   worldPosition;
    float       radius;

    glm::mat4   inverseModelMatrix;

    bool        isCaptured;
    bool        isDynamic;
    bool        isFallbackProbe; // Should be used as a fallback in case no env probe is available? (this flag discards probe's world position/radius)
    uint8_t     __PADDING__;

    //uint16_t        CaptureFrequency; // Defines the span of time during which the probe is captured and convoluted
    //uint16_t        __PADDING2__; // Defines the span of time during which the probe is captured and convoluted

    //uint32_t        __PADDING_3__[3] = { ( unsigned int )~0, ( unsigned int )0, ( unsigned int )~0 };
    //uint32_t        ProbeIndex; // Array index 
};
