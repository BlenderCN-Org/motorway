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
struct PipelineState;

using ResHandle_t = uint32_t;

class RenderDevice;
class CommandList;
class RenderPipeline;
class ShaderCache;
class GraphicsAssetCache;

#include <Framework/Light.h>
#include <Shaders/Shared.h>
#include <Maths/Vector.h>

class LightGrid
{
public:
    struct SceneInfosBuffer {
        nyaVec3f ClustersScale;
        uint32_t __PADDING__;
        nyaVec3f ClustersInverseScale;
        uint32_t __PADDING2__;
        nyaVec3f ClustersBias;
        uint32_t __PADDING3__;
        nyaVec3f SceneAABBMin;
        uint32_t __PADDING4__;
        nyaVec3f SceneAABBMax;
        uint32_t __PADDING5__;
    };
    
    struct PassData {
        ResHandle_t lightsClusters;
        ResHandle_t lightsClustersInfosBuffer;
        ResHandle_t lightsBuffer;
    };

public:
                                    LightGrid( BaseAllocator* allocator );
                                    LightGrid( LightGrid& ) = default;
                                    LightGrid& operator = ( LightGrid& ) = default;
                                    ~LightGrid();

    void                            destroy( RenderDevice* renderDevice );

    PassData                        updateClusters( RenderPipeline* renderPipeline );
    void                            loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache );

    void                            setSceneBounds( const nyaVec3f& aabbMax, const nyaVec3f& aabbMin );
  
    PointLightData*                 allocatePointLightData( const PointLightData&& lightData );
    IBLProbeData*                   allocateLocalIBLProbeData( const IBLProbeData&& probeData );

    DirectionalLightData*           updateDirectionalLightData( const DirectionalLightData&& lightData );
    IBLProbeData*                   updateGlobalIBLProbeData( const IBLProbeData&& probeData );

    const DirectionalLightData*     getDirectionalLightData() const;
    const IBLProbeData*             getGlobalIBLProbeData() const;

private:
    BaseAllocator*                  memoryAllocator;

    PipelineState*                  lightCullingPso;
    SceneInfosBuffer                sceneInfosBuffer;

    uint32_t                        pointLightCount;
    uint32_t                        localIBLProbeCount;

    struct {
        PointLightData              PointLights[MAX_POINT_LIGHT_COUNT];
        IBLProbeData                IBLProbes[MAX_IBL_PROBE_COUNT];
        DirectionalLightData        DirectionalLight;
    } lights;

private:
    void                            updateClustersInfos();
};
