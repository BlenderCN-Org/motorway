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
#pragma once

struct NativeCommandList;
struct PipelineState;
struct RenderTarget;
struct RenderPass;
struct Buffer;
struct ResourceList;
struct QueryPool;
struct Texture;

struct Viewport
{
    int     X;
    int     Y;
    int     Width;
    int     Height;

    float   MinDepth;
    float   MaxDepth;
};

class CommandList
{
public:
    NativeCommandList*  NativeCommandList;

public:
                        CommandList( BaseAllocator* allocator );
                        CommandList( CommandList& ) = delete;
                        CommandList& operator = ( CommandList& ) = delete;
                        ~CommandList();

    void                begin();
    void                end();

    void                bindVertexBuffer( const Buffer* buffer, const unsigned int bindIndex = 0 );
    void                bindIndiceBuffer( const Buffer* buffer );

    void                clearRenderTarget( RenderTarget* renderTarget, const float clearColor[4] );
    void                bindPipelineState( PipelineState* pipelineState );
    void                bindResourceList( ResourceList* resourceList );

    void                setViewport( const Viewport& viewport );
    void                getViewport( Viewport& viewport );

    void                useRenderPass( RenderPass* renderPass );

    void                draw( const unsigned int vertexCount, const unsigned int vertexOffset = 0u );
    void                drawIndexed( const unsigned int indiceCount, const unsigned int indiceOffset = 0, const size_t indiceType = sizeof( unsigned int ), const unsigned int vertexOffset = 0 );
    void                drawInstancedIndexed( const unsigned int indiceCount, const unsigned int instanceCount, const unsigned int indiceOffset, const unsigned int vertexOffset = 0, const unsigned int instanceOffset = 0 );
    void                drawInstancedIndirect( const Buffer* drawArgsBuffer, const unsigned int bufferDataOffset );
    void                dispatchCompute( const unsigned int threadCountX, const unsigned int threadCountY, const unsigned int threadCountZ );

    void                updateBuffer( Buffer* buffer, const void* data, const size_t dataSize );
    void                copyStructureCount( Buffer* srcBuffer, Buffer* dstBuffer, const unsigned int offset = 0 );

    unsigned int        allocateQuery( QueryPool* queryPool );
    void                beginQuery( QueryPool* queryPool, const unsigned int queryIndex );
    void                endQuery( QueryPool* queryPool, const unsigned int queryIndex );
    void                writeTimestamp( QueryPool* queryPool, const unsigned int queryIndex );

    // NOTE Texture must have been created with the flag 'allowCPUWrite' set
    // NOTE widthSize = texture width * texture format size (in bytes)
    void                updateTexture3D( Texture* texture, const void* data, const size_t texelSize, const size_t width, const size_t height, const size_t depth );

private:
    BaseAllocator*      memoryAllocator;
};
