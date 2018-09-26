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
#include "ShaderStageManager.h"

#include <Core/Hashing/CRC32.h>

#include <Rendering/RenderDevice.h>
#include <Rendering/Shader.h>

#include <FileSystem/VirtualFileSystem.h>
#include <Io/Text.h>
#include <Io/Binary.h>

using namespace flan::core;

ShaderStageManager::ShaderStageManager( RenderDevice* activeRenderDevice, VirtualFileSystem* activeVFS )
    : virtualFileSystem( activeVFS )
    , renderDevice( activeRenderDevice )
{

}

ShaderStageManager::~ShaderStageManager()
{
    virtualFileSystem = nullptr;

    for ( auto& shader : cachedStages ) {
        shader.second->destroy( renderDevice );
    }

    cachedStages.clear();
}

Shader* ShaderStageManager::getOrUploadStage( const fnString_t& shaderFilename, const eShaderStage stageType, const bool forceReload )
{
    auto filenameWithExtension = shaderFilename;

#if FLAN_GL460
    switch ( stageType ) {
    case eShaderStage::SHADER_STAGE_VERTEX:
        filenameWithExtension.append( FLAN_STRING( ".gl.spvv" ) );
        break;
    case eShaderStage::SHADER_STAGE_PIXEL:
        filenameWithExtension.append( FLAN_STRING( ".gl.spvp" ) );
        break;
    case eShaderStage::SHADER_STAGE_COMPUTE:
        filenameWithExtension.append( FLAN_STRING( ".gl.spvc" ) );
        break;
    case eShaderStage::SHADER_STAGE_TESSELATION_CONTROL:
        filenameWithExtension.append( FLAN_STRING( ".gl.spvtc" ) );
        break;
    case eShaderStage::SHADER_STAGE_TESSELATION_EVALUATION:
        filenameWithExtension.append( FLAN_STRING( ".gl.spvte" ) );
        break;
    }
#elif FLAN_VULKAN
    switch ( stageType ) {
    case eShaderStage::SHADER_STAGE_VERTEX:
        filenameWithExtension.append( FLAN_STRING( ".vk.spvv" ) );
        break;
    case eShaderStage::SHADER_STAGE_PIXEL:
        filenameWithExtension.append( FLAN_STRING( ".vk.spvp" ) );
        break;
    case eShaderStage::SHADER_STAGE_COMPUTE:
        filenameWithExtension.append( FLAN_STRING( ".vk.spvc" ) );
        break;
    case eShaderStage::SHADER_STAGE_TESSELATION_CONTROL:
        filenameWithExtension.append( FLAN_STRING( ".vk.spvtc" ) );
        break;
    case eShaderStage::SHADER_STAGE_TESSELATION_EVALUATION:
        filenameWithExtension.append( FLAN_STRING( ".vk.spvte" ) );
        break;
    }
#elif FLAN_D3D11
        switch ( stageType ) {
        case eShaderStage::SHADER_STAGE_VERTEX:
            filenameWithExtension.append( FLAN_STRING( ".vso" ) );
            break;
        case eShaderStage::SHADER_STAGE_PIXEL:
            filenameWithExtension.append( FLAN_STRING( ".pso" ) );
            break;
        case eShaderStage::SHADER_STAGE_COMPUTE:
            filenameWithExtension.append( FLAN_STRING( ".cso" ) );
            break;
        case eShaderStage::SHADER_STAGE_TESSELATION_CONTROL:
            filenameWithExtension.append( FLAN_STRING( ".tco" ) );
            break;
        case eShaderStage::SHADER_STAGE_TESSELATION_EVALUATION:
            filenameWithExtension.append( FLAN_STRING( ".teo" ) );
            break;
        }
#endif

    bool useCompiledShader = true;
    auto file = virtualFileSystem->openFile( FLAN_STRING( "GameData/CompiledShaders/" ) + filenameWithExtension, eFileOpenMode::FILE_OPEN_MODE_READ | eFileOpenMode::FILE_OPEN_MODE_BINARY );

    if ( file == nullptr ) {
        useCompiledShader = false;

        // Compile from source (e.g. when using GLSL)
        FLAN_CWARN << "'" << shaderFilename << "': compiled shader does not exist! Trying to load source..." << std::endl;
        file = virtualFileSystem->openFile( FLAN_STRING( "GameData/Shaders/" ) + shaderFilename + FLAN_STRING( ".lsl" ) );

        if ( file == nullptr ) {
            FLAN_CERR << "'" << shaderFilename << "': shader does not exist (compiled/shader source does not exist)" << std::endl;
            return nullptr;
        }
    }

    const auto shaderHashcode = file->getHashcode();

    auto it = cachedStages.find( shaderHashcode );
    if ( it != cachedStages.end() && !forceReload ) {
        file->close();
        return it->second.get();
    }

    // Load shader if it does not exist yet
    bool alreadyExists = ( it != cachedStages.end() );

    if ( !alreadyExists ) {
        cachedStages[shaderHashcode] = std::make_unique<Shader>( stageType );
    }

    auto shader = cachedStages[shaderHashcode].get();
    if ( !useCompiledShader ) {
        std::string shaderContent;
        flan::core::LoadTextFile( file, shaderContent );
        file->close();

        shader->create( renderDevice, shaderContent.c_str(), shaderContent.size() );
    } else {
        std::vector<uint8_t> precompiledShader;
        flan::core::LoadBinaryFile( file, precompiledShader );
        file->close();

        // Load precompiled shader
        shader->create( renderDevice, precompiledShader.data(), precompiledShader.size() );
    }

	return cachedStages[shaderHashcode].get();
}
