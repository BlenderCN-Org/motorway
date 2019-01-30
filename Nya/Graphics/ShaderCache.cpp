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
#include "ShaderCache.h"

#include <Core/Hashing/MurmurHash3.h>

#include <Rendering/RenderDevice.h>

#include <FileSystem/VirtualFileSystem.h>
#include <FileSystem/FileSystemObject.h>

#include <Io/Binary.h>

using namespace nya::core;

ShaderCache::ShaderCache( BaseAllocator* allocator, RenderDevice* activeRenderDevice, VirtualFileSystem* activeVFS )
    : virtualFileSystem( activeVFS )
    , renderDevice( activeRenderDevice )
    , memoryAllocator( allocator )
{
    defaultVertexStage = getOrUploadStage( "Error", eShaderStage::SHADER_STAGE_VERTEX );
    defaultPixelStage = getOrUploadStage( "Error", eShaderStage::SHADER_STAGE_PIXEL );
    defaultComputeStage = getOrUploadStage( "Error", eShaderStage::SHADER_STAGE_COMPUTE );
}

ShaderCache::~ShaderCache()
{
    for ( auto& shader : cachedStages ) {
        renderDevice->destroyShader( shader.second );
    }
    cachedStages.clear();

    defaultVertexStage = nullptr;
    defaultPixelStage = nullptr;
    defaultComputeStage = nullptr;

    virtualFileSystem = nullptr;
    renderDevice = nullptr;
    memoryAllocator = nullptr;
}

struct Hash128
{
    union
    {
        struct
        {
            uint64_t low;
            uint64_t high;
        };

        uint64_t data[2];
    };
};

nyaString_t GetHashcodeDigest64( const uint64_t hashcode64 )
{
    static constexpr size_t HASHCODE_SIZE = sizeof( uint64_t ) << 1;
    static constexpr nyaChar_t* LUT = NYA_STRING( "0123456789abcdef" );
    
    nyaString_t digest( HASHCODE_SIZE, NYA_STRING( '0' ) );
    for ( size_t i = 0, j = ( HASHCODE_SIZE - 1 ) * 4; i < HASHCODE_SIZE; ++i, j -= 4 ) {
        digest[i] = LUT[( hashcode64 >> j ) & 0x0f];
    }

    return digest;
}

nyaString_t GetHashcodeDigest128( const Hash128& hashcode128 )
{
    static constexpr size_t HASHCODE_SIZE = sizeof( Hash128 ) << 1;
    static constexpr size_t HALF_HASHCODE_SIZE = ( HASHCODE_SIZE / 2 );
    static constexpr nyaChar_t* LUT = NYA_STRING( "0123456789abcdef" );
    nyaString_t digest( HASHCODE_SIZE, NYA_STRING( '0' ) );
    for ( size_t i = 0, j = ( HASHCODE_SIZE - 1 ) * 4; i < HASHCODE_SIZE; ++i, j -= 4 ) {
        // NOTE Somehow, MSVC tries to unroll the loop but doesn't write the correct index (the div
        // is skipped)
        // Declaring explicitly the index seems to do the trick
        const size_t index = ( i / HALF_HASHCODE_SIZE );
        digest[i] = LUT[( hashcode128.data[index] >> j ) & 0x0f];
    }

    return digest;
}

Shader* ShaderCache::getOrUploadStage( const std::string& shaderFilename, const eShaderStage stageType, const bool forceReload )
{
    Hash128 permutationHashcode;
    MurmurHash3_x64_128( shaderFilename.c_str(), static_cast<int>( shaderFilename.size() ), 19081996, &permutationHashcode );

    nyaString_t filenameWithExtension = GetHashcodeDigest128( permutationHashcode );

#if NYA_VULKAN
    switch ( stageType ) {
    case eShaderStage::SHADER_STAGE_VERTEX:
        filenameWithExtension.append( NYA_STRING( ".vso.vk" ) );
        break;
    case eShaderStage::SHADER_STAGE_PIXEL:
        filenameWithExtension.append( NYA_STRING( ".pso.vk" ) );
        break;
    case eShaderStage::SHADER_STAGE_COMPUTE:
        filenameWithExtension.append( NYA_STRING( ".cso.vk" ) );
        break;
    case eShaderStage::SHADER_STAGE_TESSELATION_CONTROL:
        filenameWithExtension.append( NYA_STRING( ".hso.vk" ) );
        break;
    case eShaderStage::SHADER_STAGE_TESSELATION_EVALUATION:
        filenameWithExtension.append( NYA_STRING( ".dso.vk" ) );
        break;
    }
#elif NYA_D3D11
        switch ( stageType ) {
        case eShaderStage::SHADER_STAGE_VERTEX:
            filenameWithExtension.append( NYA_STRING( ".vso" ) );
            break;
        case eShaderStage::SHADER_STAGE_PIXEL:
            filenameWithExtension.append( NYA_STRING( ".pso" ) );
            break;
        case eShaderStage::SHADER_STAGE_COMPUTE:
            filenameWithExtension.append( NYA_STRING( ".cso" ) );
            break;
        case eShaderStage::SHADER_STAGE_TESSELATION_CONTROL:
            filenameWithExtension.append( NYA_STRING( ".hso" ) );
            break;
        case eShaderStage::SHADER_STAGE_TESSELATION_EVALUATION:
            filenameWithExtension.append( NYA_STRING( ".dso" ) );
            break;
        }
#endif

    FileSystemObject* file = virtualFileSystem->openFile( 
        NYA_STRING( "GameData/shaders/" ) + filenameWithExtension, 
        eFileOpenMode::FILE_OPEN_MODE_READ | eFileOpenMode::FILE_OPEN_MODE_BINARY 
    );

    if ( file == nullptr ) {
        NYA_CERR << "'" << filenameWithExtension << "': file does not exist!" << std::endl;

        // Returns shader fallback
        switch ( stageType ) {
        case eShaderStage::SHADER_STAGE_VERTEX:
            return defaultVertexStage;
        case eShaderStage::SHADER_STAGE_PIXEL:
            return defaultPixelStage;
        case eShaderStage::SHADER_STAGE_COMPUTE:
            return defaultComputeStage;
        case eShaderStage::SHADER_STAGE_TESSELATION_CONTROL:
        case eShaderStage::SHADER_STAGE_TESSELATION_EVALUATION:
        default:
            return nullptr;
        }
    }

    const auto shaderHashcode = file->getHashcode();

    auto it = cachedStages.find( shaderHashcode );
    if ( it != cachedStages.end() && !forceReload ) {
        file->close();
        return it->second;
    }

    auto shader = cachedStages[shaderHashcode];
    if ( it != cachedStages.end() ) {
        // If asked for force reloading, destroy previously cached shader instance
        renderDevice->destroyShader( it->second );
    }

    // Load precompiled shader
    {
        std::vector<uint8_t> precompiledShader;
        nya::core::LoadBinaryFile( file, precompiledShader );
        file->close();

        cachedStages[shaderHashcode] = renderDevice->createShader( stageType, precompiledShader.data(), precompiledShader.size() );
    }

	return cachedStages[shaderHashcode];
}
