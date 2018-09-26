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
#include "CommandList.h"

#include "RenderDevice.h"

#if FLAN_D3D11
#include "Direct3D11/CommandList.h"
#elif FLAN_GL460
#include "OpenGL460/CommandList.h"
#elif FLAN_VULKAN
#include "Vulkan/CommandList.h"
#endif

CommandList::CommandList()
    : nativeCommandList( nullptr )
    , currentRasterizerState( 0 )
    , currentBlendState( 0 )
{

}

CommandList::~CommandList()
{

}

void CommandList::setNativeCommandList( NativeCommandList* nativeCmdList )
{
    nativeCommandList.reset( nativeCmdList );
}

NativeCommandList* CommandList::getNativeCommandList() const
{
    return nativeCommandList.get();
}

void CommandList::beginCommandList( RenderDevice* renderDevice )
{
    flan::rendering::BeginCommandListImpl( renderDevice->getNativeRenderContext(), nativeCommandList.get() );
}

void CommandList::endCommandList( RenderDevice* renderDevice )
{
    flan::rendering::EndCommandListImpl( renderDevice->getNativeRenderContext(), nativeCommandList.get() );
}

void CommandList::playbackCommandList( RenderDevice* renderDevice )
{
    flan::rendering::PlayBackCommandListImpl( renderDevice->getNativeRenderContext(), nativeCommandList.get() );
}

Viewport CommandList::getViewportCmd()
{
    Viewport activeViewport;
    flan::rendering::GetViewportImpl( nativeCommandList.get(), activeViewport );

    return activeViewport;
}

void CommandList::setViewportCmd( const int32_t width, const int32_t height, const int32_t x, const int32_t y, const float minDepth, const float maxDepth )
{
    Viewport viewport;
    viewport.X = x;
    viewport.Y = y;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = minDepth;
    viewport.MaxDepth = maxDepth;

    setViewportCmd( viewport );
}

void CommandList::setViewportCmd( const Viewport& viewport )
{
    flan::rendering::SetViewportImpl( nativeCommandList.get(), viewport );
}

void CommandList::bindBackbufferCmd()
{
    flan::rendering::BindBackbufferImpl( nativeCommandList.get() );
}

void CommandList::bindRenderTargetsCmd( RenderTarget** renderTarget, RenderTarget* depthRenderTarget, const uint32_t renderTargetCount, const uint32_t mipLevel )
{
    flan::rendering::BindRenderTargetImpl( nativeCommandList.get(), renderTarget, depthRenderTarget, renderTargetCount, mipLevel );
}

void CommandList::bindRenderTargetsLayeredCmd( RenderTarget** renderTarget, RenderTarget* depthRenderTarget, const uint32_t renderTargetCount, const uint32_t mipLevel, const uint32_t layerIndex )
{
    flan::rendering::BindRenderTargetLayeredImpl( nativeCommandList.get(), renderTarget, depthRenderTarget, renderTargetCount, mipLevel, layerIndex );
}

void CommandList::bindPipelineStateCmd( PipelineState* pipelineState )
{
    flan::rendering::BindPipelineStateImpl( nativeCommandList.get(), pipelineState );

    const auto psoDesc = pipelineState->getDescription();

    if ( psoDesc->depthStencilState != nullptr ) {
        psoDesc->depthStencilState->bind( this );
    }

    // For each element of the pipeline state, retrieve its key (as integer) and check whether or not 
    // a rebind (total or partial) is needed
    if ( psoDesc->rasterizerState != nullptr ) {
        auto rasterizerKey = psoDesc->rasterizerState->getKeyValue();
        if ( rasterizerKey != currentRasterizerState ) {
            psoDesc->rasterizerState->bind( this );
            currentRasterizerState = rasterizerKey;
        }
    }

    if ( psoDesc->blendState != nullptr ) {
        auto blendStateKey = psoDesc->blendState->getKeyValue();
        //if ( blendStateKey != currentBlendState ) {
            psoDesc->blendState->bind( this );
            currentBlendState = blendStateKey;
        //}
    }
}

void CommandList::clearRenderTargetCmd()
{
    flan::rendering::ClearRenderTargetImpl( nativeCommandList.get() );
}

void CommandList::clearColorRenderTargetCmd()
{
    flan::rendering::ClearColorRenderTargetImpl( nativeCommandList.get() );
}

void CommandList::clearDepthRenderTargetCmd()
{
    flan::rendering::ClearDepthRenderTargetImpl( nativeCommandList.get() );
}

void CommandList::setClearDepthValueCmd( const float depthValue )
{
    flan::rendering::SetDepthClearValue( nativeCommandList.get(), depthValue );
}

void CommandList::unbindVertexArrayCmd()
{
    flan::rendering::UnbindVertexArrayImpl( nativeCommandList.get() );
}

void CommandList::drawCmd( const uint32_t vertexCount, const uint32_t vertexOffset )
{
    flan::rendering::DrawImpl( nativeCommandList.get(), vertexCount, vertexOffset );
}

void CommandList::drawIndexedCmd( const uint32_t indiceCount, const uint32_t indiceOffset, const std::size_t indiceType, const uint32_t vertexOffset )
{
    flan::rendering::DrawIndexedImpl( nativeCommandList.get(), indiceCount, indiceOffset, indiceType, vertexOffset );
}

void CommandList::dispatchComputeCmd( const unsigned int threadCountX, const unsigned int threadCountY, const unsigned int threadCountZ )
{
    flan::rendering::DispatchComputeImpl( nativeCommandList.get(), threadCountX, threadCountY, threadCountZ );
}
