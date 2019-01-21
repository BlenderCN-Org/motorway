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

#include <Core/Hashing/CRC32.h>

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

Shader* ShaderCache::getOrUploadStage( const nyaString_t& shaderFilename, const eShaderStage stageType, const bool forceReload )
{
    auto filenameWithExtension = shaderFilename;

#if NYA_GL460
    switch ( stageType ) {
    case eShaderStage::SHADER_STAGE_VERTEX:
        filenameWithExtension.append( NYA_STRING( ".gl.spvv" ) );
        break;
    case eShaderStage::SHADER_STAGE_PIXEL:
        filenameWithExtension.append( NYA_STRING( ".gl.spvp" ) );
        break;
    case eShaderStage::SHADER_STAGE_COMPUTE:
        filenameWithExtension.append( NYA_STRING( ".gl.spvc" ) );
        break;
    case eShaderStage::SHADER_STAGE_TESSELATION_CONTROL:
        filenameWithExtension.append( NYA_STRING( ".gl.spvtc" ) );
        break;
    case eShaderStage::SHADER_STAGE_TESSELATION_EVALUATION:
        filenameWithExtension.append( NYA_STRING( ".gl.spvte" ) );
        break;
    }
#elif NYA_VULKAN
    switch ( stageType ) {
    case eShaderStage::SHADER_STAGE_VERTEX:
        filenameWithExtension.append( NYA_STRING( ".vk.spvv" ) );
        break;
    case eShaderStage::SHADER_STAGE_PIXEL:
        filenameWithExtension.append( NYA_STRING( ".vk.spvp" ) );
        break;
    case eShaderStage::SHADER_STAGE_COMPUTE:
        filenameWithExtension.append( NYA_STRING( ".vk.spvc" ) );
        break;
    case eShaderStage::SHADER_STAGE_TESSELATION_CONTROL:
        filenameWithExtension.append( NYA_STRING( ".vk.spvtc" ) );
        break;
    case eShaderStage::SHADER_STAGE_TESSELATION_EVALUATION:
        filenameWithExtension.append( NYA_STRING( ".vk.spvte" ) );
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
