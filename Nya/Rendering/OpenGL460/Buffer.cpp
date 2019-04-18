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
#include <Rendering/CommandList.h>

#include "Extensions.h"
#include "ImageHelpers.h"

#include <string.h>

struct Buffer
{
    GLuint  bufferHandle;
    GLuint  textureHandle; // Image buffer ONLY
    GLuint  stride;
    GLenum  target;
    GLenum  usage;
    GLenum  viewFormat; // Image buffer ONLY
};

Buffer* RenderDevice::createBuffer( const BufferDesc& description, const void* initialData )
{
    GLenum target = GL_UNIFORM_BUFFER;
    GLenum usage = GL_STATIC_DRAW;

    Buffer* buffer = nya::core::allocate<Buffer>( memoryAllocator );

//    case BufferDesc::APPEND_STRUCTURED_BUFFER:
//    case BufferDesc::STRUCTURED_BUFFER:
//        return CreateStructuredBuffer( nativeDevice, buffer, description, initialData );

//    case BufferDesc::INDIRECT_DRAW_ARGUMENTS:
//        return CreateIndirectDrawArgsBuffer( nativeDevice, buffer, description, initialData );

//    case BufferDesc::GENERIC_BUFFER:
//        return CreateGenericBuffer( nativeDevice, buffer, description, initialData );

    // Retrieve proper target id and usage
    if ( description.type == BufferDesc::CONSTANT_BUFFER ) {
        target = GL_UNIFORM_BUFFER;
        usage = GL_DYNAMIC_DRAW;
    } else if ( description.type == BufferDesc::DYNAMIC_VERTEX_BUFFER ) {
        target = GL_ARRAY_BUFFER;
        usage = GL_DYNAMIC_DRAW;
    } else if ( description.type == BufferDesc::VERTEX_BUFFER ) {
        target = GL_ARRAY_BUFFER;
        usage = GL_STATIC_DRAW;
    } else if ( description.type == BufferDesc::DYNAMIC_INDICE_BUFFER ) {
        target = GL_ELEMENT_ARRAY_BUFFER;
        usage = GL_DYNAMIC_DRAW;
    } else if ( description.type == BufferDesc::INDICE_BUFFER ) {
        target = GL_ELEMENT_ARRAY_BUFFER;
        usage = GL_STATIC_DRAW;
    } else if ( description.type == BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER ) {
        target = GL_TEXTURE_BUFFER;
        usage = GL_STATIC_DRAW;
    } else if ( description.type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_1D ) {
        target = GL_TEXTURE_1D;
        usage = GL_STATIC_DRAW;
    } else if ( description.type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D ) {
        target = GL_TEXTURE_2D;
        usage = GL_STATIC_DRAW;
    } else if ( description.type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_3D ) {
        target = GL_TEXTURE_3D;
        usage = GL_STATIC_DRAW;
    } else {
        NYA_CERR << "Unsupported/unimplemented buffer type!" << std::endl;
        return nullptr;
    }

    buffer->target = target;
    buffer->usage = usage;
    buffer->stride = description.stride;

    glCreateBuffers( 1, &buffer->bufferHandle );
    glNamedBufferData( buffer->bufferHandle, static_cast<GLsizeiptr>( description.size ), initialData, buffer->usage );
    glNamedBufferStorage( buffer->bufferHandle, static_cast<GLsizeiptr>( description.size ), initialData, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT );

    if ( description.type == BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER ) {
        buffer->viewFormat = GL_IMAGE_FORMAT[description.viewFormat];

        glCreateTextures( buffer->target, 1, &buffer->textureHandle );
        glTextureBuffer( buffer->textureHandle, buffer->viewFormat, buffer->bufferHandle );
    }

    return buffer;
}

void RenderDevice::destroyBuffer( Buffer* buffer )
{
    if ( buffer->target != 0 ) {
        glDeleteBuffers( 1, &buffer->bufferHandle );
        buffer->bufferHandle = 0;
    }

    buffer->target = 0;
    buffer->usage = 0;
    buffer->stride = 0;

    nya::core::free( memoryAllocator, buffer );
}

void RenderDevice::setDebugMarker( Buffer* buffer, const char* objectName )
{
    glObjectLabel( GL_BUFFER, buffer->bufferHandle, strlen( objectName ), objectName );
}

void CommandList::bindVertexBuffer( const Buffer* buffer, const unsigned int bindIndex )
{

}

void CommandList::bindIndiceBuffer( const Buffer* buffer )
{

}

void CommandList::updateBuffer( Buffer* buffer, const void* data, const size_t dataSize )
{
    glBindBuffer( buffer->target, buffer->bufferHandle );

    GLvoid* bufferPointer = glMapBuffer( buffer->target, GL_WRITE_ONLY );

    if ( bufferPointer == nullptr ) {
       NYA_CERR << "Failed to map buffer! (error code: " << glGetError() << ")" << std::endl;
       return;
    }

    memcpy( bufferPointer, data, dataSize );

    glUnmapBuffer( buffer->target );
    glBindBuffer( buffer->target, 0 );
}

void CommandList::copyStructureCount( Buffer* srcBuffer, Buffer* dstBuffer, const unsigned int offset )
{

}

void CommandList::drawInstancedIndirect( const Buffer* drawArgsBuffer, const unsigned int bufferDataOffset )
{

}
#endif
