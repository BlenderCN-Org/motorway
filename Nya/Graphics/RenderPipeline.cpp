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
#include "RenderPipeline.h"

RenderPipelineBuilder::RenderPipelineBuilder()
    : passRenderTargetRefs{ {0} }
    , renderPassCount( -1 )
    , renderTargetCount( 0 )
    , bufferCount( 0 )
    , samplerCount( 0 )
{
    
}

RenderPipelineBuilder::~RenderPipelineBuilder()
{

}

void RenderPipelineBuilder::cullRenderPasses( RenderPipelineRenderPass* renderPassList, int& renderPassListLength )
{
    int tmpRenderPassCount = 0;

    for ( int32_t i = 0; i < renderPassListLength; i++ ) {
        auto& passInfos = passRenderTargetRefs[i];

        bool cullPass = true;

        if ( passInfos.renderTargetCount == static_cast< uint32_t >( -1 ) ) {
            cullPass = false;
        } else {
            for ( uint32_t j = 0; j < passInfos.renderTargetCount; j++ ) {
                if ( renderTarget[passInfos.renderTarget[j]].referenceCount > 0 ) {
                    cullPass = false;
                    break;
                }
            }
        }

        if ( !cullPass ) {
            renderPassList[tmpRenderPassCount++] = renderPassList[i];
        }
    }

    renderPassListLength = tmpRenderPassCount;
}

void RenderPipelineBuilder::useAsyncCompute( const bool state )
{

}

void RenderPipelineBuilder::setUncullablePass()
{
    passRenderTargetRefs[renderPassCount].renderTargetCount = static_cast< uint32_t >( -1 );
}

void RenderPipelineBuilder::compile( RenderDevice* renderDevice, RenderPipelineResources& resources )
{
    resources.unacquireResources();

    for ( uint32_t i = 0; i < renderTargetCount; i++ ) {
        auto& resToAlloc = renderTarget[i];

        if ( resToAlloc.referenceCount == 0 ) {
            continue;
        }

        resources.allocateRenderTarget( renderDevice, i, resToAlloc.description );
    }

    for ( uint32_t i = 0; i < bufferCount; i++ ) {
        auto& resToAlloc = buffers[i];
        resources.allocateBuffer( renderDevice, i, resToAlloc.description );
    }

    for ( uint32_t i = 0; i < samplerCount; i++ ) {
        auto& resToAlloc = samplers[i];
        resources.allocateSampler( renderDevice, i, resToAlloc );
    }

    renderPassCount = -1;
    renderTargetCount = 0;
    bufferCount = 0;
    samplerCount = 0;
}

void RenderPipelineBuilder::addRenderPass()
{
    renderPassCount++;
    passRenderTargetRefs[renderPassCount].renderTargetCount = 0;
}

void RenderPipelineBuilder::setPipelineViewport( const Viewport& viewport )
{
    pipelineViewport = viewport;
}

ResHandle_t RenderPipelineBuilder::allocateRenderTarget( TextureDescription& description, const uint32_t flags )
{
    if ( flags & eRenderTargetFlags::USE_PIPELINE_DIMENSIONS ) {
        description.width = pipelineViewport.Width;
        description.height = pipelineViewport.Height;
    }

    if ( flags & eRenderTargetFlags::USE_PIPELINE_SAMPLER_COUNT ) {
        description.samplerCount = 4;
    }

    renderTarget[renderTargetCount] = {
        description,
        flags,
        0u
    };

    auto& passInfos = passRenderTargetRefs[renderPassCount];
    passInfos.renderTarget[passInfos.renderTargetCount++] = renderTargetCount;

    return renderTargetCount++;
}

ResHandle_t RenderPipelineBuilder::allocateBuffer( const BufferDesc& description, const uint32_t shaderStageBinding )
{
    buffers[bufferCount] = {
        description,
        shaderStageBinding
    };

    return bufferCount++;
}

ResHandle_t RenderPipelineBuilder::allocateSampler( const SamplerDesc& description )
{
    samplers[samplerCount] = description;

    return samplerCount++;
}

ResHandle_t RenderPipelineBuilder::readRenderTarget( const ResHandle_t resourceHandle )
{
    renderTarget[resourceHandle].referenceCount++;

    return resourceHandle;
}

RenderPipelineResources::RenderPipelineResources()
    : cbuffers{ 0 }
    , isCBufferFree{ false }
    , cbufferAllocatedCount( 0 )
    , renderTargets{ nullptr }
    , renderTargetsDesc{}
    , isRenderTargetAvailable{ false }
    , rtAllocatedCount( 0 )
    , samplers{ nullptr }
    , samplersDesc{}
    , isSamplerAvailable{ false }
    , samplerAllocatedCount( 0 )
{

}

RenderPipelineResources::~RenderPipelineResources()
{

}

void RenderPipelineResources::releaseResources( RenderDevice* renderDevice )
{
    for ( int cbufferIdx = 0; cbufferIdx < cbufferAllocatedCount; cbufferIdx++ ) {
        renderDevice->destroyBuffer( cbuffers[cbufferIdx] );
    }

    for ( int rtIdx = 0; rtIdx < rtAllocatedCount; rtIdx++ ) {
        renderDevice->destroyRenderTarget( renderTargets[rtIdx] );
    }

    for ( int samplerIdx = 0; samplerIdx < samplerAllocatedCount; samplerIdx++ ) {
        renderDevice->destroySampler( samplers[samplerIdx] );
    }
}

void RenderPipelineResources::unacquireResources()
{
    memset( isCBufferFree, 1, sizeof( bool ) * cbufferAllocatedCount );
    memset( isRenderTargetAvailable, 1, sizeof( bool ) * rtAllocatedCount );
    memset( isSamplerAvailable, 1, sizeof( bool ) * samplerAllocatedCount );
}

