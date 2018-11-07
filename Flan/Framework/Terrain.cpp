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

#include <Core/ScopedTimer.h>

#include "Material.h"
#include "Mesh.h"

Terrain::Terrain( const fnString_t& TerrainName )
    : name( TerrainName )
    , material( nullptr )
    , aabb{}
    , meshIndiceCount( 0 )
    , heightmapLowestVertex( std::numeric_limits<float>::max() )
    , heightmapHighestVertex( std::numeric_limits<float>::min() )
    , heightmap( nullptr )
    , heightmapTexture( nullptr )
    , isEditionInProgress( false )
    , vertexBuffer{ nullptr, nullptr }
    , indiceBuffer( nullptr )
    , currentVboIndex( 0 )
    , vertexArrayObject{ nullptr, nullptr }
{

}

Terrain::~Terrain()
{
    name.clear();

    delete[] heightmap;
    material = nullptr;
}

void Terrain::create( RenderDevice* renderDevice, Material* terrainMaterial, Material* grassTest, const uint16_t* splatmapTexels, const uint16_t* heightmapTexels, const uint32_t heightmapWidth, const uint32_t heightmapHeight )
{
    heightmapDimension = heightmapWidth; // (assuming width = height)

    // Preprocess heightmap texels (uint16 to float 0..1 range precision)
    heightmap = new float[heightmapWidth * heightmapHeight];
    editorHeightmap = new float[heightmapWidth * heightmapHeight];
    for ( uint32_t texelIdx = 0; texelIdx < ( heightmapWidth * heightmapHeight ); texelIdx++ ) {
        editorHeightmap[texelIdx] = static_cast< float >( static_cast< float >( heightmapTexels[texelIdx] ) / std::numeric_limits<uint16_t>::max() );
    }

    splatmap = new uint16_t[heightmapWidth * heightmapHeight];
    memcpy( splatmap, splatmapTexels, ( heightmapWidth * heightmapHeight * sizeof( uint16_t ) ) );

    // Create GPU resource
    TextureDescription splatmapTextureDesc;
    splatmapTextureDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    splatmapTextureDesc.format = IMAGE_FORMAT_R16G16B16A16_UINT;
    splatmapTextureDesc.width = heightmapWidth;
    splatmapTextureDesc.height = heightmapHeight;
    splatmapTextureDesc.mipCount = 1;
    splatmapTextureDesc.samplerCount = 1;

    splatmapTexture.reset( new Texture() );
    splatmapTexture->createAsTexture2D( renderDevice, splatmapTextureDesc, splatmap, ( heightmapWidth * heightmapHeight * sizeof( uint16_t ) ) );

    TextureDescription heightmapTextureDesc;
    heightmapTextureDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    heightmapTextureDesc.format = IMAGE_FORMAT_R32_FLOAT;
    heightmapTextureDesc.width = heightmapWidth;
    heightmapTextureDesc.height = heightmapHeight;
    heightmapTextureDesc.mipCount = 1;
    heightmapTextureDesc.samplerCount = 1;

    heightmapTexture.reset( new Texture() );
    heightmapTexture->createAsTexture2D( renderDevice, heightmapTextureDesc, editorHeightmap, ( heightmapWidth * heightmapHeight * sizeof( float ) ) );

    // Scale vertices for physics rigid body
    float heightmapHeightScale = terrainMaterial->getHeightmapScaleTEST();
    for ( uint32_t texelIdx = 0; texelIdx < ( heightmapWidth * heightmapHeight ); texelIdx++ ) {
        heightmap[texelIdx] = editorHeightmap[texelIdx] * heightmapHeightScale;

        heightmapLowestVertex = std::min( heightmapLowestVertex, heightmap[texelIdx] );
        heightmapHighestVertex = std::max( heightmapHighestVertex, heightmap[texelIdx] );
    }

    constexpr float tessFactor = 8.0f;

    scalePatchX = heightmapWidth / static_cast<uint32_t>( tessFactor );
    scalePatchY = heightmapHeight / static_cast<uint32_t>( tessFactor );

    vertices.resize( static_cast<std::size_t>( scalePatchX * scalePatchY ) );
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
    glm::vec3 yBounds;
    CalcYBounds( vertices[0].positionWorldSpace, vertices[lod0VertexCount - 1].positionWorldSpace, yBounds );
    flan::core::CreateAABB( aabb, glm::vec3( 0, 0, 0 ), glm::vec3( heightmapWidth / 2.0f, yBounds.y - yBounds.x, heightmapHeight / 2.0f ) );

    meshIndiceCount = static_cast< uint32_t >( ( scalePatchX - 1 ) * ( scalePatchY - 1 ) ) * 4;

    int i = 0;
    indices.resize( meshIndiceCount );
    for ( uint32_t y = 0; y < ( scalePatchY - 1 ); y++ ) {
        for ( uint32_t x = 0; x < ( scalePatchX - 1 ); x++ ) {
            indices[i + 0] = x + y * scalePatchX;
            indices[i + 1] = x + 1 + y * scalePatchX;
            indices[i + 2] = x + ( y + 1 ) * scalePatchX;
            indices[i + 3] = x + 1 + ( y + 1 ) * scalePatchX;

            i += 4;
        }
    }

    computePatchsBounds();

    // Create GPU Buffers
    const std::size_t vertexCount = ( vertices.size() * 8 );

    BufferDesc vboDesc;
#if FLAN_DEVBUILD
    // Allows CPU-side binding (for patch bounds recomputing)
    // Theorically this should be slower than a static vbo
    vboDesc.Type = BufferDesc::DYNAMIC_VERTEX_BUFFER;
#else
    vboDesc.Type = BufferDesc::VERTEX_BUFFER;
#endif
    vboDesc.Size = vertexCount * sizeof( float );
    vboDesc.Stride = 8 * sizeof( float );

    for ( int i = 0; i < 2; i++ ) {
        vertexBuffer[i].reset( new Buffer() );
        vertexBuffer[i]->create( renderDevice, vboDesc, vertices.data() );
    }

    BufferDesc iboDesc;
    iboDesc.Type = BufferDesc::INDICE_BUFFER;
    iboDesc.Size = meshIndiceCount * sizeof( uint32_t );
    iboDesc.Stride = sizeof( uint32_t );

    indiceBuffer.reset( new Buffer() );
    indiceBuffer->create( renderDevice, iboDesc, indices.data() );

    for ( int i = 0; i < 2; i++ ) {
        vertexArrayObject[i].reset( new VertexArrayObject() );
        vertexArrayObject[i]->create( renderDevice, vertexBuffer[i].get(), indiceBuffer.get() );

        VertexLayout_t defaultTerrainLayout = {
            { 0, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 0 }, // POSITION
            { 1, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 3 * sizeof( float ) }, // POSITION
            { 2, VertexLayoutEntry::DIMENSION_XY, VertexLayoutEntry::FORMAT_FLOAT, 6 * sizeof( float ) }, // UVMAP0
        };

        vertexArrayObject[i]->setVertexLayout( renderDevice, defaultTerrainLayout );
    }

    material = terrainMaterial;
    material->setHeightmapTEST( heightmapTexture.get() );

    struct GrassLayout
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };

    GrassLayout grassBlade[4 * 4 * 2];

    // Grass test
    for ( int quadId = 0; quadId < 4; quadId++ ) {
        auto vertId = quadId * 4;
        auto planeDepth = static_cast< float >( quadId ) * 0.5f - 0.750f;

        grassBlade[vertId + 0] = { { -1.0f, 0.0f, planeDepth },{ 0, 1, 0 }, { 1, 1 } };
        grassBlade[vertId + 1] = { { 1.0f, 0.0f, planeDepth },{ 0, 1, 0 }, { 0, 1 } };
        grassBlade[vertId + 2] = { { 1.0f, 1.0f, planeDepth },{ 0, 1, 0 }, { 0, 0 } };
        grassBlade[vertId + 3] = { { -1.0f, 1.0f, planeDepth },{ 0, 1, 0 }, { 1, 0 } };
    }

    for ( int quadId = 0; quadId < 4; quadId++ ) {
        auto vertId = quadId * 4 + 16;
        auto planeDepth = static_cast< float >( quadId ) * 0.5f - 0.750f;

        grassBlade[vertId + 0] = { { planeDepth, 0.0f, -1.0f }, { 0, 1, 0 }, { 1, 1 } };
        grassBlade[vertId + 1] = { { planeDepth, 0.0f, 1.0f }, { 0, 1, 0 }, { 0, 1 } };
        grassBlade[vertId + 2] = { { planeDepth, 1.0f, 1.0f }, { 0, 1, 0 }, { 0, 0 } };
        grassBlade[vertId + 3] = { { planeDepth, 1.0f, -1.0f }, { 0, 1, 0 }, { 1, 0 } };
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
    vboGrassDesc.Size = 4 * 4 * 2 * sizeof( GrassLayout );
    vboGrassDesc.Stride = sizeof( GrassLayout );

    BufferDesc iboGrassDesc;
    iboGrassDesc.Type = BufferDesc::INDICE_BUFFER;
    iboGrassDesc.Size = 6 * 8 * sizeof( uint32_t );
    iboGrassDesc.Stride = sizeof( uint32_t );

    GRASS_TEST = new Mesh();
    GRASS_TEST->create( renderDevice, vboGrassDesc, iboGrassDesc, ( float* )&grassBlade[0], grassIndices );

    SubMesh baseSubMesh;
    baseSubMesh.indiceBufferOffset = 0;
    baseSubMesh.indiceCount = 6 * 8;
    baseSubMesh.material = grassTest;
    baseSubMesh.boundingSphere.center = { 0, 0, 0 };
    baseSubMesh.boundingSphere.radius = 2.0f;
    
    flan::core::CreateAABB( baseSubMesh.aabb, baseSubMesh.boundingSphere.center, { 2.0f, 2.0f, 2.0f } );

    GRASS_TEST->addLevelOfDetail( 0, 256.0f );
    GRASS_TEST->addSubMesh( 0, std::move( baseSubMesh ) );
}

