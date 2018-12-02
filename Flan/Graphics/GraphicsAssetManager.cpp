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
#include "GraphicsAssetManager.h"

// Io
#include <FileSystem/VirtualFileSystem.h>

#include <Io/DirectDrawSurface.h>
#include <Io/FontDescriptor.h>
#include <Io/Mesh.h>
#include <Io/Model.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
//
//#define STBI_MALLOC(sz)           flan::core::allocate( sz )
//#define STBI_REALLOC(p,newsz)     flan::core::allocate( newsz )
//#define STBI_FREE(p)              flan::core::free(p)

#include <stb_image.h>
#endif

#include <Core/LocaleHelpers.h>
#include <Core/Allocators/BaseAllocator.h>
#include <Core/Allocators/FreeListAllocator.h>

// Assets
#include <Rendering/Texture.h>
#include <Framework/Mesh.h>
#include <Framework/Model.h>
#include <Framework/Material.h>
#include <Rendering/Buffer.h>

using namespace flan::core;

GraphicsAssetManager::RawTexels::~RawTexels()
{
    stbi_image_free( reinterpret_cast<stbi_us*>( data ) );
}

GraphicsAssetManager::GraphicsAssetManager( RenderDevice* activeRenderDevice, ShaderStageManager* activeShaderStageManager, VirtualFileSystem* activeVFS, BaseAllocator* allocator )
    : assetStreamingHeap( flan::core::allocate<FreeListAllocator>( allocator, 32 * 1024 * 1024, allocator->allocate( 32 * 1024 * 1024 ) ) )
    , renderDevice( activeRenderDevice )
    , shaderStageManager( activeShaderStageManager )
    , virtualFileSystem( activeVFS )
{

}

GraphicsAssetManager::~GraphicsAssetManager()
{
    fontMap.clear();
    textureMap.clear();
}

void GraphicsAssetManager::destroy()
{
    for ( auto& texture : textureMap ) {
        texture.second->destroy( renderDevice );
    }
}

int stbi_readcallback( void *user, char *data, int size )
{
    auto f = ( FileSystemObject* )user; 
    f->read( ( uint8_t* )data, size ); 
    return size;
}

int stbi_eofcallback( void *user )
{
    auto f = ( FileSystemObject* )user;
    return ( int )f->isGood();
}

void stbi_skipcallback( void *user, int n )
{
    auto f = ( FileSystemObject* )user;
    f->skip( n );
}

