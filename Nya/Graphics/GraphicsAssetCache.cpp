/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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
#include "GraphicsAssetCache.h"

// Io
#include <FileSystem/VirtualFileSystem.h>
#include <FileSystem/FileSystemObject.h>

#include <Io/DirectDrawSurface.h>
#include <Io/FontDescriptor.h>
#include <Io/Mesh.h>
//#include <Io/Model.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#include <Core/Allocators/BaseAllocator.h>
#include <Core/Allocators/FreeListAllocator.h>

// Assets
#include <Rendering/RenderDevice.h>
#include <Rendering/ImageFormat.h>

#include <Framework/Mesh.h>
#include <Framework/Material.h>

#include <Core/StringHelpers.h>

using namespace nya::core;

GraphicsAssetCache::GraphicsAssetCache( BaseAllocator* allocator, RenderDevice* renderDevice, ShaderCache* shaderCache, VirtualFileSystem* virtualFileSystem )
    : assetStreamingHeap( nya::core::allocate<FreeListAllocator>( allocator, 32 * 1024 * 1024, allocator->allocate( 32 * 1024 * 1024 ) ) )
    , renderDevice( renderDevice )
    , shaderCache( shaderCache )
    , virtualFileSystem( virtualFileSystem )
{
    defaultMaterial = getMaterial( NYA_STRING( "GameData/materials/DefaultMaterial.mat" ) );
}

GraphicsAssetCache::~GraphicsAssetCache()
{
    meshMap.clear();
    materialMap.clear();
    fontMap.clear();
    textureMap.clear();
}

