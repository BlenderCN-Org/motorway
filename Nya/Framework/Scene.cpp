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
#include <Shared.h>
#include "Scene.h"

#include <Maths/Transform.h>

#include <Graphics/DrawCommandBuilder.h>

#include "Cameras/FreeCamera.h"
#include "Light.h"

#include <Core/EnvVarsRegister.h>

NYA_ENV_VAR( DisplayDebugIBLProbe, true, bool ) // [Debug] Display IBL Probe as reflective Sphere in the scene [True/False]

Scene::Scene( BaseAllocator* allocator, const std::string& sceneName )
    : name( sceneName )
    , memoryAllocator( allocator )
    , staticGeometryCount( 0u )
    , pointLightCount( 0u )
    , spotLightCount( 0u )
    , dirLightCount( 0u )
    , iblProbeCount( 0u )
{
    // TODO Test!!
    TransformDatabase.components = nya::core::allocateArray<Transform>( allocator, 8192 );
    TransformDatabase.capacity = 8192;

    RenderableMeshDatabase.components = nya::core::allocateArray<RenderableMesh>( allocator, 1024 );
    RenderableMeshDatabase.capacity = 1024;

    FreeCameraDatabase.components = nya::core::allocateArray<FreeCamera>( allocator, 4 );
    FreeCameraDatabase.capacity = 4;
}

Scene::~Scene()
{
    name.clear();

    nya::core::freeArray( memoryAllocator, TransformDatabase.components );
    nya::core::freeArray( memoryAllocator, RenderableMeshDatabase.components );
    nya::core::freeArray( memoryAllocator, FreeCameraDatabase.components );
}

void Scene::setSceneName( const std::string& sceneName )
{
    name = sceneName;
}

const std::string& Scene::getSceneName() const
{
    return name;
}

void Scene::updateLogic( const float deltaTime )
{
    // Propagate light transform prior to transform update
    for ( uint32_t pointLightIdx = 0; pointLightIdx < pointLightCount; pointLightIdx++ ) {
        auto& light = pointLight[pointLightIdx];

        auto& transform = TransformDatabase[light.transform];

        if ( transform.needRebuild() ) {
            light.pointLight->worldPosition = transform.getWorldTranslation();
        } else {
            transform.setWorldTranslation( light.pointLight->worldPosition );
        }
    }

    for ( uint32_t spotLightIdx = 0; spotLightIdx < spotLightCount; spotLightIdx++ ) {
        auto& light = spotLight[spotLightIdx];

        auto& transform = TransformDatabase[light.transform];

        if ( transform.needRebuild() ) {
            light.spotLight->worldPosition = transform.getWorldTranslation();
        } else {
            transform.setWorldTranslation( light.spotLight->worldPosition );
        }
    }

    // Update transforms
    for ( uint32_t transformIdx = 0; transformIdx < TransformDatabase.usageIndex; transformIdx++ ) {
        auto& transform = TransformDatabase[transformIdx];
        transform.rebuildModelMatrix();
    }

    // Update FreeCameras
    for ( uint32_t freeCameraIdx = 0; freeCameraIdx < FreeCameraDatabase.usageIndex; freeCameraIdx++ ) {
        auto& freeCamera = FreeCameraDatabase[freeCameraIdx];
        freeCamera.update( deltaTime );
    }
}

void Scene::collectDrawCmds( DrawCommandBuilder& drawCmdBuilder )
{
    NYA_PROFILE_FUNCTION

#if NYA_DEVBUILD
    if ( DisplayDebugIBLProbe ) {
        for ( uint32_t iblProbeIdx = 0; iblProbeIdx < iblProbeCount; iblProbeIdx++ ) {
            auto& iblProbe = iblProbes[iblProbeIdx].iblProbe;
            
            // Skip global probe
            if ( iblProbe->isFallbackProbe ) {
                continue;
            }

            drawCmdBuilder.addSphereToRender( iblProbe->worldPosition, 1.50f, drawCmdBuilder.MaterialDebugIBLProbe );
        }
    }
#endif

    for ( uint32_t iblProbeIdx = 0; iblProbeIdx < iblProbeCount; iblProbeIdx++ ) {
        auto& iblProbe = iblProbes[iblProbeIdx].iblProbe;

        if ( !iblProbe->isCaptured ) {
            drawCmdBuilder.addIBLProbeToCapture( iblProbe );
            iblProbe->isCaptured = true;
        }
    }

    for ( uint32_t staticGeomIdx = 0; staticGeomIdx < staticGeometryCount; staticGeomIdx++ ) {
        auto& geometry = staticGeometry[staticGeomIdx];

        auto& transform = TransformDatabase[geometry.transform];
        auto& renderable = RenderableMeshDatabase[geometry.mesh];

        // Check renderable flags (but don't cull the instance yet)
        if ( renderable.isVisible ) {
            drawCmdBuilder.addGeometryToRender( renderable.meshResource, transform.getWorldModelMatrix(), renderable.flags );
        }
    }

    for ( uint32_t freeCameraIdx = 0; freeCameraIdx < FreeCameraDatabase.usageIndex; freeCameraIdx++ ) {
        drawCmdBuilder.addCamera( &FreeCameraDatabase[freeCameraIdx].getData() );
    }
}

Scene::StaticGeometry& Scene::allocateStaticGeometry()
{
    auto& geom = staticGeometry[staticGeometryCount++];
    geom.transform = TransformDatabase.allocate();
    geom.mesh = RenderableMeshDatabase.allocate();

    return geom;
}

Scene::Light& Scene::allocatePointLight()
{
    auto& light = pointLight[pointLightCount++];
    light.transform = TransformDatabase.allocate();
    light.pointLight = nullptr;

    return light;
}

Scene::Light& Scene::allocateSpotLight()
{
    auto& light = spotLight[spotLightCount++];
    light.transform = TransformDatabase.allocate();
    light.spotLight = nullptr;

    return light;
}

Scene::Light& Scene::allocateDirectionalLight()
{
    auto& light = dirLight[dirLightCount++];
    light.transform = TransformDatabase.allocate();
    light.directionalLight = nullptr;

    return light;
}

Scene::Light& Scene::allocateIBLProbe()
{
    auto& light = iblProbes[iblProbeCount++];
    light.transform = TransformDatabase.allocate();
    light.iblProbe = nullptr;

    return light;
}
