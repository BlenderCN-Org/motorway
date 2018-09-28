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
#include "RenderPipelineBuilder.h"

#include "RenderPipelineResources.h"

RenderPipelineBuilder::RenderPipelineBuilder( RenderPipeline* owner )
    : builderOwner( owner )
{

}

RenderPipelineBuilder::~RenderPipelineBuilder()
{

}

void RenderPipelineBuilder::allocateResourcesAndBuild( RenderDevice* renderDevice, CommandList* cmdList, ShaderStageManager* shaderStageManager, RenderPipelineResources* pipelineResources )
{
    // Clear wellknown resources hashmap
    wellknownResources.clear();

    // Clear every resources used at previous frame (but don't release stuff, since most of the resources will be re-used next frame)
    pipelineResources->reset();

    const auto& activeViewport = getActiveViewport();
    pipelineResources->submitViewport( activeViewport );
    pipelineResources->submitCamera( viewport.worldViewport );
    pipelineResources->submitRenderPassesArgs( viewport.renderPassesArgs );

    // Allocate Pipeline State
    fnPipelineResHandle_t resourceHandle = 0;
    while ( !pipelineStatesToAllocate.empty() ) {
        ++resourceHandle;

        auto& pipelineDesc = pipelineStatesToAllocate.front();

        // Rebuild state key for each element
        pipelineDesc.rasterizerState.rebuildStateKey();
        pipelineDesc.blendState.rebuildStateKey();

        pipelineResources->allocateOrReusePipelineState( renderDevice, shaderStageManager, resourceHandle, pipelineDesc );

        pipelineStatesToAllocate.pop();
    }

    // Allocate Render Targets
    resourceHandle = 0;
    while ( !texturesToAllocate.empty() ) {
        ++resourceHandle;

        auto& textureDescription = texturesToAllocate.front();

        if ( textureDescription.useGlobalDimensions ) {
            textureDescription.description.width = activeViewport.Width;
            textureDescription.description.height = activeViewport.Height;

            FLAN_IMPORT_VAR_PTR( SSAAMultiplicator, float )
            textureDescription.description.width *= *SSAAMultiplicator;
            textureDescription.description.height *= *SSAAMultiplicator;
        }

        if ( textureDescription.useGlobalMultisamplingState ) {
            // Use predefined multisample patterns
            // See https://msdn.microsoft.com/en-us/library/windows/desktop/ff476218(v=vs.85).aspx
            textureDescription.description.flags.useMultisamplePattern = 1;

            FLAN_IMPORT_VAR_PTR( MSAASamplerCount, int32_t );         
            textureDescription.description.samplerCount = *MSAASamplerCount;
        }

        if ( textureDescription.copyResource ) {
            // Retrieve input texture
            auto& textureDesc = pipelineResources->getRenderTarget( textureDescription.resourceToCopy )->getDescription();

            // Copy Description
            textureDescription.description.width = textureDesc.width;
            textureDescription.description.height = textureDesc.height;
            textureDescription.description.depth = textureDesc.depth;
            textureDescription.description.dimension = textureDesc.dimension;
            textureDescription.description.format = textureDesc.format;
        }

        if ( textureDescription.lamdaComputeDimension != nullptr ) {
            auto renderTargetWidth = activeViewport.Width, renderTargetHeight = activeViewport.Height;

            if ( textureDescription.resourceToCopy != -1 ) {
                auto& textureDesc = pipelineResources->getRenderTarget( textureDescription.resourceToCopy )->getDescription();

                renderTargetWidth = textureDesc.width;
                renderTargetHeight = textureDesc.height;
            }

            auto computedSize = textureDescription.lamdaComputeDimension( renderTargetWidth, renderTargetHeight );

            textureDescription.description.width = glm::max( computedSize.x, 1u );
            textureDescription.description.height = glm::max( computedSize.y, 1u );
        }

        pipelineResources->allocateOrReuseRenderTarget( renderDevice, cmdList, resourceHandle, textureDescription );

        texturesToAllocate.pop();
    }

    // Allocate Buffers
    resourceHandle = 0;
    while ( !buffersToAllocate.empty() ) {
        ++resourceHandle;

        auto& bufferDesc = buffersToAllocate.front();

        pipelineResources->allocateOrReuseBuffer( renderDevice, resourceHandle, bufferDesc );

        buffersToAllocate.pop();
    }


    resourceHandle = 0; 
    while ( !samplersToAllocate.empty() ) {
        ++resourceHandle;

        auto& samplerDesc = samplersToAllocate.front();

        pipelineResources->allocateOrReuseSampler( renderDevice, resourceHandle, samplerDesc );

        samplersToAllocate.pop();
    }
}

