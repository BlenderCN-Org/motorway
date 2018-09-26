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

#include <Framework/RenderKey.h>

class FileSystemObject;

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
    glm::vec2   __PADDING__;
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
    bool        __PLACEHOLDER__[2];
};

template<typename Data>
class Light
{
public:
                            Light<Data>();
                            Light<Data>( Data&& lightData );
                            Light<Data>( FileSystemObject* stream );
                            Light<Data>( Light& light );
                            ~Light();

    void                    restore( FileSystemObject* stream );
    void                    save( FileSystemObject* stream );

    void                    setRenderKey( const fnRenderKey_t newRenderKey );
    const fnRenderKey_t&    getRenderKey() const;

    Data&                   getLightData();
    const Data*             getLightData() const;

private:
    Data                    data;
    bool                    isGPUDirty;
    fnRenderKey_t           renderKey;
};

using PointLight          = Light<PointLightData>;
using SpotLight           = Light<SpotLightData>;

using DiskLight           = Light<DiskLightData>;
using SphereLight         = Light<PointLightData>;
using RectangleLight      = Light<RectangleLightData>;
using TubeLight           = Light<RectangleLightData>;

using DirectionalLight    = Light<DirectionalLightData>;

#include "Light.inl"
