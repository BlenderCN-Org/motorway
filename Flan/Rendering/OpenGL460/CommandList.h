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

#if FLAN_GL460
#include <Rendering/Viewport.h>

class DisplaySurface;
class RenderTarget;
class BlendState;
class PipelineState;
class Texture;

struct NativeRenderContext;
struct NativeTextureObject;

#include "Extensions.h"

struct NativeCommandList
{
    static constexpr int32_t FBO_INTERNAL_COUNT = 24;

    int32_t     FrameBufferAvailableIndex;
    GLuint      FrameBuffers[NativeCommandList::FBO_INTERNAL_COUNT];
    GLuint      FrameBuffersAttachementsUsed[NativeCommandList::FBO_INTERNAL_COUNT][9];
    GLenum      PrimitiveTopology;

    // Used by any draw call not relying on vertex buffers (fullscreen triangle, etc.)
    GLuint      DummyVAO;
    GLuint      DummyVBO;
};

namespace flan
{
    namespace rendering
    {
        NativeCommandList*      CreateCommandListImpl( NativeRenderContext* renderContext );
        void                    DestroyCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList );
        void                    BeginCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList );
        void                    EndCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList );
        void                    PlayBackCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList );

        void                    SetDepthClearValue( NativeCommandList* cmdList, const float depthClearValue = 0.0f );

        void                    ClearRenderTargetImpl( NativeCommandList* cmdList );
        void                    ClearColorRenderTargetImpl( NativeCommandList* cmdList );
        void                    ClearDepthRenderTargetImpl( NativeCommandList* cmdList );

        void                    DrawImpl( NativeCommandList* cmdList, const unsigned int vertexCount, const unsigned int vertexOffset = 0 );
        void                    DrawIndexedImpl( NativeCommandList* cmdList, const uint32_t indiceCount, const uint32_t indiceOffset = 0, const std::size_t indiceType = sizeof( uint32_t ), const uint32_t vertexOffset = 0 );
        void                    DrawInstancedIndexedImpl( NativeCommandList* cmdList, const unsigned int indiceCount, const unsigned int instanceCount, const unsigned int indexOffset, const unsigned int vertexOffset = 0, const unsigned int instanceOffset = 0 );

        void                    DispatchComputeImpl( NativeCommandList* cmdList, const unsigned int threadCountX, const unsigned int threadCountY, const unsigned int threadCountZ );

        void                    GetViewportImpl( NativeCommandList* cmdList, Viewport& viewport );
        void                    SetViewportImpl( NativeCommandList* cmdList, const Viewport& viewport );
        
        void                    BindBackbufferImpl( NativeCommandList* cmdList );
        void                    BindRenderTargetImpl( NativeCommandList* cmdList, RenderTarget** renderTarget, RenderTarget* depthRenderTarget = nullptr, const uint32_t renderTargetCount = 1, const uint32_t mipLevel = 0 );
        void                    BindRenderTargetLayeredImpl( NativeCommandList* cmdList, RenderTarget** renderTarget, RenderTarget* depthRenderTarget = nullptr, const uint32_t renderTargetCount = 1, const uint32_t mipLevel = 0, const uint32_t layerIndex = 0 );

        void                    UnbindVertexArrayImpl( NativeCommandList* cmdList );
        void                    BindPipelineStateImpl( NativeCommandList* cmdList, PipelineState* pipelineState );
        void                    ResolveSubresourceImpl( NativeCommandList* cmdList, NativeTextureObject* resourceToResolve, NativeTextureObject* resolvedResource );
    }
}
#endif
