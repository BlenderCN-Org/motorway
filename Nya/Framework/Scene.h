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

class Transform;
class Mesh;
class FreeCamera;

struct PointLightData;
struct SpotLightData;
struct DirectionalLightData;
struct EnvironmentProbeData;

struct RenderableMesh
{
    Mesh*   meshResource;

    union {
        struct {
            uint8_t isVisible : 1;
            uint8_t renderDepth : 1;
            uint8_t useBatching : 1;
            uint8_t : 0;
        };

        uint32_t    flags;
    };

    RenderableMesh()
    {
        isVisible = 1;
    }
};

class Scene
{
public:
    using nyaComponentHandle_t = uint32_t;

private:
    template<typename T>
    struct ComponentDatabase {
        T*                      components;
        size_t                  capacity;
        nyaComponentHandle_t    usageIndex;

        ComponentDatabase()
            : components( nullptr )
            , capacity( 0ull )
            , usageIndex( 0u ) {

        }

        nyaComponentHandle_t allocate() {
            return ( usageIndex++ % capacity );
        }

        T& operator [] ( const nyaComponentHandle_t handle ) {
            return components[handle];
        }

        T& operator [] ( const nyaComponentHandle_t handle ) const {
            return components[handle];
        }
    };

public:
    struct Light {
        nyaComponentHandle_t    transform;

        // NOTE Light data should be managed by the Graphics subsystem
        union {
            PointLightData*         pointLight;
            SpotLightData*          spotLight;
            DirectionalLightData*   directionalLight;
            EnvironmentProbeData*   environmentProbe;
        };
    };

    struct StaticGeometry {
        nyaComponentHandle_t    transform;
        nyaComponentHandle_t    mesh;
    };

    struct GameWorldState {
        ComponentDatabase<Transform>        TransformDatabase;
        ComponentDatabase<RenderableMesh>   RenderableMeshDatabase;
        ComponentDatabase<FreeCamera>       FreeCameraDatabase;
        ComponentDatabase<Light>            LightDatabase;

        StaticGeometry                      StaticGeometry[4096];
        uint32_t                            StaticGeometryCount;
    };

    ComponentDatabase<Transform>        TransformDatabase;
    ComponentDatabase<RenderableMesh>   RenderableMeshDatabase;
    ComponentDatabase<FreeCamera>       FreeCameraDatabase;
    ComponentDatabase<Light>            LightDatabase;

public:
                            Scene( BaseAllocator* allocator, const std::string& sceneName = "Default Scene" );
                            Scene( Scene& scene ) = default;
                            Scene& operator = ( Scene& scene ) = default;
                            ~Scene();

    void                    setSceneName( const std::string& sceneName );
    const std::string&      getSceneName() const;

    void                    updateLogic( const float deltaTime );
    void                    getWorldStateSnapshot( GameWorldState& worldState );

    StaticGeometry&         allocateStaticGeometry();

    Light&                  allocatePointLight();
    Light&                  allocateSpotLight();
    Light&                  allocateDirectionalLight();

private:
    std::string             name;
    BaseAllocator*          memoryAllocator;

    StaticGeometry          staticGeometry[4096];
    uint32_t                staticGeometryCount;

    Light                   pointLight[64];
    uint32_t                pointLightCount;

    Light                   spotLight[64];
    uint32_t                spotLightCount;

    Light                   dirLight[1];
    uint32_t                dirLightCount;
};
