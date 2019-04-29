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
#include <Shaders/Shared.h>

NYA_ENV_VAR( DisplayDebugIBLProbe, true, bool ) // [Debug] Display IBL Probe as reflective Sphere in the scene [True/False]

Scene::Scene( BaseAllocator* allocator, const std::string& sceneName )
    : name( sceneName )
    , memoryAllocator( allocator )
{
    // TODO Test!!
    TransformDatabase.components = nya::core::allocateArray<Transform>( allocator, 8192 );
    TransformDatabase.capacity = 8192;

    RenderableMeshDatabase.components = nya::core::allocateArray<RenderableMesh>( allocator, 1024 );
    RenderableMeshDatabase.capacity = 1024;

    FreeCameraDatabase.components = nya::core::allocateArray<FreeCamera>( allocator, 4 );
    FreeCameraDatabase.capacity = 4;

    IBLProbeDatabase.components = nya::core::allocateArray<IBLProbe>( allocator, MAX_IBL_PROBE_COUNT );
    IBLProbeDatabase.capacity = MAX_IBL_PROBE_COUNT;
}

Scene::~Scene()
{
    name.clear();

    nya::core::freeArray( memoryAllocator, TransformDatabase.components );
    nya::core::freeArray( memoryAllocator, RenderableMeshDatabase.components );
    nya::core::freeArray( memoryAllocator, FreeCameraDatabase.components );
    nya::core::freeArray( memoryAllocator, IBLProbeDatabase.components );
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
    for ( uint32_t pointLightIdx = 0; pointLightIdx < PointLightDatabase.usageIndex; pointLightIdx++ ) {
        auto& light = PointLightDatabase[pointLightIdx];

        auto& transform = TransformDatabase[light.transform];

        if ( transform.needRebuild() ) {
            light.pointLightData->worldPosition = transform.getWorldTranslation();
        } else {
            transform.setWorldTranslation( light.pointLightData->worldPosition );
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
        for ( uint32_t iblProbeIdx = 0; iblProbeIdx < IBLProbeDatabase.usageIndex; iblProbeIdx++ ) {
            IBLProbeData* iblProbe = IBLProbeDatabase[iblProbeIdx].iblProbeData;
            
            // Skip global probe
            if ( iblProbe->isFallbackProbe ) {
                continue;
            }

            drawCmdBuilder.addSphereToRender( iblProbe->worldPosition, 1.50f, drawCmdBuilder.MaterialDebugIBLProbe );
        }
    }
#endif

    for ( uint32_t iblProbeIdx = 0; iblProbeIdx < IBLProbeDatabase.usageIndex; iblProbeIdx++ ) {
        IBLProbeData* iblProbe = IBLProbeDatabase[iblProbeIdx].iblProbeData;

        if ( !iblProbe->isCaptured ) {
            drawCmdBuilder.addIBLProbeToCapture( iblProbe );
            iblProbe->isCaptured = true;
        }
    }

    for ( uint32_t staticGeomIdx = 0; staticGeomIdx < RenderableMeshDatabase.usageIndex; staticGeomIdx++ ) {
        const RenderableMesh& geometry = RenderableMeshDatabase[staticGeomIdx];
        Transform& transform = TransformDatabase[geometry.transform];

        // Check renderable flags (but don't cull the instance yet)
        if ( geometry.isVisible ) {
            drawCmdBuilder.addGeometryToRender( geometry.meshResource, transform.getWorldModelMatrix(), geometry.flags );
        }
    }

    for ( uint32_t freeCameraIdx = 0; freeCameraIdx < FreeCameraDatabase.usageIndex; freeCameraIdx++ ) {
        drawCmdBuilder.addCamera( &FreeCameraDatabase[freeCameraIdx].getData() );
    }
}

Scene::Node* Scene::intersect( const Ray& ray )
{
    float bestIntersectionDist = std::numeric_limits<float>::max();
    Scene::Node* pickedNode = nullptr;

    for ( Node* node : sceneNodes ) {
        float intersectionDist = std::numeric_limits<float>::max();
        auto isIntersected = node->intersect( ray, intersectionDist );

        if ( isIntersected && intersectionDist >= 0.0f && intersectionDist < bestIntersectionDist ) {
            bestIntersectionDist = intersectionDist;
            pickedNode = node;
        }
    }

    return pickedNode;
}

Scene::StaticGeometryNode* Scene::allocateStaticGeometry()
{
    StaticGeometryNode* staticGeometryNode = nya::core::allocate<StaticGeometryNode>( memoryAllocator );
    staticGeometryNode->transform = TransformDatabase.allocate();
    staticGeometryNode->mesh = RenderableMeshDatabase.allocate();

    RenderableMeshDatabase[staticGeometryNode->mesh].transform = staticGeometryNode->transform;
    
    staticGeometryNode->meshResource = &RenderableMeshDatabase[staticGeometryNode->mesh].meshResource;

    sceneNodes.push_back( staticGeometryNode );

    return staticGeometryNode;
}

Scene::PointLightNode* Scene::allocatePointLight()
{
    PointLightNode* pointLightNode = nya::core::allocate<PointLightNode>( memoryAllocator );
    pointLightNode->transform = TransformDatabase.allocate();
    pointLightNode->pointLight = PointLightDatabase.allocate();

    PointLightDatabase[pointLightNode->pointLight].transform = pointLightNode->transform;

    pointLightNode->pointLightData = &PointLightDatabase[pointLightNode->pointLight].pointLightData;

    sceneNodes.push_back( pointLightNode );

    return pointLightNode;
}

Scene::IBLProbeNode* Scene::allocateIBLProbe()
{
    IBLProbeNode* iblProbeNode = nya::core::allocate<IBLProbeNode>( memoryAllocator );
    iblProbeNode->transform = TransformDatabase.allocate();
    iblProbeNode->iblProbe = IBLProbeDatabase.allocate();

    iblProbeNode->iblProbeData = &IBLProbeDatabase[iblProbeNode->iblProbe].iblProbeData;

    sceneNodes.push_back( iblProbeNode );

    return iblProbeNode;
}
