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

#include <Graphics/DrawCommandBuilder.h>

#include <Maths/Transform.h>
#include <Framework/Cameras/FreeCamera.h>

Scene::Scene( BaseAllocator* allocator, const std::string& sceneName )
    : name( sceneName )
    , memoryAllocator( allocator )
    , staticGeometryCount( 0u )
{
    // TODO Test!!
    TransformDatabase.components = nya::core::allocateArray<Transform>( allocator, 1024 );
    TransformDatabase.capacity = 1024;

    RenderableMeshDatabase.components = nya::core::allocateArray<RenderableMesh>( allocator, 1024 );
    RenderableMeshDatabase.capacity = 1024;

    FreeCameraDatabase.components = nya::core::allocateArray<FreeCamera>( allocator, 4 );
    FreeCameraDatabase.capacity = 4;
}

Scene::~Scene()
{
    name.clear();

    nya::core::freeArray( memoryAllocator, TransformDatabase.components );
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
    for ( uint32_t staticGeomIdx = 0; staticGeomIdx < staticGeometryCount; staticGeomIdx++ ) {
        auto& geometry = staticGeometry[staticGeomIdx];

        auto& transform = TransformDatabase[geometry.transform];
        auto& renderable = RenderableMeshDatabase[geometry.mesh];

        // Check renderable flags (but don't cull the instance yet)
        if ( renderable.isVisible ) {
            drawCmdBuilder.addGeometryToRender( renderable.meshResource, transform.getWorldModelMatrix() );
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
