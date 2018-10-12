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
    constexpr float width = 1024.0f;
    constexpr float height = 1024.0f;
    constexpr float tessFactor = 4.0f;

    constexpr float scalePatchX = width / tessFactor;
    constexpr float scalePatchY = height / tessFactor;

    // TODO Pool Grid and instantiate it per heightmap drawcall?
    struct VertexLayout
    {
        glm::vec3 positionWorldSpace;
        glm::vec3 normalWorldSpace;
        glm::vec2 texCoordinates;
    };

    std::vector<VertexLayout> vertices( static_cast<std::size_t>( scalePatchX * scalePatchY ) );
    for ( int z = 0; z < scalePatchY; z++ ) {
        for ( int x = 0; x < scalePatchX; x++ ) {
            vertices[static_cast<std::size_t>( z * scalePatchX + x )] = {
                glm::vec3( static_cast<float>( x * tessFactor ), 0.0f, static_cast<float>( z * tessFactor ) ),
                glm::vec3( 0.0f, 1.0f, 0.0f ),
                glm::vec2( static_cast<float>( x ) / scalePatchX, static_cast<float>( z ) / scalePatchY )
            };
        }
    }

    int numIndices = static_cast<int>( ( scalePatchX - 1 ) * ( scalePatchY - 1 ) * 4 );

    std::vector<uint32_t> indices( numIndices );

    int i = 0;
    for ( int y = 0; y < scalePatchY - 1; y++ ) {
        for ( int x = 0; x < scalePatchX - 1; x++ ) {
            indices[i++] = x + y * scalePatchX;
            indices[i++] = x + 1 + y * scalePatchX;
            indices[i++] = x + ( y + 1 ) * scalePatchX;
            indices[i++] = x + 1 + ( y + 1 ) * scalePatchX;
        }
    }

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
        { 1, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 3 * sizeof( float ) }, // POSITION
        { 2, VertexLayoutEntry::DIMENSION_XY, VertexLayoutEntry::FORMAT_FLOAT, 6 * sizeof( float ) }, // UVMAP0
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
