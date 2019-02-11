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
    , genBuffer{ 0 }
    , isGenBufferFree{ false }
    , genAllocatedCount( 0 )
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

void RenderPipelineResources::create( BaseAllocator* allocator )
{
    instanceBufferData = nya::core::allocateArray<uint8_t>( allocator, sizeof( nyaVec4f ) * 1024 );
    memset( instanceBufferData, 0, sizeof( nyaVec4f ) * 1024 );
}

void RenderPipelineResources::destroy( BaseAllocator* allocator )
{
    nya::core::freeArray<uint8_t>( allocator, (uint8_t*)instanceBufferData );
}

void RenderPipelineResources::releaseResources( RenderDevice* renderDevice )
{
    for ( int cbufferIdx = 0; cbufferIdx < cbufferAllocatedCount; cbufferIdx++ ) {
        renderDevice->destroyBuffer( cbuffers[cbufferIdx] );
    }

    for ( int genIdx = 0; genIdx < genAllocatedCount; genIdx++ ) {
        renderDevice->destroyBuffer( genBuffer[genIdx] );
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
    memset( isGenBufferFree, 1, sizeof( bool ) * genAllocatedCount );
    memset( isRenderTargetAvailable, 1, sizeof( bool ) * rtAllocatedCount );
    memset( isSamplerAvailable, 1, sizeof( bool ) * samplerAllocatedCount );
}

void RenderPipelineResources::setPipelineViewport( const Viewport& viewport, const CameraData* cameraData )
{
    activeCameraData = *cameraData;
    activeViewport = viewport;
}

void RenderPipelineResources::dispatchToBuckets( DrawCmd* drawCmds, const size_t drawCmdCount )
{
    if ( drawCmdCount == 0 ) {
        return;
    }

    const auto& firstDrawCmdKey = drawCmds[0].key.bitfield;

    DrawCommandKey::Layer layer = firstDrawCmdKey.layer;
    uint8_t viewportLayer = firstDrawCmdKey.viewportLayer;

    drawCmdBuckets[layer][viewportLayer].beginAddr = ( drawCmds + 0 );

    size_t instanceBufferOffset = 0;

    // Copy instance data to shared buffer
    const size_t instancesDataSize = sizeof( nyaMat4x4f ) * drawCmds[0].infos.instanceCount;
    memcpy( ( uint8_t* )instanceBufferData + instanceBufferOffset, drawCmds[0].infos.modelMatrix, instancesDataSize );
    instanceBufferOffset += instancesDataSize;

    DrawCmdBucket* previousBucket = &drawCmdBuckets[layer][viewportLayer];

    for ( size_t drawCmdIdx = 1; drawCmdIdx < drawCmdCount; drawCmdIdx++ ) {
        const auto& drawCmdKey = drawCmds[drawCmdIdx].key.bitfield;

        // Copy instance data to shared buffer
        const size_t instancesDataSize = sizeof( nyaMat4x4f ) * drawCmds[drawCmdIdx].infos.instanceCount;
        memcpy( ( uint8_t* )instanceBufferData + instanceBufferOffset, drawCmds[drawCmdIdx].infos.modelMatrix, instancesDataSize );
        instanceBufferOffset += instancesDataSize;

        if ( layer != drawCmdKey.layer || viewportLayer != drawCmdKey.viewportLayer ) {
            previousBucket->endAddr = ( drawCmds + drawCmdIdx );
            auto& bucket = drawCmdBuckets[drawCmdKey.layer][drawCmdKey.viewportLayer];
            bucket.beginAddr = ( drawCmds + drawCmdIdx );
            bucket.vectorPerInstance = static_cast< float >( sizeof( nyaMat4x4f ) / sizeof( nyaVec4f ) );
            bucket.instanceDataStartOffset = static_cast< float >( instanceBufferOffset );

            layer = drawCmdKey.layer;
            viewportLayer = drawCmdKey.viewportLayer;

            previousBucket = &drawCmdBuckets[drawCmdKey.layer][drawCmdKey.viewportLayer];
        }
    }

    previousBucket->endAddr = ( drawCmds + drawCmdCount );
}

const RenderPipelineResources::DrawCmdBucket& RenderPipelineResources::getDrawCmdBucket( const DrawCommandKey::Layer layer, const uint8_t viewportLayer ) const
{
    return drawCmdBuckets[layer][viewportLayer];
}

void* RenderPipelineResources::getVectorBufferData() const
{
    return instanceBufferData;
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

    case BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D: {
        for ( int i = 0; i < uavTex2dAllocatedCount; i++ ) {
            if ( uavTex2dBufferDesc[i].height == description.height
                && uavTex2dBufferDesc[i].width == description.width
                && uavTex2dBufferDesc[i].depth == description.depth
                && uavTex2dBufferDesc[i].mipCount == description.mipCount
                && uavTex2dBufferDesc[i].viewFormat == description.viewFormat
                && isUavTex2dBufferFree[i] ) {
                buffer = uavTex2dBuffer[i];
                isUavTex2dBufferFree[i] = false;
                break;
            }
        }

        if ( buffer == nullptr ) {
            buffer = renderDevice->createBuffer( description );

            uavTex2dBuffer[uavTex2dAllocatedCount] = buffer;
            uavTex2dBufferDesc[uavTex2dAllocatedCount] = description;
            uavTex2dAllocatedCount++;
        }
    } break;

    case BufferDesc::GENERIC_BUFFER: {
        for ( int i = 0; i < genAllocatedCount; i++ ) {
            if ( genBufferDesc[i].size == description.size 
                && genBufferDesc[i].stride == description.stride
                && genBufferDesc[i].viewFormat == description.viewFormat
                && isGenBufferFree[i] ) {
                buffer = genBuffer[i];
                isGenBufferFree[i] = false;
                break;
            }
        }

        if ( buffer == nullptr ) {
            buffer = renderDevice->createBuffer( description );

            genBuffer[genAllocatedCount] = buffer;
            genBufferDesc[genAllocatedCount] = description;
            genAllocatedCount++;
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

RenderPipeline::RenderPipeline( BaseAllocator* allocator )
    : memoryAllocator( allocator )
    , renderPasses{ 0 }
    , renderPassCount( 0 )
    , passGroupStartIndexes{ 0u }
    , passGroupCount( 0 )
{
    renderPipelineResources.create( allocator );
}

RenderPipeline::~RenderPipeline()
{

}

void RenderPipeline::destroy( RenderDevice* renderDevice )
{
    renderPipelineResources.releaseResources( renderDevice );
    renderPipelineResources.destroy( memoryAllocator );
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
    renderPipelineResources.dispatchToBuckets( drawCmds, drawCmdCount );
}

void RenderPipeline::setViewport( const Viewport& viewport, const CameraData* camera )
{
    activeViewport = viewport;

    renderPipelineBuilder.setPipelineViewport( viewport );

    if ( camera != nullptr ) {
        renderPipelineResources.setPipelineViewport( viewport, camera );
    }
}
