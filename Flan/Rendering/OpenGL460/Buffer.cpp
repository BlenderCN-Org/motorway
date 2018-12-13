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
#include "Buffer.h"

#include "ImageFormat.h"

NativeBufferObject* flan::rendering::CreateBufferImpl( NativeRenderContext* nativeRenderContext, const BufferDesc& description, void* initialData )
{
    GLenum target = GL_UNIFORM_BUFFER;
    GLenum usage = GL_STATIC_DRAW;

    // Retrieve proper target id and usage
    if ( description.Type == BufferDesc::CONSTANT_BUFFER ) {
        target = GL_UNIFORM_BUFFER;
        usage = GL_DYNAMIC_DRAW;
    } else if ( description.Type == BufferDesc::DYNAMIC_VERTEX_BUFFER ) {
        target = GL_ARRAY_BUFFER;
        usage = GL_DYNAMIC_DRAW;
    } else if ( description.Type == BufferDesc::VERTEX_BUFFER ) {
        target = GL_ARRAY_BUFFER;
        usage = GL_STATIC_DRAW;
    } else if ( description.Type == BufferDesc::DYNAMIC_INDICE_BUFFER ) {
        target = GL_ELEMENT_ARRAY_BUFFER;
        usage = GL_DYNAMIC_DRAW;
    } else if ( description.Type == BufferDesc::INDICE_BUFFER ) {
        target = GL_ELEMENT_ARRAY_BUFFER;
        usage = GL_STATIC_DRAW;
    } else if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER ) {
        target = GL_TEXTURE_BUFFER;
        usage = GL_STATIC_DRAW;
    } else if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_1D ) {
        target = GL_TEXTURE_1D;
        usage = GL_STATIC_DRAW;
    } else if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D ) {
        target = GL_TEXTURE_2D;
        usage = GL_STATIC_DRAW;
    } else if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_3D ) {
        target = GL_TEXTURE_3D;
        usage = GL_STATIC_DRAW;
    } else {
        FLAN_CERR << "Unsupported/unimplemented buffer type!" << std::endl;
        return nullptr;
    }

    NativeBufferObject* nativeBufferObject = new NativeBufferObject();
    nativeBufferObject->target = target;
    nativeBufferObject->usage = usage;
    nativeBufferObject->stride = description.Stride;

    glCreateBuffers( 1, &nativeBufferObject->bufferHandle );
    glNamedBufferData( nativeBufferObject->bufferHandle, static_cast<GLsizeiptr>( description.Size ), initialData, nativeBufferObject->usage );
    glNamedBufferStorage( nativeBufferObject->bufferHandle, static_cast<GLsizeiptr>( description.Size ), initialData, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT );

    if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER ) {
        nativeBufferObject->viewFormat = GL_IMAGE_FORMAT[description.ViewFormat];

        glCreateTextures( nativeBufferObject->target, 1, &nativeBufferObject->textureHandle );
        glTextureBuffer( nativeBufferObject->textureHandle, nativeBufferObject->viewFormat, nativeBufferObject->bufferHandle );
    }

    return nativeBufferObject;
}

void flan::rendering::DestroyBufferImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject )
{
    if ( bufferObject->target != 0 ) {
        glDeleteBuffers( 1, &bufferObject->bufferHandle );
        bufferObject->bufferHandle = 0;
    }

    bufferObject->target = 0;
    bufferObject->usage = 0;
    bufferObject->stride = 0;
}

void flan::rendering::UpdateBufferImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize )
{
    glBindBuffer( bufferObject->target, bufferObject->bufferHandle );

    GLvoid* bufferPointer = glMapBuffer( bufferObject->target, GL_WRITE_ONLY );

    if ( bufferPointer == nullptr ) {
        FLAN_CERR << "Failed to map buffer! (error code: " << glGetError() << ")" << std::endl;
        return;
    }

    memcpy( bufferPointer, dataToUpload, dataToUploadSize );

    glUnmapBuffer( bufferObject->target );
    glBindBuffer( bufferObject->target, 0 );
}

void flan::rendering::UpdateBufferAsynchronousImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize )
{
    glBindBuffer( bufferObject->target, bufferObject->bufferHandle );

    GLvoid* bufferPointer = glMapBuffer( bufferObject->target, GL_WRITE_ONLY );

    if ( bufferPointer == nullptr ) {
        FLAN_CERR << "Failed to map buffer! (error code: " << glGetError() << ")" << std::endl;
        return;
    }

    memcpy( bufferPointer, dataToUpload, dataToUploadSize );

    glUnmapBuffer( bufferObject->target );
    glBindBuffer( bufferObject->target, 0 );
}