Texture* GraphicsAssetManager::getTexture( const fnChar_t* assetName, const bool forceReload )
{
    auto file = virtualFileSystem->openFile( assetName, eFileOpenMode::FILE_OPEN_MODE_READ | eFileOpenMode::FILE_OPEN_MODE_BINARY );
    if ( file == nullptr ) {
        FLAN_CERR << "'" << assetName << "' does not exist!" << std::endl;
        return nullptr;
    }

    auto assetHashcode = file->getHashcode();
    auto mapIterator = textureMap.find( assetHashcode );
    const bool alreadyExists = ( mapIterator != textureMap.end() );

    if ( alreadyExists && !forceReload ) {
        return mapIterator->second;
    }

    auto texFileFormat = GetFileExtensionFromPath( assetName );
    StringToLower( texFileFormat );
    auto texFileFormatHashcode = CRC32( texFileFormat );

    switch ( texFileFormatHashcode ) {
    case FLAN_STRING_HASH( "dds" ):
    {
        DirectDrawSurface ddsData;
        LoadDirectDrawSurface( file, ddsData );

        if ( !alreadyExists ) {
            textureMap[assetHashcode] = flan::core::allocate<Texture>( assetStreamingHeap );
        }

        switch ( ddsData.textureDescription.dimension ) {
        case TextureDescription::DIMENSION_TEXTURE_1D:
            textureMap[assetHashcode]->createAsTexture1D( renderDevice, ddsData.textureDescription, ddsData.textureData.data(), ddsData.textureData.size() / ddsData.textureDescription.height );
            break;
        case TextureDescription::DIMENSION_TEXTURE_2D:
            textureMap[assetHashcode]->createAsTexture2D( renderDevice, ddsData.textureDescription, ddsData.textureData.data(), ddsData.textureData.size() / ddsData.textureDescription.height );
            break;
        case TextureDescription::DIMENSION_TEXTURE_3D:
            textureMap[assetHashcode]->createAsTexture3D( renderDevice, ddsData.textureDescription, ddsData.textureData.data(), ddsData.textureData.size() / ddsData.textureDescription.height );
            break;
        }
    } break;

    case FLAN_STRING_HASH( "png16" ):
    case FLAN_STRING_HASH( "hmap" ): {
        stbi_io_callbacks callbacks;
        callbacks.read = stbi_readcallback;
        callbacks.skip = stbi_skipcallback;
        callbacks.eof = stbi_eofcallback;

        int w;
        int h;
        int comp;
        auto* image = stbi_load_16_from_callbacks( &callbacks, file, &w, &h, &comp, STBI_default );
        
        if ( !alreadyExists ) {
            textureMap[assetHashcode] = flan::core::allocate<Texture>( assetStreamingHeap );
        }

        TextureDescription desc;
        desc.width = w;
        desc.height = h;
        desc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
        desc.arraySize = 1;
        desc.mipCount = 1;
        desc.samplerCount = 1;
        desc.depth = 0;

        switch ( comp ) {
        case 1:
            desc.format = eImageFormat::IMAGE_FORMAT_R16_UINT;
            break;
        case 2:
            desc.format = eImageFormat::IMAGE_FORMAT_R16G16_UINT;
            break;  
        case 3:
        case 4:
            desc.format = eImageFormat::IMAGE_FORMAT_R16G16B16A16_UINT;
            break;
        }

        textureMap[assetHashcode]->createAsTexture2D( renderDevice, desc, image, w * comp );

        stbi_image_free( image );
    } break;
    
    case FLAN_STRING_HASH( "jpg" ):
    case FLAN_STRING_HASH( "jpeg" ):
    case FLAN_STRING_HASH( "png" ):
    case FLAN_STRING_HASH( "tga" ):
    case FLAN_STRING_HASH( "bmp" ):
    case FLAN_STRING_HASH( "psd" ):
    case FLAN_STRING_HASH( "gif" ):
    case FLAN_STRING_HASH( "hdr" ): {
        stbi_io_callbacks callbacks;
        callbacks.read = stbi_readcallback;
        callbacks.skip = stbi_skipcallback;
        callbacks.eof = stbi_eofcallback;

        int w;
        int h;
        int comp;
        unsigned char* image = stbi_load_from_callbacks( &callbacks, file, &w, &h, &comp, STBI_default );
        
        if ( !alreadyExists ) {
            textureMap[assetHashcode] = flan::core::allocate<Texture>( assetStreamingHeap );
        }

        TextureDescription desc;
        desc.width = w;
        desc.height = h;
        desc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
        desc.arraySize = 1;
        desc.mipCount = 1;
        desc.samplerCount = 1;
        desc.depth = 0;

        switch ( comp ) {
        case 1:
            desc.format = eImageFormat::IMAGE_FORMAT_R8_UINT;
            break;
        case 2:
            desc.format = eImageFormat::IMAGE_FORMAT_R8G8_UINT;
            break;  
        case 3:
        case 4:
            desc.format = eImageFormat::IMAGE_FORMAT_R8G8B8A8_UNORM;
            break;
        }

        textureMap[assetHashcode]->createAsTexture2D( renderDevice, desc, image, w * comp );

        stbi_image_free( image );
    } break;

    default:
        FLAN_CERR << "Unsupported fileformat with extension: " << texFileFormat << std::endl;
        return nullptr;
    }

    file->close();
    textureMap[assetHashcode]->setResourceName( renderDevice, WideStringToString( assetName ) );

    return textureMap[assetHashcode];
}