fnPipelineResHandle_t RenderPipelineBuilder::allocateTexture( const RenderPassTextureDesc& textureDescription )
{
    texturesToAllocate.push( textureDescription );

    // NOTE Internal resource indexing starts at 1
    return static_cast<fnPipelineResHandle_t>( texturesToAllocate.size() );
}

fnPipelineResHandle_t RenderPipelineBuilder::allocatePipelineState( const RenderPassPipelineStateDesc& pipelineStateDescription )
{
    pipelineStatesToAllocate.push( pipelineStateDescription );

    // NOTE Internal resource indexing starts at 1
    return static_cast<fnPipelineResHandle_t>( pipelineStatesToAllocate.size() );
}

fnPipelineResHandle_t RenderPipelineBuilder::allocateBuffer( const BufferDesc& bufferDescription )
{
    buffersToAllocate.push( bufferDescription );

    // NOTE Internal resource indexing starts at 1
    return static_cast<fnPipelineResHandle_t>( buffersToAllocate.size() );
}

fnPipelineResHandle_t RenderPipelineBuilder::allocateSampler( const SamplerDesc& samplerDesc )
{
    samplersToAllocate.push( samplerDesc );

    // NOTE Internal resource indexing starts at 1
    return static_cast<fnPipelineResHandle_t>( samplersToAllocate.size() );
}

// TODO CLever resource management (dont allocate resource view if not needed, allocate RTV when asked to, ...)
fnPipelineResHandle_t RenderPipelineBuilder::useRenderTarget( const fnPipelineResHandle_t resource )
{
    return resource;
}

fnPipelineResHandle_t RenderPipelineBuilder::readRenderTarget( const fnPipelineResHandle_t resource )
{
    return resource;
}

fnPipelineResHandle_t RenderPipelineBuilder::readBuffer( const fnPipelineResHandle_t resource )
{
    return resource;
}

void RenderPipelineBuilder::pushViewportOverride( const Viewport& viewportOverride )
{
    viewportOverrideStack.push( viewportOverride );
}

void RenderPipelineBuilder::popViewportOverride()
{
    viewportOverrideStack.pop();
}

void RenderPipelineBuilder::setViewport( const RenderPipelineViewport& viewport )
{
    this->viewport = viewport;
}

const Viewport& RenderPipelineBuilder::getActiveViewport() const
{
    return viewportOverrideStack.empty() ? viewport.rendererViewport : viewportOverrideStack.front();
}

const void* RenderPipelineBuilder::getRenderPassArgs( const fnStringHash_t renderPassHashcode ) const
{
    auto it = viewport.renderPassesArgs.find( renderPassHashcode );
    return it != viewport.renderPassesArgs.end() ? it->second : nullptr;
}

void RenderPipelineBuilder::registerWellKnownResource( const fnStringHash_t hashcode, const fnPipelineResHandle_t handle )
{
    wellknownResources[hashcode] = handle;
}

fnPipelineResHandle_t RenderPipelineBuilder::getWellKnownResource( const fnStringHash_t hashcode )
{
    auto it = wellknownResources.find( hashcode );

    return it != wellknownResources.end() ? it->second : -1;
}

RenderPipeline* RenderPipelineBuilder::getOwner() const
{
    // TODO This sucks; but is required for embeeded render pass registration (e.g. register each down/upsample pass for bloom)
    return builderOwner;
}
