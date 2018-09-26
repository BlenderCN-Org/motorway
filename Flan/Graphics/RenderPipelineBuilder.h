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

#include <queue>
#include <unordered_map>

#include <Rendering/RenderDevice.h>
#include "RenderPass.h"

#include <Rendering/Texture.h>
#include <Rendering/PipelineState.h>
#include <Rendering/Buffer.h>
#include <Rendering/DepthStencilState.h>
#include <Rendering/BlendState.h>
#include <Rendering/RasterizerState.h>
#include <Rendering/Sampler.h>

class ShaderStageManager;
class RenderPipeline;

class RenderPipelineBuilder
{
public:
                                        RenderPipelineBuilder( RenderPipeline* owner );
                                        RenderPipelineBuilder( RenderPipelineBuilder& ) = delete;
                                        ~RenderPipelineBuilder();

    void                                allocateResourcesAndBuild( RenderDevice* renderDevice, CommandList* cmdList, ShaderStageManager* shaderStageManager, RenderPipelineResources* pipelineResources );

    fnPipelineResHandle_t               allocateTexture( const RenderPassTextureDesc& textureDescription );
    fnPipelineResHandle_t               allocatePipelineState( const RenderPassPipelineStateDesc& pipelineStateDescription );
    fnPipelineResHandle_t               allocateBuffer( const BufferDesc& bufferDescription );
    fnPipelineResHandle_t               allocateSampler( const SamplerDesc& samplerDesc );

    fnPipelineMutableResHandle_t        useRenderTarget( const fnPipelineMutableResHandle_t resource );
    fnPipelineMutableResHandle_t        readRenderTarget( const fnPipelineMutableResHandle_t resource );
    fnPipelineMutableResHandle_t        readBuffer( const fnPipelineMutableResHandle_t resource );

    void                                pushViewportOverride( const Viewport& viewportOverride );
    void                                popViewportOverride();

    void                                setViewport( const RenderPipelineViewport& viewport );
    const Viewport&                     getActiveViewport() const;

    const void*                         getRenderPassArgs( const fnStringHash_t renderPassHashcode ) const;

    void                                registerWellKnownResource( const fnStringHash_t hashcode, const fnPipelineResHandle_t handle );
    fnPipelineResHandle_t               getWellKnownResource( const fnStringHash_t hashcode );

    RenderPipeline*                     getOwner() const;

private:
    std::queue<Viewport>                    viewportOverrideStack;
    RenderPipelineViewport                  viewport;
    RenderPipeline*                         builderOwner;

    std::queue<RenderPassTextureDesc>       texturesToAllocate;
    std::queue<RenderPassPipelineStateDesc> pipelineStatesToAllocate;
    std::queue<BufferDesc>                  buffersToAllocate;
    std::queue<SamplerDesc>                 samplersToAllocate;

    std::unordered_map<fnStringHash_t, fnPipelineResHandle_t> wellknownResources;
};