void RenderPipelineResources::setPipelineViewport( const Viewport& viewport, const CameraData* cameraData )
{
    activeCameraData = *cameraData;
    activeViewport = viewport;
}

const CameraData* RenderPipelineResources::getMainCamera() const
{
    return &activeCameraData;
}

const Viewport* RenderPipelineResources::getMainViewport() const
{
    return &activeViewport;
}

Buffer* RenderPipelineResources::getBuffer( const ResHandle_t resourceHandle ) const
{
    return allocatedBuffers[resourceHandle];
}

RenderTarget* RenderPipelineResources::getRenderTarget( const ResHandle_t resourceHandle ) const
{
    return allocatedRenderTargets[resourceHandle];
}

Sampler* RenderPipelineResources::getSampler( const ResHandle_t resourceHandle ) const
{
    return allocatedSamplers[resourceHandle];
}

void RenderPipelineResources::allocateBuffer( RenderDevice* renderDevice, const ResHandle_t resourceHandle, const BufferDesc& description )
{
    Buffer* buffer = nullptr;
    
    switch ( description.type ) {
    case BufferDesc::CONSTANT_BUFFER: {
        auto desiredSize = description.size;

        for ( int i = 0; i < cbufferAllocatedCount; i++ ) {
            if ( cbuffersSize[i] == desiredSize && isCBufferFree[i] ) {
                buffer = cbuffers[i];
                isCBufferFree[i] = false;
                break;
            }
        }

        if ( buffer == nullptr ) {
            buffer = renderDevice->createBuffer( description );

            cbuffers[cbufferAllocatedCount] = buffer;
            cbuffersSize[cbufferAllocatedCount] = description.size;
            cbufferAllocatedCount++;
        }
    } break;

    default:
        break;
    }

    allocatedBuffers[resourceHandle] = buffer;
}

void RenderPipelineResources::allocateRenderTarget( RenderDevice* renderDevice, const ResHandle_t resourceHandle, const TextureDescription& description )
{
    RenderTarget* renderTarget = nullptr;

    for ( int i = 0; i < rtAllocatedCount; i++ ) {
        if ( renderTargetsDesc[i] == description && isRenderTargetAvailable[i] ) {
            renderTarget = renderTargets[i];
            isRenderTargetAvailable[i] = false;
            break;
        }
    }

    if ( renderTarget == nullptr ) {
        switch ( description.dimension ) {
        case TextureDescription::DIMENSION_TEXTURE_1D:
            renderTarget = renderDevice->createRenderTarget1D( description );
            break;
        case TextureDescription::DIMENSION_TEXTURE_2D:
            renderTarget = renderDevice->createRenderTarget2D( description );
            break;
        case TextureDescription::DIMENSION_TEXTURE_3D:
            renderTarget = renderDevice->createRenderTarget3D( description );
            break;
        default:
            break;
        }

        renderTargets[rtAllocatedCount] = renderTarget;
        renderTargetsDesc[rtAllocatedCount] = description;
        rtAllocatedCount++;
    }

    allocatedRenderTargets[resourceHandle] = renderTarget;
}

void RenderPipelineResources::allocateSampler( RenderDevice* renderDevice, const ResHandle_t resourceHandle, const SamplerDesc& description )
{
    Sampler* sampler = nullptr;

    for ( int i = 0; i < samplerAllocatedCount; i++ ) {
        if ( samplersDesc[i] == description && isSamplerAvailable[i] ) {
            sampler = samplers[i];
            isSamplerAvailable[i] = false;
            break;
        }
    }

    if ( sampler == nullptr ) {
        sampler = renderDevice->createSampler( description );

        samplers[samplerAllocatedCount] = sampler;
        samplersDesc[samplerAllocatedCount] = description;
        samplerAllocatedCount++;
    }

    allocatedSamplers[resourceHandle] = sampler;
}

RenderPipeline::RenderPipeline()
    : renderPasses{ 0 } 
    , renderPassCount( 0 )
    , passGroupStartIndexes{ 0u }
    , passGroupCount( 0 )
{

}

RenderPipeline::~RenderPipeline()
{

}

void RenderPipeline::destroy( RenderDevice* renderDevice )
{
    renderPipelineResources.releaseResources( renderDevice );
}

void RenderPipeline::enableProfiling( RenderDevice* renderDevice )
{

}

void RenderPipeline::beginPassGroup()
{
    passGroupStartIndexes[passGroupCount++] = renderPassCount;
}

void RenderPipeline::execute( RenderDevice* renderDevice )
{
    // Cull & compile
    renderPipelineBuilder.cullRenderPasses( renderPasses, renderPassCount );
    renderPipelineBuilder.compile( renderDevice, renderPipelineResources );

    // Execute render passes
    CommandList& cmdList = renderDevice->allocateGraphicsCommandList();

    cmdList.begin();
    cmdList.setViewport( activeViewport );

    for ( int passIdx = 0; passIdx < renderPassCount; passIdx++ ) {
        renderPasses[passIdx].execute( renderPipelineResources, renderDevice, &cmdList );
    }

    cmdList.end();

    renderDevice->submitCommandList( &cmdList );

    renderPassCount = 0;
    passGroupCount = 0;
}

void RenderPipeline::submitAndDispatchDrawCmds( DrawCmd* drawCmds, const size_t drawCmdCount )
{
}

void RenderPipeline::setViewport( const Viewport& viewport, const CameraData* camera )
{
    activeViewport = viewport;

    renderPipelineBuilder.setPipelineViewport( viewport );

    if ( camera != nullptr ) {
        renderPipelineResources.setPipelineViewport( viewport, camera );
    }
}
