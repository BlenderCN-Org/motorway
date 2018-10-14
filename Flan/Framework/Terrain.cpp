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

constexpr float width = 512;
constexpr float height = 512;
constexpr float tessFactor = 16.0f;

constexpr float heightScale = width / 16.0f;

constexpr float scalePatchX = width / tessFactor;
constexpr float scalePatchY = height / tessFactor;

glm::vec2 CalcYBounds( uint16_t* texels, const glm::vec3& bottomLeft, const glm::vec3& topRight )
{
    float max = -std::numeric_limits<float>::max();
    float min = std::numeric_limits<float>::max();

    int bottomLeftX = ( bottomLeft.x == 0 ) ? ( int )bottomLeft.x : ( ( int )bottomLeft.x - 1 );
    int bottomLeftY = ( bottomLeft.z == 0 ) ? ( int )bottomLeft.z : ( ( int )bottomLeft.z - 1 );
    int topRightX = ( topRight.x >= width ) ? ( int )topRight.x : ( ( int )topRight.x + 1 );
    int topRightY = ( topRight.z >= width ) ? ( int )topRight.z : ( ( int )topRight.z + 1 );

    for ( int y = bottomLeftY; y <= topRightY; y++ ) {
        for ( int x = bottomLeftX; x <= topRightX; x++ ) {
            uint16_t texelUi = texels[static_cast< int >( y * width + x )];
            float texel = texelUi / static_cast< float >( std::numeric_limits<uint16_t>::max() );
            float z = texel * heightScale;

            max = std::max( max, z );
            min = std::min( min, z );
        }
    }

    return glm::vec2( min, max );
}

void Terrain::create( RenderDevice* renderDevice, Material* terrainMaterial, uint16_t* heightmapTexels )
{
    struct VertexLayout {
        glm::vec3 positionWorldSpace;
        glm::vec3 normalWorldSpace;
        glm::vec2 texCoordinates;
    };

    std::vector<VertexLayout> vertices( static_cast<std::size_t>( scalePatchX * scalePatchY ) );
    for ( int z = 0; z < scalePatchY; z++ ) {
        for ( int x = 0; x < scalePatchX; x++ ) {
            vertices[static_cast<std::size_t>( z * scalePatchX + x )] = {
                glm::vec3( static_cast<float>( x * tessFactor ), 0.0f, static_cast<float>( z * tessFactor ) ),
                glm::vec3( 0.0f, 0.0f, 0.0f ),
                glm::vec2( static_cast<float>( x ) / scalePatchX, static_cast<float>( z ) / scalePatchY )
            };
        }
    }

    const int numTiles = static_cast<int>( ( scalePatchX - 1 ) * ( scalePatchY - 1 ) );
    const int numIndices = numTiles * 4;

    std::vector<uint32_t> indices( numIndices );
    terrainTiles.resize( numTiles );

    int tileIndex = 0;
    int i = 0;
    for ( int y = 0; y < scalePatchY - 1; y++ ) {
        for ( int x = 0; x < scalePatchX - 1; x++ ) {
            indices[i + 0] = x + y * scalePatchX;
            indices[i + 1] = x + 1 + y * scalePatchX;
            indices[i + 2] = x + ( y + 1 ) * scalePatchX;
            indices[i + 3] = x + 1 + ( y + 1 ) * scalePatchX;

            // Compute Tile Bounding Sphere
            auto& vertex0 = vertices[indices[i + 0]];
            auto& vertex3 = vertices[indices[i + 3]];
          
            glm::vec2 boundsZ = CalcYBounds( heightmapTexels, vertex0.positionWorldSpace, vertex3.positionWorldSpace );
            vertex0.normalWorldSpace = glm::vec3( boundsZ, 0.0f );

            i += 4;
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
