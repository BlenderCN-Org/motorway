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
#include "VertexArrayObject.h"

#include "Buffer.h"

GLenum ToOpenGLFormat( const decltype( VertexLayoutEntry::format ) format )
{
    switch ( format ) {
    case VertexLayoutEntry::FORMAT_BYTE:
        return GL_BYTE;

    case VertexLayoutEntry::FORMAT_DOUBLE:
        return GL_DOUBLE;

    case VertexLayoutEntry::FORMAT_FLOAT:
        return GL_FLOAT;

    case VertexLayoutEntry::FORMAT_HALF_FLOAT:
        return GL_HALF_FLOAT;

    case VertexLayoutEntry::FORMAT_INT:
        return GL_INT;

    case VertexLayoutEntry::FORMAT_SHORT:
        return GL_SHORT;

    default:
        return 0;
    }
}

NativeVertexArrayObject* flan::rendering::CreateVertexArrayImpl( NativeRenderContext* nativeRenderContext, Buffer* vbo, Buffer* ibo )
{
    NativeVertexArrayObject* vertexArrayObject = new NativeVertexArrayObject();
    glCreateVertexArrays( 1, &vertexArrayObject->bufferHandle );

    auto vboHandle = vbo->getNativeBufferObject();
    glVertexArrayVertexBuffer( vertexArrayObject->bufferHandle, 0, vboHandle->bufferHandle, 0, 0 );

    vertexArrayObject->vboHandle = vboHandle->bufferHandle;

    if ( ibo != nullptr ) {
        auto iboHandle = ibo->getNativeBufferObject();
        glVertexArrayElementBuffer( vertexArrayObject->bufferHandle, iboHandle->bufferHandle );

        vertexArrayObject->iboHandle = iboHandle->bufferHandle;
    }

    return vertexArrayObject;
}

void flan::rendering::DestroyVertexArrayImpl( NativeRenderContext* nativeRenderContext, NativeVertexArrayObject* vertexArrayObject )
{
    glDeleteVertexArrays( 1, &vertexArrayObject->bufferHandle );
}

void flan::rendering::SetVertexArrayVertexLayoutImpl( NativeRenderContext* nativeRenderContext, NativeVertexArrayObject* vertexArrayObject, const VertexLayout_t& vertexLayout )
{
    for ( auto& entry : vertexLayout ) {
        glEnableVertexArrayAttrib( vertexArrayObject->bufferHandle, entry.index );
        glVertexArrayAttribBinding( vertexArrayObject->bufferHandle, entry.index, 0 );
        glVertexArrayAttribFormat( vertexArrayObject->bufferHandle, entry.index, ( GLint )entry.dimension, ToOpenGLFormat( entry.format ), GL_FALSE, entry.offset );
    }
}

void flan::rendering::BindVertexArrayCmdImpl( NativeCommandList* nativeCmdList, NativeVertexArrayObject* vertexArrayObject )
{
    glBindVertexArray( vertexArrayObject->bufferHandle );
}
#endif
