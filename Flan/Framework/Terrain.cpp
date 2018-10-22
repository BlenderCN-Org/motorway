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

#include <Rendering/VertexArrayObject.h>
#include <Rendering/Texture.h>
#include <Rendering/Buffer.h>

#include "Material.h"
#include "Mesh.h"

Terrain::Terrain( const fnString_t& TerrainName )
    : name( TerrainName )
    , material( nullptr )
    , aabb{}
    , meshIndiceCount( 0 )
    , heightmap( nullptr )
    , heightmapTexture( nullptr )
    , vertexBuffer( nullptr )
    , indiceBuffer( nullptr )
    , vertexArrayObject( new VertexArrayObject() )
{

}

Terrain::~Terrain()
{
    name.clear();

    delete[] heightmap;
    material = nullptr;
}

glm::vec2 CalcYBounds( const uint16_t* texels, const int width, const glm::vec3& bottomLeft, const glm::vec3& topRight )
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

void Terrain::create( RenderDevice* renderDevice, Material* terrainMaterial, const uint16_t* heightmapTexels, const uint32_t heightmapWidth, const uint32_t heightmapHeight )
{
    struct VertexLayout {
        glm::vec3 positionWorldSpace;
        glm::vec3 patchBoundsAndSkirtIndex;
        glm::vec2 texCoordinates;
    };

    // Preprocess heightmap texels (uint16 to float 0..1 range precision)
    heightmap = new float[heightmapWidth * heightmapHeight];
    for ( uint32_t texelIdx = 0; texelIdx < ( heightmapWidth * heightmapHeight ); texelIdx++ ) {
        heightmap[texelIdx] = static_cast<float>( static_cast<float>( heightmapTexels[texelIdx] ) / std::numeric_limits<uint16_t>::max() );
    }

    // Create GPU resource
    TextureDescription heightmapTextureDesc;
    heightmapTextureDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    heightmapTextureDesc.format = IMAGE_FORMAT_R32_FLOAT;
    heightmapTextureDesc.width = heightmapWidth;
    heightmapTextureDesc.height = heightmapHeight;
    heightmapTextureDesc.mipCount = 1;
    heightmapTextureDesc.samplerCount = 1;

    heightmapTexture.reset( new Texture() );
    heightmapTexture->createAsTexture2D( renderDevice, heightmapTextureDesc, heightmap, ( heightmapWidth * heightmapHeight * sizeof( float ) ) );

    constexpr float tessFactor = 8.0f;

    uint32_t scalePatchX = heightmapWidth / static_cast<uint32_t>( tessFactor );
    uint32_t scalePatchY = heightmapHeight / static_cast<uint32_t>( tessFactor );

    std::vector<VertexLayout> vertices( static_cast<std::size_t>( scalePatchX * scalePatchY ) );
    for ( uint32_t z = 0; z < scalePatchY; z++ ) {
        for ( uint32_t x = 0; x < scalePatchX; x++ ) {
            vertices[static_cast<std::size_t>( z * scalePatchX + x )] = {
                glm::vec3( static_cast<float>( x * tessFactor ), 0.0f, static_cast<float>( z * tessFactor ) ),
                glm::vec3( 0.0f, 0.0f, 0.0f ),
                glm::vec2( static_cast<float>( x ) / scalePatchX, static_cast<float>( z ) / scalePatchY )
            };
        }
    }

    // Update Terrain Bounding Geometry
    const int lod0VertexCount = static_cast<int>( scalePatchX * scalePatchY );
    glm::vec2 zBounds = CalcYBounds( heightmapTexels, heightmapWidth, vertices[0].positionWorldSpace, vertices[lod0VertexCount - 1].positionWorldSpace );
    flan::core::CreateAABB( aabb, glm::vec3( 0, 0, 0 ), glm::vec3( heightmapWidth / 2.0f, zBounds.y - zBounds.x, heightmapHeight / 2.0f ) );

    meshIndiceCount = static_cast< uint32_t >( ( scalePatchX - 1 ) * ( scalePatchY - 1 ) ) * 4;

    int i = 0;
    std::vector<uint32_t> indices( meshIndiceCount );
    for ( uint32_t y = 0; y < ( scalePatchY - 1 ); y++ ) {
        for ( uint32_t x = 0; x < ( scalePatchX - 1 ); x++ ) {
            indices[i + 0] = x + y * scalePatchX;
            indices[i + 1] = x + 1 + y * scalePatchX;
            indices[i + 2] = x + ( y + 1 ) * scalePatchX;
            indices[i + 3] = x + 1 + ( y + 1 ) * scalePatchX;

            // Compute Tile Bounding Sphere
            auto& vertex0 = vertices[indices[i + 0]];
            auto& vertex3 = vertices[indices[i + 3]];
          
            glm::vec2 boundsZ = CalcYBounds( heightmapTexels, heightmapWidth, vertex0.positionWorldSpace, vertex3.positionWorldSpace );
            vertex0.patchBoundsAndSkirtIndex = glm::vec3( boundsZ, 5.0f );

            i += 4;
        }
    }

    // Create GPU Buffers
    const std::size_t vertexCount = ( vertices.size() * 8 );

    BufferDesc vboDesc;
    vboDesc.Type = BufferDesc::VERTEX_BUFFER;
    vboDesc.Size = vertexCount * sizeof( float );
    vboDesc.Stride = 8 * sizeof( float );

    vertexBuffer.reset( new Buffer() );
    vertexBuffer->create( renderDevice, vboDesc, vertices.data() );

    BufferDesc iboDesc;
    iboDesc.Type = BufferDesc::INDICE_BUFFER;
    iboDesc.Size = meshIndiceCount * sizeof( uint32_t );
    iboDesc.Stride = sizeof( uint32_t );

    indiceBuffer.reset( new Buffer() );
    indiceBuffer->create( renderDevice, iboDesc, indices.data() );

    vertexArrayObject->create( renderDevice, vertexBuffer.get(), indiceBuffer.get() );

    VertexLayout_t defaultTerrainLayout = {
        { 0, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 0 }, // POSITION
        { 1, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 3 * sizeof( float ) }, // POSITION
        { 2, VertexLayoutEntry::DIMENSION_XY, VertexLayoutEntry::FORMAT_FLOAT, 6 * sizeof( float ) }, // UVMAP0
    };

    vertexArrayObject->setVertexLayout( renderDevice, defaultTerrainLayout );

    material = terrainMaterial;
    material->setHeightmapTEST( heightmapTexture.get() );

    struct GrassLayout
    {
        glm::vec3 position;
        glm::vec2 texCoords;
    };

    GrassLayout grassBlade[4 * 4 * 2];

    // Grass test
    for ( int quadId = 0; quadId < 4; quadId++ ) {
        auto vertId = quadId * 4;
        auto planeDepth = static_cast< float >( quadId ) * 0.5f - 0.750f;

        grassBlade[vertId + 0] = { { -1.0f, 0.0f, planeDepth }, { 0, 0 } };
        grassBlade[vertId + 1] = { { 1.0f, 0.0f, planeDepth }, { 0, 0 } };
        grassBlade[vertId + 2] = { { 1.0f, 1.0f, planeDepth }, { 0, 0 } };
        grassBlade[vertId + 3] = { { -1.0f, 1.0f, planeDepth }, { 0, 0 } };
    }

    for ( int quadId = 0; quadId < 4; quadId++ ) {
        auto vertId = quadId * 4 + 16;
        auto planeDepth = static_cast< float >( quadId ) * 0.5f - 0.750f;

        grassBlade[vertId + 0] = { { planeDepth, 0.0f, -1.0f }, { 0, 0 } };
        grassBlade[vertId + 1] = { { planeDepth, 0.0f, 1.0f }, { 0, 0 } };
        grassBlade[vertId + 2] = { { planeDepth, 1.0f, 1.0f }, { 0, 0 } };
        grassBlade[vertId + 3] = { { planeDepth, 1.0f, -1.0f }, { 0, 0 } };
    }

    uint32_t grassIndices[6 * 8];
    i = 0;
    for ( int quadId = 0; quadId < 8; quadId++ ) {
        grassIndices[i + 0] = quadId * 4 + 0;
        grassIndices[i + 1] = quadId * 4 + 1;
        grassIndices[i + 2] = quadId * 4 + 2;

        grassIndices[i + 3] = quadId * 4 + 0;
        grassIndices[i + 4] = quadId * 4 + 2;
        grassIndices[i + 5] = quadId * 4 + 3;

        i += 6;
    }
    
    BufferDesc vboGrassDesc;
    vboGrassDesc.Type = BufferDesc::VERTEX_BUFFER;
    vboGrassDesc.Size = 4 * 4 * 2 * 5 * sizeof( float );
    vboGrassDesc.Stride = 5 * sizeof( float );

    BufferDesc iboGrassDesc;
    iboGrassDesc.Type = BufferDesc::INDICE_BUFFER;
    iboGrassDesc.Size = 6 * 8 * sizeof( uint32_t );
    iboGrassDesc.Stride = sizeof( uint32_t );

    GRASS_TEST = new Mesh();
    GRASS_TEST->create( renderDevice, vboGrassDesc, iboGrassDesc, ( float* )&grassBlade[0], grassIndices );

    SubMesh baseSubMesh;
    baseSubMesh.indiceBufferOffset = 0;
    baseSubMesh.indiceCount = 48;
    baseSubMesh.material = material;
    baseSubMesh.boundingSphere.center = { 0, 0, 0 };
    baseSubMesh.boundingSphere.radius = 2.0f;

    GRASS_TEST->addLevelOfDetail( 0, 64.0f );
    GRASS_TEST->addSubMesh( 0, std::move( baseSubMesh ) );
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
    return meshIndiceCount;
}

const AABB& Terrain::getAxisAlignedBoundingBox() const
{
    return aabb;
}

float* Terrain::getHeightmapValues() const
{
    return heightmap;
}
