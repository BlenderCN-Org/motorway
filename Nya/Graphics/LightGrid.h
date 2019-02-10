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

struct Buffer;
struct Texture;

class RenderDevice;
class CommandList;

#include <Framework/Light.h>
#include <Shaders/Shared.h>
#include <Maths/Vector.h>

class LightGrid
{
public:
    struct ClustersInfos {
        nyaVec3f   Scale;
        nyaVec3f   Bias;
    };

public:
                            LightGrid( BaseAllocator* allocator );
                            LightGrid( LightGrid& ) = default;
                            LightGrid& operator = ( LightGrid& ) = default;
                            ~LightGrid();

    void                    create( RenderDevice* renderDevice );
    void                    destroy( RenderDevice* renderDevice );

    void                    updateClusters( CommandList* cmdList );

    void                    setSceneBounds( const nyaVec3f& aabbMax, const nyaVec3f& aabbMin );

    Buffer*                 getLightsBuffer() const;
    Texture*                getLightsClusters() const;
    const ClustersInfos&    getClustersInfos() const;

    DirectionalLightData*   allocateDirectionalLightData( const DirectionalLightData&& lightData );
    PointLightData*         allocatePointLightData( const PointLightData&& lightData );

private:
    BaseAllocator*          memoryAllocator;

    Buffer*                 lightsBuffer;
    Texture*                clustersTexture;
    ClustersInfos           clustersInfos;

    nyaVec3f                aabbMin;
    nyaVec3f                aabbMax;

    uint32_t                PointLightCount;
    uint32_t                DirectionalLightCount;

    struct {
        PointLightData          PointLights[MAX_POINT_LIGHT_COUNT];
        DirectionalLightData    DirectionalLights[MAX_DIRECTIONAL_LIGHT_COUNT];
    } lights;

private:
    void                        updateClustersInfos();
};