const VertexArrayObject* Terrain::getVertexArrayObject() const
{
    return vertexArrayObject[currentVboIndex].get();
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
    return editorHeightmap;
}

float* Terrain::getHeightmapValuesHeightScaled() const
{
    return heightmap;
}

float Terrain::getHeightmapLowestVertex() const
{
    return heightmapLowestVertex;
}

float Terrain::getHeightmapHighestVertex() const
{
    return heightmapHighestVertex;
}

void Terrain::setVertexHeight( const uint32_t vertexIndex, const float updatedHeight )
{
    editorHeightmap[vertexIndex] = updatedHeight;
}

void Terrain::setVertexMaterial( const uint32_t vertexIndex, const uint32_t layerIndex, const int materialIndex, const float weight )
{
    splatmap[vertexIndex + layerIndex] = materialIndex;
    splatmap[vertexIndex + 3] = weight;
}

void Terrain::setGrassWeight( const uint32_t vertexIndex, const float weight )
{
    splatmap[vertexIndex + 2] = weight;
}

void Terrain::uploadHeightmap( CommandList* cmdList )
{
    TextureCopyBox cpyBox;
    cpyBox.x = 0;
    cpyBox.y = 0;
    cpyBox.arrayIndex = 0;
    cpyBox.mipLevel = 0;

    heightmapTexture->updateSubresource( cmdList, cpyBox, heightmapDimension, heightmapDimension, 1 * sizeof( float ), editorHeightmap );
}

