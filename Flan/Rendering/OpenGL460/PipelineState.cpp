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
#include "PipelineState.h"
#include "Shader.h"

#include <vector>

bool IsProgramCompiled( const GLuint programHandle )
{
    GLint isLinked = 0;
    glGetProgramiv( programHandle, GL_LINK_STATUS, &isLinked );
    if ( isLinked == GL_FALSE ) {
        GLint maxLength = 0;
        glGetProgramiv( programHandle, GL_INFO_LOG_LENGTH, &maxLength );

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog( maxLength );
        glGetProgramInfoLog( programHandle, maxLength, &maxLength, &infoLog[0] );

        FLAN_CERR << "Shader compilation failed!" << std::endl;
        FLAN_CERR << infoLog.data() << std::endl;

        return false;
    }

    return true;
}

NativePipelineStateObject* flan::rendering::CreatePipelineStateImpl( NativeRenderContext* nativeRenderContext, const PipelineStateDesc& description )
{
#define ATTACH_SHADER_IF_AVAILABLE(shader )\
    if ( description.shader != nullptr ) {\
        auto shaderObject = description.shader->getNativeShaderObject();\
        glAttachShader( program, shaderObject->shaderHandle );\
    }

#define DETACH_SHADER_IF_AVAILABLE(shader )\
    if ( description.shader != nullptr ) {\
        auto shaderObject = description.shader->getNativeShaderObject();\
        glDetachShader( program, shaderObject->shaderHandle );\
    }

    GLuint program = glCreateProgram();

    ATTACH_SHADER_IF_AVAILABLE( vertexStage );
    ATTACH_SHADER_IF_AVAILABLE( pixelStage );
    ATTACH_SHADER_IF_AVAILABLE( tesselationControlStage );
    ATTACH_SHADER_IF_AVAILABLE( tesselationEvalStage );
    ATTACH_SHADER_IF_AVAILABLE( computeStage );

    glLinkProgram( program );
    if ( !IsProgramCompiled( program ) ) {
        glDeleteProgram( program );
        return nullptr;
    }

    DETACH_SHADER_IF_AVAILABLE( vertexStage );
    DETACH_SHADER_IF_AVAILABLE( pixelStage );
    DETACH_SHADER_IF_AVAILABLE( tesselationControlStage );
    DETACH_SHADER_IF_AVAILABLE( tesselationEvalStage );
    DETACH_SHADER_IF_AVAILABLE( computeStage );

#undef ATTACH_SHADER_IF_AVAILABLE
#undef DETACH_SHADER_IF_AVAILABLE

    NativePipelineStateObject* nativePipelineStateObject = new NativePipelineStateObject();
    nativePipelineStateObject->renderProgram = program;

    return nativePipelineStateObject;
}

void flan::rendering::DestroyPipelineStateImpl( NativeRenderContext* nativeRenderContext, NativePipelineStateObject* PipelineStateObject )
{
    if ( PipelineStateObject->renderProgram != 0 ) {
        glDeleteProgram( PipelineStateObject->renderProgram );
        PipelineStateObject->renderProgram = 0;
    }
}
#endif
