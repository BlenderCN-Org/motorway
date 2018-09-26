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

#if FLAN_GL460
#include "Shader.h"

#include <vector>

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

NativeShaderObject* flan::rendering::CreateAndCompileShaderImpl( NativeRenderContext* nativeRenderContext, const eShaderStage shaderStage, const char* shaderToBeCompiled, const std::size_t shaderToBeCompiledLength )
{
    GLenum shaderType = GetOpenGLShaderStage( shaderStage );
    GLuint shaderHandle = glCreateShader( shaderType );

    glShaderSource( shaderHandle, 1, &shaderToBeCompiled, NULL );
    glCompileShader( shaderHandle );

    GLint isCompiled = 0;
    glGetShaderiv( shaderHandle, GL_COMPILE_STATUS, &isCompiled );
    if ( isCompiled == GL_FALSE ) {
        FLAN_CERR << "Shader compilation failed!" << std::endl;

        GLint maxLength = 0;
        glGetShaderiv( shaderHandle, GL_INFO_LOG_LENGTH, &maxLength );

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog( maxLength );
        glGetShaderInfoLog( shaderHandle, maxLength, &maxLength, &infoLog[0] );

        FLAN_CERR << infoLog.data() << std::endl;

        glDeleteShader( shaderHandle );

        return nullptr;
    }

    NativeShaderObject* nativeShaderObject = new NativeShaderObject();
    nativeShaderObject->shaderHandle = shaderHandle;
    nativeShaderObject->shaderType = shaderType;

    return nativeShaderObject;
}

NativeShaderObject* flan::rendering::CreatePrecompiledShaderImpl( NativeRenderContext* nativeRenderContext, const eShaderStage shaderStage,  const uint8_t* precompiledShader, const std::size_t precompiledShaderLength )
{
    GLenum shaderType = GetOpenGLShaderStage( shaderStage );
    GLuint shaderHandle = glCreateShader( shaderType );

    glShaderBinary( 1, &shaderHandle, GL_SHADER_BINARY_FORMAT_SPIR_V, precompiledShader, static_cast<GLsizei>( precompiledShaderLength ) );
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

            FLAN_CERR << "Precompiled Shader parsing failed!" << std::endl;
            FLAN_CERR << infoLog.data() << std::endl;
        }

        glDeleteShader( shaderHandle );

        return nullptr;
    }

    NativeShaderObject* nativeShaderObject = new NativeShaderObject();
    nativeShaderObject->shaderHandle = shaderHandle;
    nativeShaderObject->shaderType = shaderType;

    return nativeShaderObject;
}

void flan::rendering::DestroyShaderImpl( NativeRenderContext* nativeRenderContext, NativeShaderObject* shaderObject )
{
    if ( shaderObject->shaderType != 0 ) {
        glDeleteShader( shaderObject->shaderHandle );
        shaderObject->shaderHandle = 0;
    }

    shaderObject->shaderType = 0;
}
#endif
