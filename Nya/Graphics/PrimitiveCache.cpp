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
#include "PrimitiveCache.h"

#include <Maths/Primitives.h>
#include <Rendering/RenderDevice.h>

PrimitiveCache::PrimitiveCache( BaseAllocator* allocator )
    : memoryAllocator( allocator )
{

}

PrimitiveCache::~PrimitiveCache()
{

}

void PrimitiveCache::createPrimitivesBuffer( RenderDevice* renderDevice )
{
    PrimitiveData spherePrimitiveData = nya::maths::CreateSpherePrimitive();

    BufferDesc sphereVboDesc = {};
    sphereVboDesc.type = BufferDesc::VERTEX_BUFFER;
    sphereVboDesc.size = spherePrimitiveData.vertices.size() * sizeof( float );
    sphereVboDesc.stride = 8 * sizeof( float );

    BufferDesc sphereIboDesc = {};
    sphereIboDesc.type = BufferDesc::INDICE_BUFFER;
    sphereIboDesc.size = spherePrimitiveData.indices.size() * sizeof( uint32_t );
    sphereIboDesc.stride = sizeof( uint32_t );

    spherePrimitive.indiceCount = static_cast< uint32_t >( spherePrimitiveData.indices.size() );
    spherePrimitive.indiceBufferOffset = 0u;
    spherePrimitive.vertexBuffer = renderDevice->createBuffer( sphereVboDesc, spherePrimitiveData.vertices.data() );
    spherePrimitive.indiceBuffer = renderDevice->createBuffer( sphereIboDesc, spherePrimitiveData.indices.data() );


    PrimitiveData rectanglePrimitiveData = nya::maths::CreateQuadPrimitive();

    BufferDesc rectangleVboDesc = {};
    rectangleVboDesc.type = BufferDesc::VERTEX_BUFFER;
    rectangleVboDesc.size = rectanglePrimitiveData.vertices.size() * sizeof( float );
    rectangleVboDesc.stride = 8 * sizeof( float );

    BufferDesc rectangleIboDesc = {};
    rectangleIboDesc.type = BufferDesc::INDICE_BUFFER;
    rectangleIboDesc.size = rectanglePrimitiveData.indices.size() * sizeof( uint32_t );
    rectangleIboDesc.stride = sizeof( uint32_t );

    rectanglePrimitive.indiceCount = static_cast< uint32_t >( rectanglePrimitiveData.indices.size() );
    rectanglePrimitive.indiceBufferOffset = 0u;
    rectanglePrimitive.vertexBuffer = renderDevice->createBuffer( rectangleVboDesc, rectanglePrimitiveData.vertices.data() );
    rectanglePrimitive.indiceBuffer = renderDevice->createBuffer( rectangleIboDesc, rectanglePrimitiveData.indices.data() );
}

void PrimitiveCache::destroy( RenderDevice* renderDevice )
{
    renderDevice->destroyBuffer( spherePrimitive.vertexBuffer );
    renderDevice->destroyBuffer( spherePrimitive.indiceBuffer );

    renderDevice->destroyBuffer( rectanglePrimitive.vertexBuffer );
    renderDevice->destroyBuffer( rectanglePrimitive.indiceBuffer );
}

const PrimitiveCache::Primitive& PrimitiveCache::getSpherePrimitive() const
{
    return spherePrimitive;
}

const PrimitiveCache::Primitive& PrimitiveCache::getRectanglePrimitive() const
{
    return rectanglePrimitive;
}