FontDescriptor* GraphicsAssetManager::getFont( const fnChar_t* assetName, const bool forceReload )
{
    auto file = virtualFileSystem->openFile( assetName, eFileOpenMode::FILE_OPEN_MODE_READ );
    if ( file == nullptr ) {
        FLAN_CERR << "'" << assetName << "' does not exist!" << std::endl;
        return nullptr;
    }

    fnStringHash_t assetHashcode = CRC32( assetName );

    auto mapIterator = fontMap.find( assetHashcode );
    const bool alreadyExists = ( mapIterator != fontMap.end() );

    if ( alreadyExists && !forceReload ) {
        return mapIterator->second;
    }

    if ( !alreadyExists ) {
        fontMap[assetHashcode] = flan::core::allocate<FontDescriptor>( assetStreamingHeap );
    }

    auto font = fontMap[assetHashcode];

    flan::core::LoadFontFile( virtualFileSystem->openFile( assetName ), *font );

    return fontMap[assetHashcode];
}

Material* GraphicsAssetManager::getMaterialCopy( const fnChar_t* assetName )
{
    auto materialLoaded = getMaterial( assetName );
    
    Material* matCopy = nullptr;
    if ( materialLoaded != nullptr ) {
        matCopy = flan::core::allocate<Material>( assetStreamingHeap, *materialLoaded );
        matCopy->create( renderDevice, shaderStageManager );
    }

    return matCopy;
}

Material* GraphicsAssetManager::getMaterial( const fnChar_t* assetName, const bool forceReload )
{
    auto file = virtualFileSystem->openFile( assetName, eFileOpenMode::FILE_OPEN_MODE_READ );
    if ( file == nullptr ) {
        FLAN_CERR << "'" << assetName << "' does not exist!" << std::endl;
        return nullptr;
    }

    auto assetHashcode = file->getHashcode();
    auto mapIterator = materialMap.find( assetHashcode );
    const bool alreadyExists = ( mapIterator != materialMap.end() );

    if ( alreadyExists && !forceReload ) {
        return mapIterator->second;
    }

    if ( !alreadyExists ) {
        materialMap[assetHashcode] = flan::core::allocate<Material>( assetStreamingHeap );
    }

    auto materialInstance = materialMap[assetHashcode];
    materialInstance->deserialize( file, this );
    materialInstance->create( renderDevice, shaderStageManager );

    file->close();

    return materialMap[assetHashcode];
}

Mesh* GraphicsAssetManager::getMesh( const fnChar_t* assetName, const bool forceReload )
{
    auto file = virtualFileSystem->openFile( assetName, eFileOpenMode::FILE_OPEN_MODE_READ | eFileOpenMode::FILE_OPEN_MODE_BINARY );
    if ( file == nullptr ) {
        FLAN_CERR << "'" << assetName << "' does not exist!" << std::endl;
        return nullptr;
    }

    auto assetHashcode = file->getHashcode();
    auto mapIterator = meshMap.find( assetHashcode );
    const bool alreadyExists = ( mapIterator != meshMap.end() );

    if ( alreadyExists && !forceReload ) {
        return mapIterator->second;
    }

    if ( !alreadyExists ) {
        meshMap[assetHashcode] = flan::core::allocate<Mesh>( assetStreamingHeap );
    } else {
        meshMap[assetHashcode]->reset();
    }

    auto meshInstance = meshMap[assetHashcode];
    
    GeomLoadData loadData;
    flan::core::LoadGeometryFile( file, loadData );
    file->close();

    meshInstance->setName( assetName );

    // Allocate VertexBuffer
    BufferDesc vertexBufferDesc;
    vertexBufferDesc.Type = BufferDesc::VERTEX_BUFFER;
    vertexBufferDesc.Size = loadData.vertices.size() * sizeof( float );

    // Compute vertex stride (sum component count per vertex)
    uint32_t vertexBufferStride = 0;
    for ( auto stride : loadData.vertexStrides ) {
        vertexBufferStride += stride;
    }

    vertexBufferDesc.Stride = vertexBufferStride * sizeof( float );

    // Allocate IndiceBuffer
    BufferDesc indiceBufferDesc;
    indiceBufferDesc.Type = BufferDesc::INDICE_BUFFER;
    indiceBufferDesc.Size = loadData.indices.size() * sizeof( uint32_t );
    indiceBufferDesc.Stride = sizeof( uint32_t );

    meshInstance->create( renderDevice, vertexBufferDesc, indiceBufferDesc, loadData.vertices.data(), loadData.indices.data() );
    
    // TODO Custom Distance Definition (define per model LoD distance?)
    constexpr float LOD_DISTANCE[4] = { 50.0f, 100.0f, 256.0f, 512.0f };
    for ( int i = 0; i < 4; i++ )
        meshInstance->addLevelOfDetail( i, LOD_DISTANCE[i] );

    // Build each LevelOfDetail
    for ( GeomLoadData::SubMesh& subMesh : loadData.subMesh ) {
        meshInstance->addSubMesh( subMesh.levelOfDetailIndex, {
            getMaterialCopy( FLAN_STRING( "GameData/Materials/MaterialTest.amat" ) ), 
            subMesh.indiceBufferOffset, 
            subMesh.indiceCount, 
            subMesh.boundingSphere, 
            subMesh.name, 
            subMesh.aabb 
        } );
    }

    return meshInstance;
}

