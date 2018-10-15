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
constexpr float tessFactor = 4.0f;

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
           
            max = std::max( max, texel );
            min = std::min( min, texel );
        }
    }

    return glm::vec2( min, max );
}

void Terrain::create( RenderDevice* renderDevice, Material* terrainMaterial, uint16_t* heightmapTexels )
{
    struct VertexLayout {
        glm::vec3 positionWorldSpace;
        glm::vec3 patchBoundsAndSkirtIndex;
        glm::vec2 texCoordinates;
    };

    std::vector<VertexLayout> vertices( static_cast<std::size_t>( scalePatchX * scalePatchY /*+ scalePatchX * 4*/ ) );
    for ( int z = 0; z < scalePatchY; z++ ) {
        for ( int x = 0; x < scalePatchX; x++ ) {
            vertices[static_cast<std::size_t>( z * scalePatchX + x )] = {
                glm::vec3( static_cast<float>( x * tessFactor ), 0.0f, static_cast<float>( z * tessFactor ) ),
                glm::vec3( 0.0f, 0.0f, 0.0f ),
                glm::vec2( static_cast<float>( x ) / scalePatchX, static_cast<float>( z ) / scalePatchY )
            };
        }
    }

    const int lod0VertexCount = ( scalePatchX * scalePatchY );
    glm::vec2 zBounds = CalcYBounds( heightmapTexels, vertices[0].positionWorldSpace, vertices[lod0VertexCount - 1].positionWorldSpace );
    
    const auto heightBase = ( zBounds.x - 10.0f );

    // Update TerrainBoundingSphere
    flan::core::CreateAABB( aabb, glm::vec3( 0, 0, 0 ), glm::vec3( width / 2.0f, zBounds.y - zBounds.x, height / 2.0f ) );

    //// Generate vertices for skirt levels
    //int vertexIdx = lod0VertexCount;
    //for ( int x = 0; x < scalePatchX; ++x ) {
    //    vertices[vertexIdx] = {
    //        glm::vec3( static_cast<float>( x * tessFactor ), heightBase, 0.0f ),
    //        glm::vec3( 0.0f, 0.0f, 1.0f ),
    //        glm::vec2( static_cast<float>( x ) / scalePatchX, 0.0f )
    //    };

    //    vertexIdx++;
    //}

    //for ( int x = 0; x < scalePatchX; ++x ) {
    //    vertices[vertexIdx] = {
    //        glm::vec3( static_cast<float>( x * tessFactor ), heightBase, static_cast<float>( height - tessFactor ) ),
    //        glm::vec3( 0.0f, 0.0f, 2.0f ),
    //        glm::vec2( static_cast<float>( x ) / scalePatchX, 0.0f )
    //    };

    //    vertexIdx++;
    //}

    //for ( int y = 0; y < scalePatchY; ++y ) {
    //    vertices[vertexIdx] = {
    //        glm::vec3( 0.0f, heightBase, static_cast<float>( y * tessFactor ) ),
    //        glm::vec3( 0.0f, 0.0f, 3.0f ),
    //        glm::vec2( 0.0f, static_cast<float>( y ) / scalePatchY )
    //    };

    //    vertexIdx++;
    //}

    //for ( int y = 0; y < scalePatchY; ++y ) {
    //    vertices[vertexIdx] = {
    //        glm::vec3( static_cast<float>( width - tessFactor ), heightBase, static_cast<float>( y * tessFactor ) ),
    //        glm::vec3( 0.0f, 0.0f, 4.0f ),
    //        glm::vec2( 0.0f, static_cast<float>( y ) / scalePatchY )
    //    };

    //    vertexIdx++;
    //}

    const int numIndices = static_cast<int>( ( scalePatchX - 1 ) * ( scalePatchY - 1 ) ) * 4 /*+ 2 * 4 * ( scalePatchX - 1 ) + 2 * 4 * ( scalePatchY - 1 ) + 4*/;
   
    std::vector<uint32_t> indices( numIndices );

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
            vertex0.patchBoundsAndSkirtIndex = glm::vec3( boundsZ, 5.0f );

            i += 4;
        }
    }

    //vertexIdx = lod0VertexCount;
    //for ( int x = 0; x < scalePatchX - 1; ++x ) {
    //    indices[i++] = vertexIdx;		// control point 0
    //    indices[i++] = vertexIdx + 1;	// control point 1
    //    indices[i++] = x;				// control point 2
    //    indices[i++] = x + 1;			// control point 3

    //    glm::vec2 boundsZ = CalcYBounds( heightmapTexels, vertices[x].positionWorldSpace, vertices[x + 1].positionWorldSpace );
    //    vertices[vertexIdx].patchBoundsAndSkirtIndex.x = heightBase;
    //    vertices[vertexIdx].patchBoundsAndSkirtIndex.y = boundsZ.y;

    //    vertexIdx++;
    //}

    //for ( int x = 0; x < scalePatchX - 1; ++x ) {
    //    indices[i++] = vertexIdx + 1;
    //    indices[i++] = vertexIdx;

    //    int offset = scalePatchX * ( scalePatchY - 1 );
    //    indices[i++] = x + offset + 1;
    //    indices[i++] = x + offset;

    //    glm::vec2 boundsZ = CalcYBounds( heightmapTexels, vertices[x + offset].positionWorldSpace, vertices[x + offset + 1].positionWorldSpace );
    //    vertices[vertexIdx].patchBoundsAndSkirtIndex.x = heightBase;
    //    vertices[vertexIdx].patchBoundsAndSkirtIndex.y = boundsZ.y;

    //    vertexIdx++;
    //}

    //for ( int y = 0; y < scalePatchY - 1; ++y ) {
    //    indices[i++] = vertexIdx + 1;
    //    indices[i++] = vertexIdx;
    //    indices[i++] = ( y + 1 ) * scalePatchX;
    //    indices[i++] = y * scalePatchX;

    //    glm::vec2 boundsZ = CalcYBounds( heightmapTexels, vertices[y * scalePatchX].positionWorldSpace, vertices[( y + 1 ) * scalePatchX].positionWorldSpace );
    //    vertices[vertexIdx].patchBoundsAndSkirtIndex.x = heightBase;
    //    vertices[vertexIdx].patchBoundsAndSkirtIndex.y = boundsZ.y;

    //    vertexIdx++;
    //}

    //++vertexIdx;
    //for ( int y = 0; y < scalePatchY - 1; ++y ) {
    //    indices[i++] = vertexIdx;
    //    indices[i++] = vertexIdx + 1;
    //    indices[i++] = y * scalePatchX + scalePatchX - 1;
    //    indices[i++] = ( y + 1 ) * scalePatchX + scalePatchX - 1;

    //    glm::vec2 boundsZ = CalcYBounds( heightmapTexels, vertices[y * scalePatchX + scalePatchX - 1].positionWorldSpace, vertices[( y + 1 ) * scalePatchX + scalePatchX - 1].positionWorldSpace );
    //    vertices[vertexIdx].patchBoundsAndSkirtIndex.x = heightBase;
    //    vertices[vertexIdx].patchBoundsAndSkirtIndex.y = boundsZ.y;

    //    vertexIdx++;
    //}

    //indices[i++] = lod0VertexCount + scalePatchX - 1;
    //indices[i++] = lod0VertexCount;
    //indices[i++] = lod0VertexCount + scalePatchX + scalePatchX - 1;
    //indices[i++] = lod0VertexCount + scalePatchX;

    //vertices[lod0VertexCount + scalePatchX - 1].patchBoundsAndSkirtIndex = glm::vec3( heightBase, heightBase, 0.0f );

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

const AABB& Terrain::getAxisAlignedBoundingBox() const
{
    return aabb;
}
