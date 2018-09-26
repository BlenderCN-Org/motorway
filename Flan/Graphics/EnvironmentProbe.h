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

#include <Core/Maths/BoundingSphere.h>

struct EnvironmentProbe
{
    BoundingSphere  Sphere; // TODO Support multiple bouding primitives?
    glm::mat4       InverseModelMatrix;

    uint64_t        RenderKey;

    bool            IsCaptured;
    bool            IsDynamic;
    bool            IsFallbackProbe; // Should be used as a fallback in case no env probe is available? (this flag discards probe's world position/radius)
    uint8_t         __PADDING__;

    uint16_t        CaptureFrequency; // Defines the span of time during which the probe is captured and convoluted
    uint16_t        __PADDING2__; // Defines the span of time during which the probe is captured and convoluted

    uint32_t        __PADDING_3__[3] = { ( unsigned int )~0, ( unsigned int )0, ( unsigned int )~0 };
    uint32_t        ProbeIndex; // Array index 
};
