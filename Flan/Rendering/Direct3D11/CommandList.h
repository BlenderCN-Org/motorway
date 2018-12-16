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

#if FLAN_D3D11
#include <d3d11.h>

#include <Rendering/Viewport.h>

class DisplaySurface;
class RenderTarget;
class BlendState;
class PipelineState;
class Texture;

struct NativeRenderContext;
struct NativeBufferObject;
struct NativeTextureObject;

struct NativeCommandList
{
    ID3D11DeviceContext*	deferredContext;
    ID3D11CommandList*      cmdList;
    ID3D11RenderTargetView* backbuffer;
    float                   depthClearValue;

    ID3D11DepthStencilView* currentDepthBuffer;
    ID3D11RenderTargetView* currentRenderTargets[8];
    int                     currentRenderTargetCount;


    NativeCommandList()
        : deferredContext( nullptr )
        , cmdList( nullptr )
        , backbuffer( nullptr )
        , depthClearValue( 0.0f )
        , currentDepthBuffer( nullptr )
        , currentRenderTargets{ nullptr }
        , currentRenderTargetCount( 0 )
    {

    }
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
        void                    DrawInstancedIndirectImpl( NativeCommandList* cmdList, const NativeBufferObject* drawArgsBuffer, const unsigned int bufferDataOffset );

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