void GraphicsAssetCache::destroy()
{
    for ( auto& meshes : meshMap ) {
        meshes.second->destroy( renderDevice );
    }

    for ( auto& mat : materialMap ) {
        mat.second->destroy( renderDevice );
    }

    for ( auto& texture : textureMap ) {
        renderDevice->destroyTexture( texture.second );
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

Texture* GraphicsAssetCache::getTexture( const nyaChar_t* assetName, const bool forceReload )
{
    auto file = virtualFileSystem->openFile( assetName, eFileOpenMode::FILE_OPEN_MODE_READ | eFileOpenMode::FILE_OPEN_MODE_BINARY );
    if ( file == nullptr ) {
        NYA_CERR << "'" << assetName << "' does not exist!" << std::endl;
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
    case NYA_STRING_HASH( "dds" ): {
        DirectDrawSurface ddsData;
        LoadDirectDrawSurface( file, ddsData );

        if ( alreadyExists ) {
            renderDevice->destroyTexture( textureMap[assetHashcode] );
        }

        switch ( ddsData.textureDescription.dimension ) {
        case TextureDescription::DIMENSION_TEXTURE_1D:
            textureMap[assetHashcode] = renderDevice->createTexture1D( ddsData.textureDescription, ddsData.textureData.data(), ddsData.textureData.size() / ddsData.textureDescription.height );
            break;
        case TextureDescription::DIMENSION_TEXTURE_2D:
            textureMap[assetHashcode] = renderDevice->createTexture2D( ddsData.textureDescription, ddsData.textureData.data(), ddsData.textureData.size() / ddsData.textureDescription.height );
            break;
        case TextureDescription::DIMENSION_TEXTURE_3D:
            textureMap[assetHashcode] = renderDevice->createTexture3D( ddsData.textureDescription, ddsData.textureData.data(), ddsData.textureData.size() / ddsData.textureDescription.height );
            break;
        }
    } break;

    case NYA_STRING_HASH( "png16" ):
    case NYA_STRING_HASH( "hmap" ): {
        stbi_io_callbacks callbacks;
        callbacks.read = stbi_readcallback;
        callbacks.skip = stbi_skipcallback;
        callbacks.eof = stbi_eofcallback;

        int w;
        int h;
        int comp;
        auto* image = stbi_load_16_from_callbacks( &callbacks, file, &w, &h, &comp, STBI_default );

        if ( alreadyExists ) {
            renderDevice->destroyTexture( textureMap[assetHashcode] );
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

        textureMap[assetHashcode] = renderDevice->createTexture2D( desc, image, w * comp );

        stbi_image_free( image );
    } break;
    
    case NYA_STRING_HASH( "jpg" ):
    case NYA_STRING_HASH( "jpeg" ):
    case NYA_STRING_HASH( "png" ):
    case NYA_STRING_HASH( "tga" ):
    case NYA_STRING_HASH( "bmp" ):
    case NYA_STRING_HASH( "psd" ):
    case NYA_STRING_HASH( "gif" ):
    case NYA_STRING_HASH( "hdr" ): {
        stbi_io_callbacks callbacks;
        callbacks.read = stbi_readcallback;
        callbacks.skip = stbi_skipcallback;
        callbacks.eof = stbi_eofcallback;

        int w;
        int h;
        int comp;
        unsigned char* image = stbi_load_from_callbacks( &callbacks, file, &w, &h, &comp, STBI_default );

        if ( alreadyExists ) {
            renderDevice->destroyTexture( textureMap[assetHashcode] );
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

        textureMap[assetHashcode] = renderDevice->createTexture2D( desc, image, w * comp );

        stbi_image_free( image );
    } break;

    default:
        NYA_CERR << "Unsupported fileformat with extension: " << texFileFormat << std::endl;
        return nullptr;
    }

    file->close();
    renderDevice->setDebugMarker( textureMap[assetHashcode], WideStringToString( assetName ).c_str() );

    return textureMap[assetHashcode];
}

FontDescriptor* GraphicsAssetCache::getFont( const nyaChar_t* assetName, const bool forceReload )
{
    auto file = virtualFileSystem->openFile( assetName, eFileOpenMode::FILE_OPEN_MODE_READ );
    if ( file == nullptr ) {
        NYA_CERR << "'" << assetName << "' does not exist!" << std::endl;
        return nullptr;
    }

    nyaStringHash_t assetHashcode = CRC32( assetName );

    auto mapIterator = fontMap.find( assetHashcode );
    const bool alreadyExists = ( mapIterator != fontMap.end() );

    if ( alreadyExists && !forceReload ) {
        return mapIterator->second;
    }

    if ( !alreadyExists ) {
        fontMap[assetHashcode] = nya::core::allocate<FontDescriptor>( assetStreamingHeap );
    }

    auto font = fontMap[assetHashcode];

    nya::core::LoadFontFile( virtualFileSystem->openFile( assetName ), *font );

    return fontMap[assetHashcode];
}

Material* GraphicsAssetCache::getMaterialCopy( const nyaChar_t* assetName )
{
    Material* materialLoaded = getMaterial( assetName );

    Material* matCopy = nya::core::allocate<Material>( assetStreamingHeap, *materialLoaded );
    matCopy->create( renderDevice, shaderCache );

    return matCopy;
}

Material* GraphicsAssetCache::getMaterial( const nyaChar_t* assetName, const bool forceReload )
{
    auto file = virtualFileSystem->openFile( assetName, eFileOpenMode::FILE_OPEN_MODE_READ );
    if ( file == nullptr ) {
        NYA_CERR << "'" << assetName << "' does not exist!" << std::endl;
        return defaultMaterial;
    }

    auto assetHashcode = file->getHashcode();
    auto mapIterator = materialMap.find( assetHashcode );
    const bool alreadyExists = ( mapIterator != materialMap.end() );

    if ( alreadyExists && !forceReload ) {
        return mapIterator->second;
    }

    if ( !alreadyExists ) {
        materialMap[assetHashcode] = nya::core::allocate<Material>( assetStreamingHeap );
    }

    auto materialInstance = materialMap[assetHashcode];
    materialInstance->load( file, this );
    materialInstance->create( renderDevice, shaderCache );

    file->close();

    return materialMap[assetHashcode];
}

Mesh* GraphicsAssetCache::getMesh( const nyaChar_t* assetName, const bool forceReload )
{
    auto file = virtualFileSystem->openFile( assetName, eFileOpenMode::FILE_OPEN_MODE_READ | eFileOpenMode::FILE_OPEN_MODE_BINARY );
    if ( file == nullptr ) {
        NYA_CERR << "'" << assetName << "' does not exist!" << std::endl;
        return nullptr;
    }

    auto assetHashcode = file->getHashcode();
    auto mapIterator = meshMap.find( assetHashcode );
    const bool alreadyExists = ( mapIterator != meshMap.end() );

    if ( alreadyExists && !forceReload ) {
        return mapIterator->second;
    }

    if ( !alreadyExists ) {
        meshMap[assetHashcode] = nya::core::allocate<Mesh>( assetStreamingHeap );
    } else {
        meshMap[assetHashcode]->reset();
    }

    auto meshInstance = meshMap[assetHashcode];
    
    GeomLoadData loadData;
    nya::core::LoadGeometryFile( file, loadData );
    file->close();

    meshInstance->setName( assetName );

    // Allocate VertexBuffer
    BufferDesc vertexBufferDesc;
    vertexBufferDesc.type = BufferDesc::VERTEX_BUFFER;
    vertexBufferDesc.size = loadData.vertices.size() * sizeof( float );

    // Compute vertex stride (sum component count per vertex)
    uint32_t vertexBufferStride = 0;
    for ( auto stride : loadData.vertexStrides ) {
        vertexBufferStride += stride;
    }

    vertexBufferDesc.stride = vertexBufferStride * sizeof( float );

    // Allocate IndiceBuffer
    BufferDesc indiceBufferDesc;
    indiceBufferDesc.type = BufferDesc::INDICE_BUFFER;
    indiceBufferDesc.size = loadData.indices.size() * sizeof( uint32_t );
    indiceBufferDesc.stride = sizeof( uint32_t );

    meshInstance->create( renderDevice, vertexBufferDesc, indiceBufferDesc, loadData.vertices.data(), loadData.indices.data() );
    
    // TODO Custom Distance Definition (define per model LoD distance?)
    constexpr float LOD_DISTANCE[4] = { 250.0f, 500.0f, 1000.0f, 2048.0f };
    for ( int i = 0; i < 1; i++ )
        meshInstance->addLevelOfDetail( i, LOD_DISTANCE[i] );

    // Build each LevelOfDetail
    for ( GeomLoadData::SubMesh& subMesh : loadData.subMesh ) {
        meshInstance->addSubMesh( subMesh.levelOfDetailIndex, {
            defaultMaterial,
            subMesh.indiceBufferOffset, 
            subMesh.indiceCount, 
            subMesh.boundingSphere,
            subMesh.aabb
        } );
    }

    return meshInstance;
}

//
//Model* GraphicsAssetCache::getModel( const nyaChar_t* assetName, const bool forceReload )
//{
//    auto file = virtualFileSystem->openFile( assetName, eFileOpenMode::FILE_OPEN_MODE_READ | eFileOpenMode::FILE_OPEN_MODE_BINARY );
//    if ( file == nullptr ) {
//        NYA_CERR << "'" << assetName << "' does not exist!" << std::endl;
//        return nullptr;
//    }
//
//    auto assetHashcode = file->getHashcode();
//    auto mapIterator = modelMap.find( assetHashcode );
//    const bool alreadyExists = ( mapIterator != modelMap.end() );
//
//    if ( alreadyExists && !forceReload ) {
//        return mapIterator->second;
//    }
//
//    if ( !alreadyExists ) {
//        modelMap[assetHashcode] = nya::core::allocate<Model>( assetStreamingHeap );
//    } else {
//        modelMap[assetHashcode]->meshes.clear();
//        modelMap[assetHashcode]->name.clear();
//    }
//
//    auto modelInstance = modelMap[assetHashcode];
//
//    ModelLoadData loadData;
//    Io_ReadModelFile( file, loadData );
//    file->close();
//
//    modelInstance->name = nya::core::WideStringToString( assetName );
//    for ( auto& mesh : loadData.Meshes ) {
//        auto loadedMesh = getMesh( mesh.Name.c_str() );
//
//        if ( loadedMesh == nullptr ) {
//            continue;
//        }
//
//    /*    auto& subMeshes = loadedMesh->getSubMeshVectorRW();
//        for ( auto& subMesh : subMeshes ) {
//            for ( auto& subMeshName : mesh.SubMeshMaterialMap ) {
//                if ( subMesh.name == subMeshName.first ) {
//#if NYA_DEVBUILD
//                    subMesh.material = getMaterialCopy( fnString_t( subMeshName.second ).c_str() );
//#else
//                    subMesh.material = getMaterial( fnString_t( subMeshName.second ).c_str() );
//#endif
//                }
//            }
//        }*/
//
//        modelInstance->meshes.push_back( loadedMesh );
//    }
//
//    return modelInstance;
//}
//
//void GraphicsAssetCache::getImageTexels( const nyaChar_t* assetName, GraphicsAssetCache::RawTexels& texels )
//{
//    auto file = virtualFileSystem->openFile( assetName, eFileOpenMode::FILE_OPEN_MODE_READ | eFileOpenMode::FILE_OPEN_MODE_BINARY );
//    if ( file == nullptr ) {
//        NYA_CERR << "'" << assetName << "' does not exist!" << std::endl;
//        return;
//    }
//
//    auto texFileFormat = GetFileExtensionFromPath( assetName );
//    StringToLower( texFileFormat );
//    auto texFileFormatHashcode = CRC32( texFileFormat );
//
//    switch ( texFileFormatHashcode ) {
//    case NYA_STRING_HASH( "png16" ):
//    case NYA_STRING_HASH( "hmap" ):
//    {
//        stbi_io_callbacks callbacks;
//        callbacks.read = stbi_readcallback;
//        callbacks.skip = stbi_skipcallback;
//        callbacks.eof = stbi_eofcallback;
//
//        texels.data = stbi_load_16_from_callbacks( &callbacks, file, &texels.width, &texels.height, &texels.channelCount, STBI_default );
//        texels.bytePerPixel = sizeof( stbi_us );
//
//        file->close();
//    } break;
//    default:
//        break;
//    }
//}
