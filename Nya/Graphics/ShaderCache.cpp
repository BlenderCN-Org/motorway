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

}

ShaderCache::~ShaderCache()
{
    for ( auto& shader : cachedStages ) {
        renderDevice->destroyShader( shader.second );
    }
}

void GetHashDigest( void* hashcode, char *s, bool lowerAlpha )
{
    static constexpr char digitsLowerAlpha[513] =
        "000102030405060708090a0b0c0d0e0f"
        "101112131415161718191a1b1c1d1e1f"
        "202122232425262728292a2b2c2d2e2f"
        "303132333435363738393a3b3c3d3e3f"
        "404142434445464748494a4b4c4d4e4f"
        "505152535455565758595a5b5c5d5e5f"
        "606162636465666768696a6b6c6d6e6f"
        "707172737475767778797a7b7c7d7e7f"
        "808182838485868788898a8b8c8d8e8f"
        "909192939495969798999a9b9c9d9e9f"
        "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
        "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
        "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
        "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
        "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";

    uint32_t x = ( uint32_t )hashcode;
    int i = 12;
    while ( i >= 0 ) {
        int pos = ( x & 0xFF ) * 2;
        char ch = digitsLowerAlpha[pos];
        s[i * 2] = ch;

        ch = digitsLowerAlpha[pos + 1];
        s[i * 2 + 1] = ch;

        x >>= 8;
        i -= 1;
    }
}

Shader* ShaderCache::getOrUploadStage( const std::string& shaderFilename, const eShaderStage stageType, const bool forceReload )
{
    auto filenameWithExtension = shaderFilename;

    struct {
        uint64_t low;
        uint64_t high;
    } permutationHashcode;

    MurmurHash3_x64_128( shaderFilename.c_str(), shaderFilename.size(), 19081996, &permutationHashcode );

    // - Compute hash digest (permutationHash To string)
    // - Append stage extension
    // - Check if available
    // - Do the regular shader loading
    return nullptr;

//
//#if NYA_VULKAN
//    switch ( stageType ) {
//    case eShaderStage::SHADER_STAGE_VERTEX:
//        filenameWithExtension.append( NYA_STRING( ".vso.vk" ) );
//        break;
//    case eShaderStage::SHADER_STAGE_PIXEL:
//        filenameWithExtension.append( NYA_STRING( ".pso.vk" ) );
//        break;
//    case eShaderStage::SHADER_STAGE_COMPUTE:
//        filenameWithExtension.append( NYA_STRING( ".cso.vk" ) );
//        break;
//    case eShaderStage::SHADER_STAGE_TESSELATION_CONTROL:
//        filenameWithExtension.append( NYA_STRING( ".hso.vk" ) );
//        break;
//    case eShaderStage::SHADER_STAGE_TESSELATION_EVALUATION:
//        filenameWithExtension.append( NYA_STRING( ".dso.vk" ) );
//        break;
//    }
//#elif NYA_D3D11
//        switch ( stageType ) {
//        case eShaderStage::SHADER_STAGE_VERTEX:
//            filenameWithExtension.append( NYA_STRING( ".vso" ) );
//            break;
//        case eShaderStage::SHADER_STAGE_PIXEL:
//            filenameWithExtension.append( NYA_STRING( ".pso" ) );
//            break;
//        case eShaderStage::SHADER_STAGE_COMPUTE:
//            filenameWithExtension.append( NYA_STRING( ".cso" ) );
//            break;
//        case eShaderStage::SHADER_STAGE_TESSELATION_CONTROL:
//            filenameWithExtension.append( NYA_STRING( ".hso" ) );
//            break;
//        case eShaderStage::SHADER_STAGE_TESSELATION_EVALUATION:
//            filenameWithExtension.append( NYA_STRING( ".dso" ) );
//            break;
//        }
//#endif
//
//    FileSystemObject* file = virtualFileSystem->openFile( 
//        NYA_STRING( "GameData/shaders/" ) + filenameWithExtension, 
//        eFileOpenMode::FILE_OPEN_MODE_READ | eFileOpenMode::FILE_OPEN_MODE_BINARY 
//    );
//
//    if ( file == nullptr ) {
//        NYA_CERR << "'" << filenameWithExtension << "': file does not exist!" << std::endl;
//        return nullptr;
//    }
//
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
