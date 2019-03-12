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

#include <Maths/Vector.h>
#include <Maths/Matrix.h>

// Punctual Lights
struct PointLightData
{
    nyaVec3f    worldPosition;
    float       radius;

    nyaVec3f    colorRGB;
    float       lightPower;
};

struct SpotLightData
{
    nyaVec3f   worldPosition;
    float       cutoff;

    nyaVec3f   colorRGB;
    float       lightPower;

    nyaVec3f   lightDirection;
    float       outerCutoff;

    float       radius;
    float       inverseCosCone;
    uint32_t    __PADDING__[2];
};

// Area Light
// NOTE Sphere Lights share the same data as a pointlight
struct DiskLightData
{
    nyaVec3f   worldPosition;
    float       radius;

    nyaVec3f   colorRGB;
    float       lightPower;

    nyaVec3f   planeNormal;
    int32_t     __PADDING__;
};

struct RectangleLightData
{
    nyaVec3f   worldPosition;
    float       radius;

    nyaVec3f   colorRGB;
    float       lightPower;

    nyaVec3f   planeNormal;
    float       width;

    nyaVec3f   upVector;
    float       height;

    nyaVec3f   leftVector;
    uint32_t    isTubeLight;
};

// Directional Light
struct DirectionalLightData
{
    nyaVec3f    colorRGB;
    float       angularRadius;

    nyaVec3f    direction;
    float       illuminanceInLux;

    nyaVec2f    sphericalCoordinates;

    float       intensityInLux;
    bool        isSunLight;
    bool        enableShadow;
    bool        isEnabled;
    bool        __PADDING__;
};

// IBL Probe
struct IBLProbeData
{
    nyaVec3f    worldPosition;
    float       radius;

    nyaMat4x4f  inverseModelMatrix;

    uint32_t    ProbeIndex; // Array index

    uint16_t    CaptureFrequency; // Defines the span of time during which the probe is captured and convoluted
    bool        isCaptured;
    bool        isDynamic;
    bool        isFallbackProbe; // Should be used as a fallback in case no env probe is available? (this flag discards probe's world position/radius)
    uint8_t     __PADDING__[4];
};
