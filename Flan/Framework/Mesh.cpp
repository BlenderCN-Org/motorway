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

#include "Shared.h"
#include "Mesh.h"

#include <Rendering/RenderDevice.h>
#include <Rendering/VertexArrayObject.h>

Mesh::Mesh( const fnString_t& meshName )
    : name( meshName )
    , aabb{}
    , vertexArrayObject( nullptr )
{
    subMeshes.reserve( 64 );
}

Mesh::~Mesh()
{
    name.clear();
    aabb = {};
}

void Mesh::create( RenderDevice* renderDevice, const BufferDesc& vertexBufferDesc, const BufferDesc& indiceBufferDesc, const float* vertexBufferContent, const uint32_t* indiceBufferContent )
{
    vertexArrayObject.reset( new VertexArrayObject() );

    vertexBuffer.reset( new Buffer() );
    vertexBuffer->create( renderDevice, vertexBufferDesc, (void*)vertexBufferContent );

    indiceBuffer.reset( new Buffer() );
    indiceBuffer->create( renderDevice, indiceBufferDesc, (void*)indiceBufferContent );

    vertexArrayObject->create( renderDevice, vertexBuffer.get(), indiceBuffer.get() );

    // TODO Data oriented layout (write the layout in the .mesh file; build the layout object at runtime)
    VertexLayout_t defaultMeshLayout = {
        { 0, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 0 }, // POSITION
        { 1, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 3 * sizeof( float ) }, // NORMAL
        { 2, VertexLayoutEntry::DIMENSION_XY, VertexLayoutEntry::FORMAT_FLOAT, 6 * sizeof( float ) }, // UVMAP0
        { 3, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 8 * sizeof( float ) }, // TANGENT
        { 4, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 11 * sizeof( float ) }, // BINORMAL
    };

    vertexArrayObject->setVertexLayout( renderDevice, defaultMeshLayout );

    // Implicitly resets bounding box
    aabb.maxPoint = aabb.maxPoint = glm::vec3( 0, 0, 0 );
}

void Mesh::addSubMesh( SubMesh&& subMeshData )
{
    subMeshes.push_back( std::move( subMeshData ) );

    flan::core::ExpandAABB( aabb, subMeshes.back().aabb );
}

const AABB& Mesh::getAABB() const
{
    return aabb;
}

BoundingSphere Mesh::getBoundingSphere() const
{
    auto sphereCenter = flan::core::GetAABBCentroid( aabb );
    auto sphereRadius = flan::core::GetAABBHalfExtents( aabb )[flan::core::GetMaxDimensionAxisAABB( aabb )];

    BoundingSphere sphere;
    flan::core::CreateSphere( sphere, sphereCenter, sphereRadius );
    return sphere;
}

fnStringHash_t Mesh::getHashcode() const
{
    return flan::core::CRC32( name );
}

const VertexArrayObject* Mesh::getVertexArrayObject() const
{
    return vertexArrayObject.get();
}

void Mesh::setName( const fnString_t& meshName )
{
    name = meshName;
}

const fnString_t& Mesh::getName() const
{
    return name;
}

const std::vector<SubMesh>& Mesh::getSubMeshVector() const
{
    return subMeshes;
}

std::vector<SubMesh>& Mesh::getSubMeshVectorRW()
{
    return subMeshes;
}

void Mesh::reset()
{
    name.clear();
    aabb.maxPoint = aabb.minPoint = glm::vec3( 0, 0, 0 );
    subMeshes.clear();

    flags.canBeCulled = 0;
    flags.castShadows = 0;
}