void Terrain::uploadPatchBounds( CommandList* cmdList )
{
    vertexBuffer[currentVboIndex]->updateAsynchronous( cmdList, vertices.data(), vertices.size() * sizeof( VertexLayout ) );
    currentVboIndex = ( currentVboIndex == 0 ) ? 1 : 0;
}

void Terrain::computePatchsBounds()
{
    int i = 0;
    for ( uint32_t y = 0; y < ( scalePatchY - 1 ); y++ ) {
        for ( uint32_t x = 0; x < ( scalePatchX - 1 ); x++ ) {
            auto& vertex0 = vertices[indices[i + 0]];
            auto& vertex3 = vertices[indices[i + 3]];
    
            CalcYBounds( vertex0.positionWorldSpace, vertex3.positionWorldSpace, vertex0.patchBoundsAndSkirtIndex );

            i += 4;
        }
    }
}

void Terrain::CalcYBounds( const glm::vec3& bottomLeft, const glm::vec3& topRight, glm::vec3& output )
{
    const float hmapDimensionFloat = static_cast< float >( heightmapDimension );

    float max = -std::numeric_limits<float>::max();
    float min = std::numeric_limits<float>::max();

    const int bottomLeftX = ( bottomLeft.x == 0.0f ) ? ( int )bottomLeft.x : ( ( int )bottomLeft.x - 1 );
    const int bottomLeftY = ( bottomLeft.z == 0.0f ) ? ( int )bottomLeft.z : ( ( int )bottomLeft.z - 1 );
    const int topRightX = ( topRight.x >= hmapDimensionFloat ) ? ( int )topRight.x : ( ( int )topRight.x + 1 );
    const int topRightY = ( topRight.z >= hmapDimensionFloat ) ? ( int )topRight.z : ( ( int )topRight.z + 1 );

    for ( int y = bottomLeftY; y <= topRightY; y++ ) {
        for ( int x = bottomLeftX; x <= topRightX; x++ ) {
            float texel = editorHeightmap[static_cast< int >( y * heightmapDimension + x )];

            max = std::max( max, texel );
            min = std::min( min, texel );
        }
    }

    output.x = min;
    output.y = max;
}
