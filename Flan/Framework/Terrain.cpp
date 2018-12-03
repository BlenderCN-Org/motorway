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

#include <Core/BlueNoise.h>

Terrain::Terrain( const fnString_t& TerrainName )
    : name( TerrainName )
    , material( nullptr )
    , aabb{}
    , meshIndiceCount( 0 )
    , heightmapLowestVertex( std::numeric_limits<float>::max() )
    , heightmapHighestVertex( std::numeric_limits<float>::min() )
    , heightmap( nullptr )
    , heightmapTexture( nullptr )
    , grassmap( nullptr )
    , grassmapTexture( nullptr )
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

void Terrain::create( RenderDevice* renderDevice, BaseAllocator* allocator, Material* terrainMaterial, const uint16_t* grassmapTexels, const uint16_t* splatmapTexels, const uint16_t* heightmapTexels, const uint32_t heightmapWidth, const uint32_t heightmapHeight )
{
    heightmapDimension = heightmapWidth; // (assuming width = height)

    // Preprocess heightmap texels (uint16 to float 0..1 range precision)
    heightmap = flan::core::allocateArray<float>( allocator, heightmapWidth * heightmapHeight );
    editorHeightmap = flan::core::allocateArray<float>( allocator, heightmapWidth * heightmapHeight );
    for ( uint32_t texelIdx = 0; texelIdx < ( heightmapWidth * heightmapHeight ); texelIdx++ ) {
        editorHeightmap[texelIdx] = static_cast< float >( static_cast< float >( heightmapTexels[texelIdx] ) / std::numeric_limits<uint16_t>::max() );
    }

    splatmap = flan::core::allocateArray<uint16_t>( allocator, heightmapWidth * heightmapHeight * 4 );
    memcpy( splatmap, splatmapTexels, ( heightmapWidth * heightmapHeight * 4 * sizeof( uint16_t ) ) );

    grassmap = flan::core::allocateArray<float>( allocator, heightmapWidth * heightmapHeight * 4 );
    for ( uint32_t texelIdx = 0; texelIdx < ( heightmapWidth * heightmapHeight ) * 4; texelIdx += 4 ) {
        grassmap[texelIdx] = static_cast< float >( static_cast< float >( grassmapTexels[texelIdx] ) / std::numeric_limits<uint16_t>::max() );
        grassmap[texelIdx + 1] = static_cast< float >( static_cast< float >( grassmapTexels[texelIdx + 1] ) / std::numeric_limits<uint16_t>::max() );
        grassmap[texelIdx + 2] = static_cast< float >( static_cast< float >( grassmapTexels[texelIdx + 2] ) / std::numeric_limits<uint16_t>::max() );
        grassmap[texelIdx + 3] = static_cast< float >( static_cast< float >( grassmapTexels[texelIdx + 3] ) / std::numeric_limits<uint16_t>::max() );
    }

    // Create GPU resource
    TextureDescription splatmapTextureDesc;
    splatmapTextureDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    splatmapTextureDesc.format = IMAGE_FORMAT_R16G16B16A16_UINT;
    splatmapTextureDesc.width = heightmapWidth;
    splatmapTextureDesc.height = heightmapHeight;
    splatmapTextureDesc.mipCount = 1;
    splatmapTextureDesc.samplerCount = 1;

    splatmapTexture = flan::core::allocate<Texture>( allocator );
    splatmapTexture->createAsTexture2D( renderDevice, splatmapTextureDesc, splatmap, ( heightmapWidth * heightmapHeight * 4 * sizeof( uint16_t ) ) );

    TextureDescription heightmapTextureDesc;
    heightmapTextureDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    heightmapTextureDesc.format = IMAGE_FORMAT_R32_FLOAT;
    heightmapTextureDesc.width = heightmapWidth;
    heightmapTextureDesc.height = heightmapHeight;
    heightmapTextureDesc.mipCount = 1;
    heightmapTextureDesc.samplerCount = 1;

    heightmapTexture = flan::core::allocate<Texture>( allocator );
    heightmapTexture->createAsTexture2D( renderDevice, heightmapTextureDesc, editorHeightmap, ( heightmapWidth * heightmapHeight * sizeof( float ) ) );

    TextureDescription grassmapTextureDesc;
    grassmapTextureDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    grassmapTextureDesc.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
    grassmapTextureDesc.width = 512;
    grassmapTextureDesc.height = 512;
    grassmapTextureDesc.mipCount = 1;
    grassmapTextureDesc.samplerCount = 1;

    grassmapTexture = flan::core::allocate<Texture>( allocator );
    grassmapTexture->createAsTexture2D( renderDevice, grassmapTextureDesc );

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
    material->setHeightmapTEST( heightmapTexture, splatmapTexture, grassmapTexture );
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

void Terrain::setVertexMaterial( const uint32_t vertexIndex, const int materialIndexBaseLayer, const int materialIndexOverlayLayer, const float overlayLayerStrength )
{
    auto blendWeight = static_cast<uint16_t>( overlayLayerStrength * std::numeric_limits<uint16_t>::max() );

    splatmap[vertexIndex] = static_cast<uint16_t>( materialIndexBaseLayer * 257 );
    splatmap[vertexIndex + 1] = static_cast<uint16_t>( materialIndexOverlayLayer * 257 );
    splatmap[vertexIndex + 2] = blendWeight;
}

void Terrain::setGrassHeight( const uint32_t vertexIndex, const glm::vec3& grassColor, const float updatedGrassHeight )
{
    grassmap[vertexIndex] = grassColor.r;
    grassmap[vertexIndex + 1] = grassColor.g;
    grassmap[vertexIndex + 2] = grassColor.b;

    grassmap[vertexIndex + 3] = updatedGrassHeight;
}

void Terrain::uploadSplatmap( CommandList* cmdList )
{
    TextureCopyBox cpyBox;
    cpyBox.x = 0;
    cpyBox.y = 0;
    cpyBox.arrayIndex = 0;
    cpyBox.mipLevel = 0;

    splatmapTexture->updateSubresource( cmdList, cpyBox, heightmapDimension, heightmapDimension, 4 * sizeof( uint16_t ), splatmap );
}

void Terrain::uploadGrassmap( CommandList* cmdList )
{
    TextureCopyBox cpyBox;
    cpyBox.x = 0;
    cpyBox.y = 0;
    cpyBox.arrayIndex = 0;
    cpyBox.mipLevel = 0;

    grassmapTexture->updateSubresource( cmdList, cpyBox, heightmapDimension, heightmapDimension, 4 * sizeof( float ), grassmap );
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

void Terrain::CalcYBounds( const glm::vec3& FLAN_RESTRICT bottomLeft, const glm::vec3& FLAN_RESTRICT topRight, glm::vec3& FLAN_RESTRICT output )
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
