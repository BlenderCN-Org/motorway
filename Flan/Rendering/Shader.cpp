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
#include <Shared.h>
#include "Shader.h"

#include "RenderDevice.h"

#if FLAN_GL460
#include "OpenGL460/RenderContext.h"
#include "OpenGL460/Shader.h"
#elif FLAN_D3D11
#include "Direct3D11/RenderContext.h"
#include "Direct3D11/Shader.h"
#elif FLAN_VULKAN
#include "Vulkan/RenderContext.h"
#include "Vulkan/CommandList.h"
#include "Vulkan/Shader.h"
#endif

Shader::Shader( const eShaderStage shaderStage )
    : nativeShaderObject( nullptr )
    , stage( shaderStage )
{

}

Shader::~Shader()
{

}

void Shader::create( RenderDevice* renderDevice, const char* shaderToBeCompiled, const std::size_t shaderToBeCompiledLength )
{
    nativeShaderObject.reset( flan::rendering::CreateAndCompileShaderImpl( renderDevice->getNativeRenderContext(), stage, shaderToBeCompiled, shaderToBeCompiledLength ) );
}

void Shader::create( RenderDevice* renderDevice, const uint8_t* precompiledShader, const std::size_t precompiledShaderLength )
{
    nativeShaderObject.reset( flan::rendering::CreatePrecompiledShaderImpl( renderDevice->getNativeRenderContext(), stage, precompiledShader, precompiledShaderLength ) );

    shaderBytecode.resize( precompiledShaderLength );
    memcpy( &shaderBytecode[0], precompiledShader, precompiledShaderLength );
}

void Shader::destroy( RenderDevice* renderDevice )
{
    flan::rendering::DestroyShaderImpl( renderDevice->getNativeRenderContext(), nativeShaderObject.get() );
}

NativeShaderObject* Shader::getNativeShaderObject() const
{
    return nativeShaderObject.get();
}

const std::vector<uint8_t>* Shader::getShaderBytecode() const
{
    return &shaderBytecode;
}
