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

#include "Viewport.h"
#include "RenderTarget.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "RasterizerState.h"
#include "PipelineState.h"

class RenderDevice;
class PipelineState;
struct NativeCommandList;

class CommandList
{
public:
                            CommandList();
                            CommandList( CommandList& ) = delete;
                            CommandList& operator = ( CommandList& ) = delete;
                            ~CommandList();

    void                    setNativeCommandList( NativeCommandList* nativeCmdList );
    NativeCommandList*      getNativeCommandList() const;

    void                    beginCommandList( RenderDevice* renderDevice );
    void                    endCommandList( RenderDevice* renderDevice );
    void                    playbackCommandList( RenderDevice* renderDevice );

    void                    drawCmd( const uint32_t vertexCount, const uint32_t vertexOffset = 0 );
    void                    drawIndexedCmd( const uint32_t indiceCount, const uint32_t indiceOffset = 0, const std::size_t indiceType = sizeof( uint32_t ), const uint32_t vertexOffset = 0 );
    void                    dispatchComputeCmd( const unsigned int threadCountX, const unsigned int threadCountY, const unsigned int threadCountZ );

    Viewport                getViewportCmd();
    void                    setViewportCmd( const int32_t width, const int32_t height, const int32_t x = 0, const int32_t y = 0, const float minDepth = 0.0f, const float maxDepth = 1.0f );
    void                    setViewportCmd( const Viewport& viewport );

    void                    bindBackbufferCmd();
    void                    bindRenderTargetsCmd( RenderTarget** renderTarget, RenderTarget* depthRenderTarget = nullptr, const uint32_t renderTargetCount = 1, const uint32_t mipLevel = 0 );
    void                    bindRenderTargetsLayeredCmd( RenderTarget** renderTarget, RenderTarget* depthRenderTarget = nullptr, const uint32_t renderTargetCount = 1, const uint32_t mipLevel = 0, const uint32_t layerIndex = 0 );
    void                    bindPipelineStateCmd( PipelineState* pipelineState );

    void                    clearRenderTargetCmd();

    void                    clearColorRenderTargetCmd();
    void                    clearDepthRenderTargetCmd();

    void                    setClearDepthValueCmd( const float depthValue = 0.0f );

    void                    unbindVertexArrayCmd();

private:
    std::unique_ptr<NativeCommandList> nativeCommandList;
    RasterizerStateDesc::KeyValue   currentRasterizerState;
    BlendStateDesc::KeyValue        currentBlendState;
};
