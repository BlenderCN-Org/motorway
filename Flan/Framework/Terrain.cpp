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
#include "Terrain.h"

#include <Rendering/RenderDevice.h>
#include <Rendering/VertexArrayObject.h>

Terrain::Terrain( const fnString_t& TerrainName )
    : name( TerrainName )
    , material( nullptr )
    , vertexBuffer( nullptr )
    , indiceBuffer( nullptr )
    , vertexArrayObject( new VertexArrayObject() )
{

}

Terrain::~Terrain()
{
    name.clear();
    material = nullptr;
}


void Terrain::create( RenderDevice* renderDevice, Material* terrainMaterial )
{
    constexpr float width = 512.0f;
    constexpr float height = 512.0f;
    constexpr float tileRes = 16.0f;
    constexpr float texelScale = 1024.0f / width;

    constexpr float halfWidth = ( ( float )width - 1.0f ) / 2.0f;
    constexpr float halfLength = ( ( float )height - 1.0f ) / 2.0f;

    // TODO Pool Grid and instantiate it per heightmap drawcall?
    struct VertexLayout
    {
        glm::vec3 positionWorldSpace;
        glm::vec3 normalWorldSpace;
        glm::vec2 texCoordinates;
    };

    std::vector<VertexLayout> vertices( static_cast<std::size_t>( width * height ) );
    for ( int z = 0; z < height; z++ ) {
        for ( int x = 0; x < width; x++ ) {
            vertices[static_cast<std::size_t>( z * height + x )] = {
                glm::vec3( static_cast<float>( x * texelScale ), 0.0f, static_cast<float>( z * texelScale ) ),
                glm::vec3( 0.0f, 1.0f, 0.0f ),
                glm::vec2( static_cast<float>( x ) / ( width - 1 ), static_cast<float>( z ) / ( height - 1 ) )
            };
        }
    }

    int numIndices = static_cast<int>( ( width - 1 ) * ( height - 1 ) * 6 );

    std::vector<uint32_t> indices( numIndices );

    int i = 0;
    for ( int y = 0; y < height - 1; y++ ) {
        for ( int x = 0; x < width - 1; x++ ) {
            indices[i++] = x + y * width;
            indices[i++] = x + 1 + y * width;
            indices[i++] = x + ( y + 1 ) * width;

            indices[i++] = x + 1 + y * width;
            indices[i++] = x + 1 + ( y + 1 ) * width;
            indices[i++] = x + ( y + 1 ) * width;
        }
    }
    //    // Even rows move left to right, odd rows move right to left.
    //    if ( z % 2 == 0 ) {
    //        // Even row
    //        int x;
    //        for ( x = 0; x < width; x++ ) {
    //           indices[index++] = x + static_cast<int>( z * width );
    //           indices[index++] = x + static_cast<int>( z * width ) + width;
    //        }
    //        // Insert degenerate vertex if this isn't the last row
    //        if ( z != height - 2 ) {
    //           indices[index++] = --x + static_cast<int>( z * width );
    //        }
    //    } else {
    //        // Odd row
    //        int x;
    //        for ( x = width - 1; x >= 0; x-- ) {
    //           indices[index++] = x + static_cast<int>( z * width );
    //           indices[index++] = x + static_cast<int>( z * width ) + width;
    //        }
    //        // Insert degenerate vertex if this isn't the last row
    //        if ( z != height - 2 ) {
    //           indices[index++] = ++x + static_cast<int>( z * width );
    //        }
    //    }
    //}

    // Create GPU Buffers
    const auto vertexCount = vertices.size() * 8;

    BufferDesc vboDesc;
    vboDesc.Type = BufferDesc::VERTEX_BUFFER;
    vboDesc.Size = vertexCount * sizeof( float );
    vboDesc.Stride = 8 * sizeof( float );

    vertexBuffer.reset( new Buffer() );
    vertexBuffer->create( renderDevice, vboDesc, ( void* )vertices.data() );

    BufferDesc iboDesc;
    iboDesc.Type = BufferDesc::INDICE_BUFFER;
    iboDesc.Size = numIndices * sizeof( uint32_t );
    iboDesc.Stride = sizeof( uint32_t );

    indiceBuffer.reset( new Buffer() );
    indiceBuffer->create( renderDevice, iboDesc, ( void* )indices.data() );

    vertexArrayObject->create( renderDevice, vertexBuffer.get(), indiceBuffer.get() );

    VertexLayout_t defaultTerrainLayout = {
        { 0, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 0 }, // POSITION
        { 1, VertexLayoutEntry::DIMENSION_XY, VertexLayoutEntry::FORMAT_FLOAT, 3 * sizeof( float ) }, // UVMAP0
    };

    vertexArrayObject->setVertexLayout( renderDevice, defaultTerrainLayout );

    material = terrainMaterial;
}

const VertexArrayObject* Terrain::getVertexArrayObject() const
{
    return vertexArrayObject.get();
}

Material* Terrain::getMaterial()
{
    return material;
}

const uint32_t Terrain::getIndiceCount() const
{
    const auto& bufferDesc = indiceBuffer->getDescription();

    return static_cast< uint32_t >( bufferDesc.Size / bufferDesc.Stride );
}