Model* GraphicsAssetManager::getModel( const fnChar_t* assetName, const bool forceReload )
{
    auto file = virtualFileSystem->openFile( assetName, eFileOpenMode::FILE_OPEN_MODE_READ | eFileOpenMode::FILE_OPEN_MODE_BINARY );
    if ( file == nullptr ) {
        FLAN_CERR << "'" << assetName << "' does not exist!" << std::endl;
        return nullptr;
    }

    auto assetHashcode = file->getHashcode();
    auto mapIterator = modelMap.find( assetHashcode );
    const bool alreadyExists = ( mapIterator != modelMap.end() );

    if ( alreadyExists && !forceReload ) {
        return mapIterator->second;
    }

    if ( !alreadyExists ) {
        modelMap[assetHashcode] = flan::core::allocate<Model>( assetStreamingHeap );
    } else {
        modelMap[assetHashcode]->meshes.clear();
        modelMap[assetHashcode]->name.clear();
    }

    auto modelInstance = modelMap[assetHashcode];

    ModelLoadData loadData;
    Io_ReadModelFile( file, loadData );
    file->close();

    modelInstance->name = flan::core::WideStringToString( assetName );
    for ( auto& mesh : loadData.Meshes ) {
        auto loadedMesh = getMesh( mesh.Name.c_str() );

        if ( loadedMesh == nullptr ) {
            continue;
        }

    /*    auto& subMeshes = loadedMesh->getSubMeshVectorRW();
        for ( auto& subMesh : subMeshes ) {
            for ( auto& subMeshName : mesh.SubMeshMaterialMap ) {
                if ( subMesh.name == subMeshName.first ) {
#if FLAN_DEVBUILD
                    subMesh.material = getMaterialCopy( fnString_t( subMeshName.second ).c_str() );
#else
                    subMesh.material = getMaterial( fnString_t( subMeshName.second ).c_str() );
#endif
                }
            }
        }*/

        modelInstance->meshes.push_back( loadedMesh );
    }

    return modelInstance;
}

void GraphicsAssetManager::getImageTexels( const fnChar_t* assetName, GraphicsAssetManager::RawTexels& texels )
{
    auto file = virtualFileSystem->openFile( assetName, eFileOpenMode::FILE_OPEN_MODE_READ | eFileOpenMode::FILE_OPEN_MODE_BINARY );
    if ( file == nullptr ) {
        FLAN_CERR << "'" << assetName << "' does not exist!" << std::endl;
        return;
    }

    auto texFileFormat = GetFileExtensionFromPath( assetName );
    StringToLower( texFileFormat );
    auto texFileFormatHashcode = CRC32( texFileFormat );

    switch ( texFileFormatHashcode ) {
    case FLAN_STRING_HASH( "png16" ):
    case FLAN_STRING_HASH( "hmap" ):
    {
        stbi_io_callbacks callbacks;
        callbacks.read = stbi_readcallback;
        callbacks.skip = stbi_skipcallback;
        callbacks.eof = stbi_eofcallback;

        texels.data = stbi_load_16_from_callbacks( &callbacks, file, &texels.width, &texels.height, &texels.channelCount, STBI_default );
        texels.bytePerPixel = sizeof( stbi_us );

        file->close();
    } break;
    default:
        break;
    }
}
