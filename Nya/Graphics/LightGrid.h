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

class LightGrid
{
public:
                        LightGrid( BaseAllocator* allocator );
                        LightGrid( LightGrid& ) = default;
                        LightGrid& operator = ( LightGrid& ) = default;
                        ~LightGrid();

    void                create( RenderDevice* renderDevice );
    void                updateClusters( CommandList* cmdList );

    void                setSceneBounds( const glm::vec3& aabbMax, const glm::vec3& aabbMin );

    DirectionalLight&   allocateDirectionalLight( const DirectionalLightData&& lightData );
    PointLight&         allocatePointLight( const PointLightData&& lightData );
    SpotLight&          allocateSpotLight( const SpotLightData&& lightData );

    EnvironmentProbe&   allocateLocalEnvironmentProbe( const EnvironmentProbeData&& lightData );
    EnvironmentProbe&   allocateGlobalEnvironmentProbe( const EnvironmentProbeData&& lightData );

private:
    BaseAllocator*      memoryAllocator;

    Buffer*             lightsBuffer;
    Texture*            clustersTexture;

    glm::vec3           aabbMin;
    glm::vec3           aabbMax;

    struct {
        uint32_t                DirectionalLightCount;
        uint32_t                PointLightCount;
        uint32_t                SpotLightCount;
        uint32_t                SphereLightCount;

        uint32_t                DiskLightCount;
        uint32_t                RectangleLightCount;
        uint32_t                LocalEnvironmentProbeCount;
        uint32_t                GlobalEnvironmentProbeCount;

        DirectionalLightData    DirectionalLights[MAX_DIRECTIONAL_LIGHT_COUNT];
        PointLightData          PointLights[MAX_POINT_LIGHT_COUNT];
        SpotLightData           SpotLights[MAX_SPOT_LIGHT_COUNT];
        PointLightData          SphereLights[MAX_SPHERE_LIGHT_COUNT];
        DiskLightData           DiskLights[MAX_DISC_LIGHT_COUNT];
        RectangleLightData      RectangleLights[MAX_RECTANGLE_LIGHT_COUNT];

        EnvironmentProbe        GlobalEnvironmentProbes[MAX_GLOBAL_ENVIRONMENT_PROBE_COUNT];
        EnvironmentProbe        LocalEnvironmentProbes[MAX_LOCAL_ENVIRONMENT_PROBE_COUNT];
    } lights;
};
