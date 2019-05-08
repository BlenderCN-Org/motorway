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
NYA_ENV_VAR( DisplayGeometryAABB, true, bool ) // [Debug] Display Static Geometry AABB as wireframe boundingbox in the scene [True/False]

Scene::Scene( BaseAllocator* allocator, const std::string& sceneName )
    : name( sceneName )
    , memoryAllocator( allocator )
    , sceneAabb()
{
    nya::maths::CreateAABB( sceneAabb, nyaVec3f( 0.0f ), nyaVec3f( 0.0f ) );

    // TODO Test!!
    TransformDatabase.components = nya::core::allocateArray<Transform>( allocator, 8192 );
    TransformDatabase.capacity = 8192;

    RenderableMeshDatabase.components = nya::core::allocateArray<RenderableMesh>( allocator, 1024 );
    RenderableMeshDatabase.capacity = 1024;

    FreeCameraDatabase.components = nya::core::allocateArray<FreeCamera>( allocator, 4 );
    FreeCameraDatabase.capacity = 4;

    IBLProbeDatabase.components = nya::core::allocateArray<IBLProbe>( allocator, MAX_IBL_PROBE_COUNT );
    IBLProbeDatabase.capacity = MAX_IBL_PROBE_COUNT;

    PointLightDatabase.components = nya::core::allocateArray<PointLight>( allocator, MAX_POINT_LIGHT_COUNT );
    PointLightDatabase.capacity = MAX_POINT_LIGHT_COUNT;
}

Scene::~Scene()
{
    name.clear();

    nya::core::freeArray( memoryAllocator, TransformDatabase.components );
    nya::core::freeArray( memoryAllocator, RenderableMeshDatabase.components );
    nya::core::freeArray( memoryAllocator, FreeCameraDatabase.components );
    nya::core::freeArray( memoryAllocator, IBLProbeDatabase.components );
    nya::core::freeArray( memoryAllocator, PointLightDatabase.components );
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

        light.pointLightData->worldPosition = transform.getLocalTranslation();
    }

    for ( uint32_t iblProbeIdx = 0; iblProbeIdx < IBLProbeDatabase.usageIndex; iblProbeIdx++ ) {
        auto& probe = IBLProbeDatabase[iblProbeIdx];
        auto& transform = TransformDatabase[probe.transform];
        
        if ( transform.needRebuild() )
            probe.iblProbeData->worldPosition = transform.getLocalTranslation();
    }

    // Update transforms
    for ( uint32_t transformIdx = 0; transformIdx < TransformDatabase.usageIndex; transformIdx++ ) {
        TransformDatabase[transformIdx].rebuildModelMatrix();
    }

    // Update FreeCameras
    for ( uint32_t freeCameraIdx = 0; freeCameraIdx < FreeCameraDatabase.usageIndex; freeCameraIdx++ ) {
        FreeCameraDatabase[freeCameraIdx].update( deltaTime );
    }
}

void Scene::collectDrawCmds( DrawCommandBuilder& drawCmdBuilder )
{
    NYA_PROFILE_FUNCTION

    sceneAabb.minPoint = nyaVec3f::Max;
    sceneAabb.maxPoint = -nyaVec3f::Max;

    for ( uint32_t iblProbeIdx = 0; iblProbeIdx < IBLProbeDatabase.usageIndex; iblProbeIdx++ ) {
        auto& iblProbe = IBLProbeDatabase[iblProbeIdx];
        IBLProbeData* iblProbeData = iblProbe.iblProbeData;

        if ( !iblProbeData->isCaptured ) {
            drawCmdBuilder.addIBLProbeToCapture( iblProbeData );
            iblProbeData->isCaptured = true;
        }

        nya::maths::ExpandAABB( sceneAabb, iblProbeData->worldPosition );

        nyaMat4x4f* modelMatrix = TransformDatabase[iblProbe.transform].getWorldModelMatrix();
        iblProbeData->inverseModelMatrix = ( *modelMatrix ).transpose().inverse();
    }

    for ( uint32_t staticGeomIdx = 0; staticGeomIdx < RenderableMeshDatabase.usageIndex; staticGeomIdx++ ) {
        RenderableMesh& geometry = RenderableMeshDatabase[staticGeomIdx];

        // Check renderable flags (but don't cull the instance yet)
        if ( geometry.isVisible ) {
            Transform& transform = TransformDatabase[geometry.transform];

            drawCmdBuilder.addGeometryToRender( geometry.meshResource, transform.getWorldModelMatrix(), geometry.flags );

            const AABB& meshAABB = geometry.meshResource->getMeshAABB();

            geometry.meshBoundingBox.minPoint = meshAABB.minPoint * transform.getWorldScale() + transform.getWorldTranslation();
            geometry.meshBoundingBox.maxPoint = meshAABB.maxPoint * transform.getWorldScale() + transform.getWorldTranslation();

            nya::maths::ExpandAABB( sceneAabb, geometry.meshBoundingBox );
        }
    }

    for ( uint32_t freeCameraIdx = 0; freeCameraIdx < FreeCameraDatabase.usageIndex; freeCameraIdx++ ) {
        drawCmdBuilder.addCamera( &FreeCameraDatabase[freeCameraIdx].getData() );
    }

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

    if ( DisplayGeometryAABB ) {
        for ( uint32_t staticGeomIdx = 0; staticGeomIdx < RenderableMeshDatabase.usageIndex; staticGeomIdx++ ) {
            RenderableMesh& geometry = RenderableMeshDatabase[staticGeomIdx];

            if ( geometry.isVisible ) {
                drawCmdBuilder.addAABBToRender( geometry.meshBoundingBox, drawCmdBuilder.MaterialDebugWireframe );
            }
        }
    }
#endif

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
    staticGeometryNode->worldTransform = &TransformDatabase[staticGeometryNode->transform];
    staticGeometryNode->mesh = RenderableMeshDatabase.allocate();

    RenderableMeshDatabase[staticGeometryNode->mesh].transform = staticGeometryNode->transform;
    
    staticGeometryNode->renderableMesh = &RenderableMeshDatabase[staticGeometryNode->mesh];

    sceneNodes.push_back( staticGeometryNode );

    return staticGeometryNode;
}

Scene::PointLightNode* Scene::allocatePointLight()
{
    PointLightNode* pointLightNode = nya::core::allocate<PointLightNode>( memoryAllocator );
    pointLightNode->transform = TransformDatabase.allocate();
    pointLightNode->worldTransform = &TransformDatabase[pointLightNode->transform];
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

    iblProbeNode->worldTransform = &TransformDatabase[iblProbeNode->transform];
    iblProbeNode->iblProbeData = &IBLProbeDatabase[iblProbeNode->iblProbe].iblProbeData;

    IBLProbeDatabase[iblProbeNode->iblProbe].transform = iblProbeNode->transform;

    sceneNodes.push_back( iblProbeNode );

    return iblProbeNode;
}

Scene::DirectionalLightNode* Scene::allocateDirectionalLight()
{
    DirectionalLightNode* dirLightNode = nya::core::allocate<DirectionalLightNode>( memoryAllocator );
    dirLightNode->transform = TransformDatabase.allocate();
    dirLightNode->worldTransform = &TransformDatabase[dirLightNode->transform];

    sceneNodes.push_back( dirLightNode );

    return dirLightNode;
}

const std::vector<Scene::Node*>& Scene::getNodes() const
{
    return sceneNodes;
}

const AABB& Scene::getSceneAabb() const
{
    return sceneAabb;
}