void flan::rendering::UpdateBufferRangeImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize, const int32_t bufferOffset )
{
    glBindBuffer( bufferObject->target, bufferObject->bufferHandle );

    GLvoid* bufferPointer = glMapBufferRange( bufferObject->target, bufferOffset, dataToUploadSize, GL_WRITE_ONLY );

    if ( bufferPointer == nullptr ) {
        FLAN_CERR << "Failed to map buffer! (error code: " << glGetError() << ")" << std::endl;
        return;
    }

    memcpy( bufferPointer, dataToUpload, dataToUploadSize );

    glUnmapBuffer( bufferObject->target );
    glBindBuffer( bufferObject->target, 0 );
}

void flan::rendering::FlushAndUpdateBufferRangeImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize, const int32_t bufferOffset )
{
    glBindBuffer( bufferObject->target, bufferObject->bufferHandle );

    GLvoid* bufferPointer = glMapBufferRange( bufferObject->target, bufferOffset, dataToUploadSize, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT );

    if ( bufferPointer == nullptr ) {
        FLAN_CERR << "Failed to map buffer! (error code: " << glGetError() << ")" << std::endl;
        return;
    }

    memcpy( bufferPointer, dataToUpload, dataToUploadSize );

    glFlushMappedBufferRange( bufferObject->target, bufferOffset, dataToUploadSize );

    glUnmapBuffer( bufferObject->target );
    glBindBuffer( bufferObject->target, 0 );
}

void flan::rendering::BindBufferCmdImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const uint32_t shaderStagesToBindTo, const uint32_t bindingIndex )
{
    switch ( bufferObject->target ) {
    case GL_UNIFORM_BUFFER:
        glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, bufferObject->bufferHandle );
        break;

    case GL_ARRAY_BUFFER:
        glBindVertexBuffer( bindingIndex, bufferObject->bufferHandle, 0, static_cast<GLsizei>( bufferObject->stride ) );
        break;

    case GL_ELEMENT_ARRAY_BUFFER:
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, bufferObject->bufferHandle );
        break;

    case GL_TEXTURE_BUFFER:
        glBindImageTexture( bindingIndex, bufferObject->textureHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, bufferObject->viewFormat );
        break;

    default:
        break;
    }
}

void flan::rendering::BindBufferReadOnlyCmdImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const uint32_t shaderStagesToBindTo, const uint32_t bindingIndex )
{
    switch ( bufferObject->target ) {
    case GL_UNIFORM_BUFFER:
    case GL_ARRAY_BUFFER:
    case GL_ELEMENT_ARRAY_BUFFER:
        // No need to distinguish read/write for those kind of buffer
        BindBufferCmdImpl( nativeCmdList, bufferObject, shaderStagesToBindTo, bindingIndex );
        break;

    case GL_TEXTURE_BUFFER:
        glActiveTexture( GL_TEXTURE0 + bindingIndex );
        glBindTexture( bufferObject->target, bufferObject->textureHandle );
        break;

    default:
        break;
    }
}

void flan::rendering::UnbindBufferCmdImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const uint32_t shaderStagesToBindTo, const uint32_t bindingIndex, const Buffer::BindMode bindMode )
{
    switch ( bufferObject->target ) {
    case GL_UNIFORM_BUFFER:
        glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, 0 );
        break;

    case GL_ARRAY_BUFFER:
        glBindVertexBuffer( bindingIndex, 0, 0, 0 );
        break;

    case GL_ELEMENT_ARRAY_BUFFER:
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        break;

    case GL_TEXTURE_BUFFER:
        if ( bindMode == Buffer::BindMode::READ_ONLY ) {
            glActiveTexture( GL_TEXTURE0 + bindingIndex );
            glBindTexture( bufferObject->target, 0 );
        }
        break;

    default:
        break;
    }
}

void flan::rendering::CopyStructureCountImpl( NativeCommandList* nativeCmdList, NativeBufferObject* sourceBufferObject, NativeBufferObject* destinationBufferObject, const uint32_t byteOffset )
{

}
#endif
