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

#include <Rendering/RenderDevice.h>

#include <Framework/Light.h>
#include <Framework/RenderKey.h>
#include "EnvironmentProbe.h"

#include <Graphics/RenderPass.h>
#include <Shaders/Shared.h>

#include <queue>

class Buffer;
class RenderPipeline;

#define FLAN_IMPLEMENT_ENTITY_STORAGE( type, name, enumType )\
private:\
    bool name##NeedRebuild;\
    bool name##sUsed[MAX_##enumType##_COUNT];\
    type name##s[MAX_##enumType##_COUNT];

#define FLAN_IMPLEMENT_RENDERABLE_ENTITY( type, enumType )\
FLAN_IMPLEMENT_ENTITY_STORAGE( type, type, enumType );\
public:\
    auto get##type##ByIndex( const int index ) -> type*\
    {\
        return ( index >= MAX_##enumType##_COUNT ) ? nullptr : &type##s[index];\
    }\
    auto create##type( type&& type##Args ) -> type*\
    {\
        if ( renderableEntities.type##Count >= MAX_##enumType##_COUNT ) {\
            FLAN_CERR << "Too many " << #type << " in the buffer!" << std::endl;\
            return nullptr;\
        }\
    \
        int cpuIndex = 0;\
        for ( ; cpuIndex < MAX_##enumType##_COUNT; cpuIndex++ ) {\
            if ( !type##sUsed[cpuIndex] ) {\
                type##sUsed[cpuIndex] = true;\
                break;\
            }\
        }\
        type##s[cpuIndex] = std::move( type##Args );\
        auto& entity = type##s[cpuIndex];\
        entity.setRenderKey( flan::framework::BuildRenderKey( flan::framework::RENDERABLE_TYPE_##enumType, cpuIndex ) );\
        renderableEntities.type##Count++;\
        return &entity;\
    }

class RenderableEntityManager
{
FLAN_IMPLEMENT_RENDERABLE_ENTITY( DirectionalLight, DIRECTIONAL_LIGHT )
FLAN_IMPLEMENT_RENDERABLE_ENTITY( PointLight, POINT_LIGHT )
FLAN_IMPLEMENT_RENDERABLE_ENTITY( SpotLight, SPOT_LIGHT )
FLAN_IMPLEMENT_RENDERABLE_ENTITY( SphereLight, SPHERE_LIGHT )
FLAN_IMPLEMENT_RENDERABLE_ENTITY( DiskLight, DISC_LIGHT )
FLAN_IMPLEMENT_RENDERABLE_ENTITY( RectangleLight, RECTANGLE_LIGHT )

// Special case, since we need to distinguish global from local environment probes
FLAN_IMPLEMENT_ENTITY_STORAGE( EnvironmentProbe, GlobalEnvironmentProbe, GLOBAL_ENVIRONMENT_PROBE )
FLAN_IMPLEMENT_ENTITY_STORAGE( EnvironmentProbe, LocalEnvironmentProbe, LOCAL_ENVIRONMENT_PROBE )

public:
    inline EnvironmentProbe*    GetLocalEnvironmentProbeByIndex( const uint32_t index ) { return &LocalEnvironmentProbes[index]; }
    inline EnvironmentProbe*    GetGlobalEnvironmentProbeByIndex( const uint32_t index ) { return &GlobalEnvironmentProbes[index]; }

    struct EntityBuffer
    {
        Buffer* buffer;
    };

public:
                                RenderableEntityManager();
                                RenderableEntityManager( RenderableEntityManager& ) = delete;
                                ~RenderableEntityManager() = default;

    void                        create( RenderDevice* renderDevice );
    void                        rebuildBuffer( RenderDevice* renderDevice );
    void                        clear();

    void                        updateEntity( const fnRenderKey_t renderKey );
    void                        removeEntity( const fnRenderKey_t renderKey );

    // Since environment probe involve some trickery (share the same array, local indexes have an offset, ...), those should
    // be kept hand-written
    EnvironmentProbe*           createLocalEnvironmentProbe();
    EnvironmentProbe*           createGlobalEnvironmentProbe();

private:
    bool                        needReupload;

    std::unique_ptr<Buffer>     renderableEntitiesBuffer;
    EntityBuffer entityBuffer;

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
    } renderableEntities;

private:
    fnPipelineMutableResHandle_t addLightCullingPass( RenderPipeline* renderPipeline, bool enableMSAA = false );
};

#undef FLAN_IMPLEMENT_RENDERABLE_ENTITY
#undef FLAN_IMPLEMENT_ENTITY_STORAGE
