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

#if NYA_GL460
#include <Rendering/RenderDevice.h>

#include "Extensions.h"

#include <vector>

struct Shader
{
    GLuint  shaderHandle;
    GLenum  shaderType;
};

GLenum GetOpenGLShaderStage( const eShaderStage stage )
{
    switch ( stage ) {
    case eShaderStage::SHADER_STAGE_VERTEX:
        return GL_VERTEX_SHADER;
    case eShaderStage::SHADER_STAGE_PIXEL:
        return GL_FRAGMENT_SHADER;
    case eShaderStage::SHADER_STAGE_TESSELATION_CONTROL:
        return GL_TESS_CONTROL_SHADER;
    case eShaderStage::SHADER_STAGE_TESSELATION_EVALUATION:
        return GL_TESS_EVALUATION_SHADER;
    case eShaderStage::SHADER_STAGE_COMPUTE:
        return GL_COMPUTE_SHADER;
    default:
        return 0;
    }
}

Shader* RenderDevice::createShader( const eShaderStage stage, const void* bytecode, const size_t bytecodeSize )
{
    GLenum shaderType = GetOpenGLShaderStage( stage );
    GLuint shaderHandle = glCreateShader( shaderType );

    glShaderBinary( 1, &shaderHandle, GL_SHADER_BINARY_FORMAT_SPIR_V, bytecode, static_cast<GLsizei>( bytecodeSize ) );
    glSpecializeShader( shaderHandle, "main", 0, nullptr, nullptr );

    GLint isCompiled = 0;
    glGetShaderiv( shaderHandle, GL_COMPILE_STATUS, &isCompiled );
    if ( isCompiled == GL_FALSE ) {
        GLint maxLength = 0;
        glGetShaderiv( shaderHandle, GL_INFO_LOG_LENGTH, &maxLength );

        if ( maxLength != 0 ) {
            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog( maxLength );
            glGetShaderInfoLog( shaderHandle, maxLength, &maxLength, &infoLog[0] );

            NYA_CERR << "Precompiled Shader parsing failed!" << std::endl;
            NYA_CERR << infoLog.data() << std::endl;
        }

        glDeleteShader( shaderHandle );

        return nullptr;
    }

    Shader* shader = nya::core::allocate<Shader>( memoryAllocator );
    shader->shaderHandle = shaderHandle;
    shader->shaderType = shaderType;

    return shader;
}

void RenderDevice::destroyShader( Shader* shader )
{
    glDeleteShader( shader->shaderHandle );
}
#endif
