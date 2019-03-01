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
#include <Maths/Helpers.h>

Mesh::Mesh( const nyaString_t& meshName )
    : name( meshName )
    , vertexBuffer( nullptr )
    , indiceBuffer( nullptr )
    , lodCount( 0 )
{
    for ( int lodIdx = 0; lodIdx < MAX_LOD_COUNT; lodIdx++ ) {
        lod[lodIdx].startDistance = -1.0f;
    }
}

Mesh::~Mesh()
{
    name.clear();
    aabb = {};
}

void Mesh::create( RenderDevice* renderDevice, const BufferDesc& vertexBufferDesc, const BufferDesc& indiceBufferDesc, const float* vertexBufferContent, const uint32_t* indiceBufferContent )
{
    vertexBuffer = renderDevice->createBuffer( vertexBufferDesc, vertexBufferContent );
    indiceBuffer = renderDevice->createBuffer( indiceBufferDesc, indiceBufferContent );

    // Implicitly resets bounding box
    aabb.minPoint = aabb.maxPoint = nyaVec3f( 0, 0, 0 );
}

void Mesh::destroy( RenderDevice* renderDevice )
{
    renderDevice->destroyBuffer( vertexBuffer );
    renderDevice->destroyBuffer( indiceBuffer );
}

void Mesh::addLevelOfDetail( const uint32_t lodIndex, const float lodDistance )
{
    lod[lodIndex].startDistance = ( lodIndex > 0 ) ? lod[lodIndex - 1].lodDistance : 0.0f;
    lod[lodIndex].lodDistance = lodDistance;
    lod[lodIndex].lodIndex = lodIndex;
    lod[lodIndex].subMeshes.clear();

    lodCount++;
}

void Mesh::addSubMesh( const uint32_t lodIndex, SubMesh&& subMeshData )
{
    nya::core::ExpandAABB( aabb, subMeshData.aabb );

    lod[lodIndex].subMeshes.push_back( std::move( subMeshData ) );
}

nyaStringHash_t Mesh::getHashcode() const
{
    return nya::core::CRC32( name );
}

void Mesh::setName( const nyaString_t& meshName )
{
    name = meshName;
}

const nyaString_t& Mesh::getName() const
{
    return name;
}

const Mesh::LevelOfDetail& Mesh::getLevelOfDetail( const float distance ) const
{
    for ( int lodIdx = 0; lodIdx < lodCount; lodIdx++ ) {
        if ( lod[lodIdx].lodDistance >= distance ) {
            return lod[lodIdx];
        }
    }

    return lod[lodCount - 1];
}

const Mesh::LevelOfDetail& Mesh::getLevelOfDetailByIndex( const uint32_t lodIndex ) const
{
    return lod[nya::maths::min( ( MAX_LOD_COUNT - 1u ), lodIndex )];
}

const int Mesh::getLevelOfDetailCount() const
{
    return lodCount;
}

void Mesh::reset()
{
    name.clear();
    aabb.maxPoint = aabb.minPoint = nyaVec3f( 0, 0, 0 );
    lodCount = 0;
}

const Buffer* Mesh::getVertexBuffer() const
{
    return vertexBuffer;
}

const Buffer* Mesh::getIndiceBuffer() const
{
    return indiceBuffer;
}

const AABB& Mesh::getMeshAABB() const
{
    return aabb;
}
