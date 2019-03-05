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

#include "GraphicsProfiler.h"

#include <Rendering/ImageFormat.h>

RenderPipelineBuilder::RenderPipelineBuilder()
    : passRefs{ {0} }
    , renderPassCount( -1 )
    , renderTargetCount( 0 )
    , bufferCount( 0 )
    , samplerCount( 0 )
    , persitentBufferCount( 0 )
    , persitentRenderTargetCount( 0 )
    , pipelineSamplerCount( 1 )
    , pipelineImageQuality( 1.0f )
{

}

RenderPipelineBuilder::~RenderPipelineBuilder()
{

}

void RenderPipelineBuilder::cullRenderPasses( RenderPipelineRenderPass* renderPassList, int& renderPassListLength )
{
    int tmpRenderPassCount = 0;

    for ( int32_t i = 0; i < renderPassListLength; i++ ) {
        auto& passInfos = passRefs[i];

        bool cullPass = true;

        if ( passInfos.renderTargetCount == static_cast< uint32_t >( -1 ) ) {
            cullPass = false;
        } else {
            for ( uint32_t j = 0; j < passInfos.renderTargetCount; j++ ) {
                if ( renderTargets[passInfos.renderTargets[j]].referenceCount > 0 ) {
                    cullPass = false;
                    break;
                }
            }

            for ( uint32_t j = 0; j < passInfos.buffersCount; j++ ) {
                if ( buffers[passInfos.buffers[j]].referenceCount > 0 ) {
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
    passRefs[renderPassCount].renderTargetCount = static_cast< uint32_t >( -1 );
}

void RenderPipelineBuilder::compile( RenderDevice* renderDevice, RenderPipelineResources& resources )
{
    resources.unacquireResources();

    for ( uint32_t i = 0; i < renderTargetCount; i++ ) {
        auto& resToAlloc = renderTargets[i];

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

    for ( uint32_t i = 0; i < persitentBufferCount; i++ ) {
        resources.bindPersistentBuffers( i, persitentBuffers[i] );
    }

    for ( uint32_t i = 0; i < persitentRenderTargetCount; i++ ) {
        resources.bindPersistentRenderTargets( i, persitentRenderTargets[i] );
    }
    
    renderPassCount = -1;
    renderTargetCount = 0;
    bufferCount = 0;
    samplerCount = 0;
    persitentBufferCount = 0;
    persitentRenderTargetCount = 0;
}

void RenderPipelineBuilder::addRenderPass()
{
    renderPassCount++;
    passRefs[renderPassCount].renderTargetCount = 0;
    passRefs[renderPassCount].buffersCount = 0;
}

void RenderPipelineBuilder::setPipelineViewport( const Viewport& viewport )
{
    pipelineViewport = viewport;
}

void RenderPipelineBuilder::setMSAAQuality( const uint32_t samplerCount )
{
    pipelineSamplerCount = samplerCount;
}

void RenderPipelineBuilder::setImageQuality( const float imageQuality )
{
    pipelineImageQuality = imageQuality;
}

ResHandle_t RenderPipelineBuilder::allocateRenderTarget( TextureDescription& description, const uint32_t flags )
{
    if ( flags & eRenderTargetFlags::USE_PIPELINE_DIMENSIONS ) {
        description.width = static_cast<uint32_t>( pipelineViewport.Width * pipelineImageQuality );
        description.height = static_cast<uint32_t>( pipelineViewport.Height * pipelineImageQuality );
    }

    if ( flags & eRenderTargetFlags::USE_PIPELINE_SAMPLER_COUNT ) {
        description.samplerCount = pipelineSamplerCount; 
        description.flags.useMultisamplePattern = ( pipelineSamplerCount > 1 );
    }

    renderTargets[renderTargetCount] = {
        description,
        flags,
        0u
    };

    auto& passInfos = passRefs[renderPassCount];
    passInfos.renderTargets[passInfos.renderTargetCount++] = renderTargetCount;

    return renderTargetCount++;
}

ResHandle_t RenderPipelineBuilder::copyRenderTarget( const ResHandle_t resourceToCopy, const uint32_t copyFlags )
{
    renderTargets[renderTargetCount].description = renderTargets[resourceToCopy].description;
    renderTargets[renderTargetCount].flags = 0u;
    renderTargets[renderTargetCount].referenceCount = 0u;

    if ( copyFlags & eRenderTargetCopyFlags::NO_MULTISAMPLE ) {
        renderTargets[renderTargetCount].description.samplerCount = 1u;
        renderTargets[renderTargetCount].description.flags.useMultisamplePattern = 0;
    }

    auto& passInfos = passRefs[renderPassCount];
    passInfos.renderTargets[passInfos.renderTargetCount++] = renderTargetCount;

    return renderTargetCount++;
}

ResHandle_t RenderPipelineBuilder::allocateBuffer( BufferDesc& description, const uint32_t shaderStageBinding, const uint32_t flags )
{
    if ( flags & eRenderTargetFlags::USE_PIPELINE_DIMENSIONS ) {
        description.width = pipelineViewport.Width;
        description.height = pipelineViewport.Height;
    }

    buffers[bufferCount] = {
        description,
        shaderStageBinding,
        0u
    };

    auto& passInfos = passRefs[renderPassCount];
    passInfos.buffers[passInfos.buffersCount++] = bufferCount;

    return bufferCount++;
}

ResHandle_t RenderPipelineBuilder::allocateSampler( const SamplerDesc& description )
{
    samplers[samplerCount] = description;

    return samplerCount++;
}

ResHandle_t RenderPipelineBuilder::readRenderTarget( const ResHandle_t resourceHandle )
{
    renderTargets[resourceHandle].referenceCount++;

    return resourceHandle;
}

ResHandle_t RenderPipelineBuilder::readBuffer( const ResHandle_t resourceHandle )
{
    buffers[resourceHandle].referenceCount++;

    return resourceHandle;
}

ResHandle_t RenderPipelineBuilder::retrievePersistentRenderTarget( const nyaStringHash_t resourceHashcode )
{
    persitentRenderTargets[persitentRenderTargetCount] = resourceHashcode;

    // NOTE If a render pass uses a persitent resource, it implicitly become uncullable (since persistent resources
    // have no reference tracking)
    setUncullablePass();

    return persitentRenderTargetCount++;
}

ResHandle_t RenderPipelineBuilder::retrievePersistentBuffer( const nyaStringHash_t resourceHashcode )
{
    persitentBuffers[persitentBufferCount] = resourceHashcode;

    // NOTE If a render pass uses a persitent resource, it implicitly become uncullable (since persistent resources
    // have no reference tracking)
    setUncullablePass();

    return persitentBufferCount++;
}

RenderPipelineResources::RenderPipelineResources()
    : cbuffers{ 0 }
    , isCBufferFree{ false }
    , cbufferAllocatedCount( 0 )
    , genBuffer{ 0 }
    , isGenBufferFree{ false }
    , genAllocatedCount( 0 )
    , uavTex2dBuffer{ 0 }
    , isUavTex2dBufferFree{ false }
    , uavTex2dAllocatedCount( 0 )
    , uavBuffer{ 0 }
    , isUavBufferFree{ false }
    , uavBufferAllocatedCount( 0 )
    , renderTargets{ nullptr }
    , renderTargetsDesc{}
    , isRenderTargetAvailable{ false }
    , rtAllocatedCount( 0 )
    , samplers{ nullptr }
    , samplersDesc{}
    , isSamplerAvailable{ false }
    , samplerAllocatedCount( 0 )
    , allocatedPersistentBuffers{ nullptr }
    , allocatedPersistentRenderTargets{ nullptr }
    , pipelineImageQuality( 1.0f )
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

    for ( int uav2dIdx = 0; uav2dIdx < uavTex2dAllocatedCount; uav2dIdx++ ) {
        renderDevice->destroyBuffer( uavTex2dBuffer[uav2dIdx] );
    }

    for ( int uavBufferIdx = 0; uavBufferIdx < uavBufferAllocatedCount; uavBufferIdx++ ) {
        renderDevice->destroyBuffer( uavBuffer[uavBufferIdx] );
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
    memset( isUavTex2dBufferFree, 1, sizeof( bool ) * uavTex2dAllocatedCount );
    memset( isUavBufferFree, 1, sizeof( bool ) * uavBufferAllocatedCount );
    memset( isRenderTargetAvailable, 1, sizeof( bool ) * rtAllocatedCount );
    memset( isSamplerAvailable, 1, sizeof( bool ) * samplerAllocatedCount );
}

void RenderPipelineResources::setPipelineViewport( const Viewport& viewport, const CameraData* cameraData )
{
    activeCameraData = *cameraData;
    activeViewport = viewport;
}

void RenderPipelineResources::setImageQuality( const float imageQuality )
{
    pipelineImageQuality = imageQuality;
}

// Copy instance data to shared buffer
void RenderPipelineResources::updateVectorBuffer( const DrawCmd& cmd, size_t& instanceBufferOffset )
{
    switch ( cmd.key.bitfield.layer ) {
    case DrawCommandKey::LAYER_DEPTH: {
        const size_t instancesDataSize = sizeof( nyaMat4x4f ) * cmd.infos.instanceCount;
        
        nyaMat4x4f mvp = *cmd.infos.modelMatrix * activeCameraData.shadowViewMatrix[cmd.key.bitfield.viewportLayer - 1u];

        memcpy( ( uint8_t* )instanceBufferData + instanceBufferOffset, &mvp, sizeof( nyaMat4x4f ) );
        //memcpy( ( uint8_t* )instanceBufferData + instanceBufferOffset + sizeof( nyaMat4x4f ), &activeCameraData.shadowViewMatrix[cmd.key.bitfield.viewportLayer - 1u], sizeof( nyaMat4x4f ) );

        instanceBufferOffset += instancesDataSize;
    } break;

    case DrawCommandKey::LAYER_WORLD:
    default: {
        const size_t instancesDataSize = sizeof( nyaMat4x4f ) * cmd.infos.instanceCount;

        memcpy( ( uint8_t* )instanceBufferData + instanceBufferOffset, cmd.infos.modelMatrix, instancesDataSize );

        instanceBufferOffset += instancesDataSize;
    } break;
    }
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

    size_t instanceBufferOffset = 0ull;

    updateVectorBuffer( drawCmds[0], instanceBufferOffset );

    DrawCmdBucket* previousBucket = &drawCmdBuckets[layer][viewportLayer];
    previousBucket->instanceDataStartOffset = 0.0f;

 
    previousBucket->vectorPerInstance = static_cast< float >( sizeof( nyaMat4x4f ) / sizeof( nyaVec4f ) );
    
    for ( size_t drawCmdIdx = 1; drawCmdIdx < drawCmdCount; drawCmdIdx++ ) {
        const auto& drawCmdKey = drawCmds[drawCmdIdx].key.bitfield;

        if ( layer != drawCmdKey.layer || viewportLayer != drawCmdKey.viewportLayer ) {
            previousBucket->endAddr = ( drawCmds + drawCmdIdx );

            auto& bucket = drawCmdBuckets[drawCmdKey.layer][drawCmdKey.viewportLayer];
            bucket.beginAddr = ( drawCmds + drawCmdIdx );

            bucket.vectorPerInstance = static_cast< float >( sizeof( nyaMat4x4f ) / sizeof( nyaVec4f ) );
          
            bucket.instanceDataStartOffset = static_cast< float >( instanceBufferOffset / sizeof( nyaVec4f ) );

            layer = drawCmdKey.layer;
            viewportLayer = drawCmdKey.viewportLayer;

            previousBucket = &drawCmdBuckets[drawCmdKey.layer][drawCmdKey.viewportLayer];
        }

        updateVectorBuffer( drawCmds[drawCmdIdx], instanceBufferOffset );
    }

    previousBucket->endAddr = ( drawCmds + drawCmdCount );
}

void RenderPipelineResources::importPersistentRenderTarget( const nyaStringHash_t resourceHashcode, RenderTarget* renderTarget )
{
    persistentRenderTarget[resourceHashcode] = renderTarget;
}

void RenderPipelineResources::importPersistentBuffer( const nyaStringHash_t resourceHashcode, Buffer* buffer )
{
    persistentBuffers[resourceHashcode] = buffer;
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

void RenderPipelineResources::updateDeltaTime( const float dt )
{
    deltaTime = dt;
}

const float RenderPipelineResources::getDeltaTime() const
{
    return deltaTime;
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

Buffer* RenderPipelineResources::getPersistentBuffer( const ResHandle_t resourceHandle ) const
{
    return allocatedPersistentBuffers[resourceHandle];
}

RenderTarget* RenderPipelineResources::getPersitentRenderTarget( const ResHandle_t resourceHandle ) const
{
    return allocatedPersistentRenderTargets[resourceHandle];
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

    case BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER: {
        for ( int i = 0; i < uavBufferAllocatedCount; i++ ) {
            if ( uavBufferDesc[i].viewFormat == description.viewFormat
                && uavBufferDesc[i].size == description.size
                && uavBufferDesc[i].singleElementSize == description.singleElementSize
                && isUavBufferFree[i] ) {
                buffer = uavBuffer[i];
                isUavBufferFree[i] = false;
                break;
            }
        }

        if ( buffer == nullptr ) {
            buffer = renderDevice->createBuffer( description );

            uavBuffer[uavBufferAllocatedCount] = buffer;
            uavBufferDesc[uavBufferAllocatedCount] = description;
            uavBufferAllocatedCount++;
        }
    } break;

    case BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_1D:
    case BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D:
    case BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_3D: {
        for ( int i = 0; i < uavTex2dAllocatedCount; i++ ) {
            if ( uavTex2dBufferDesc[i].type == description.type
                && uavTex2dBufferDesc[i].height == description.height
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

void RenderPipelineResources::bindPersistentBuffers( const ResHandle_t resourceHandle, const nyaStringHash_t hashcode )
{
    auto it = persistentBuffers.find( hashcode );
    allocatedPersistentBuffers[resourceHandle] = ( it != persistentBuffers.end() ) ? it->second : nullptr;
}

void RenderPipelineResources::bindPersistentRenderTargets( const ResHandle_t resourceHandle, const nyaStringHash_t hashcode )
{
    auto it = persistentRenderTarget.find( hashcode );
    allocatedPersistentRenderTargets[resourceHandle] = ( it != persistentRenderTarget.end() ) ? it->second : nullptr;
}

bool RenderPipelineResources::isPersistentRenderTargetAvailable( const nyaStringHash_t resourceHashcode ) const
{
    return ( persistentRenderTarget.find( resourceHashcode ) != persistentRenderTarget.end() );
}

bool RenderPipelineResources::isPersistentBufferAvailable( const nyaStringHash_t resourceHashcode ) const
{
    return ( persistentBuffers.find( resourceHashcode ) != persistentBuffers.end() );
}

RenderPipeline::RenderPipeline( BaseAllocator* allocator )
    : memoryAllocator( allocator )
    , renderPasses{ 0 }
    , renderPassCount( 0 )
    , passGroupStartIndexes{ 0u }
    , passGroupCount( 0 )
    , graphicsProfiler( nullptr )
    , activeViewport{ 0 }
    , hasViewportChanged( false )
    , pipelineImageQuality( 1.0f )
    , lastFrameRenderTarget( nullptr )
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

    if ( graphicsProfiler != nullptr ) {
        graphicsProfiler->destroy( renderDevice );
        nya::core::free<GraphicsProfiler>( memoryAllocator, graphicsProfiler );
    }

    if ( lastFrameRenderTarget != nullptr ) {
        renderDevice->destroyRenderTarget( lastFrameRenderTarget );
    }
}

void RenderPipeline::enableProfiling( RenderDevice* renderDevice )
{
    graphicsProfiler = nya::core::allocate<GraphicsProfiler>( memoryAllocator );
    graphicsProfiler->create( renderDevice );
}

void RenderPipeline::beginPassGroup()
{
    passGroupStartIndexes[passGroupCount++] = renderPassCount;
}

void RenderPipeline::execute( RenderDevice* renderDevice, const float deltaTime )
{
    NYA_PROFILE_FUNCTION

    // Update per-frame renderer infos
    renderPipelineResources.updateDeltaTime( deltaTime );

    // Check if we need to reallocate persitent resources
    if ( hasViewportChanged ) {
        if ( lastFrameRenderTarget != nullptr ) {
            renderDevice->destroyRenderTarget( lastFrameRenderTarget );
        }

        TextureDescription lastFrameDesc = {};
        lastFrameDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
        lastFrameDesc.format = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;
        lastFrameDesc.width = static_cast<uint32_t>( activeViewport.Width * pipelineImageQuality );
        lastFrameDesc.height = static_cast<uint32_t>( activeViewport.Height * pipelineImageQuality );
        lastFrameDesc.depth = 1;
        lastFrameDesc.mipCount = 1;
        lastFrameDesc.samplerCount = 1;

        lastFrameRenderTarget = renderDevice->createRenderTarget2D( lastFrameDesc );

        // Import the new render target
        renderPipelineResources.importPersistentRenderTarget( NYA_STRING_HASH( "LastFrameRenderTarget" ), lastFrameRenderTarget );

        hasViewportChanged = false;
    }

    // Cull & compile
    renderPipelineBuilder.cullRenderPasses( renderPasses, renderPassCount );
    renderPipelineBuilder.compile( renderDevice, renderPipelineResources );

    // Execute render passes
    CommandList& cmdList = renderDevice->allocateGraphicsCommandList();

    cmdList.begin();
    cmdList.setViewport( activeViewport );

#if NYA_DEVBUILD
    if ( graphicsProfiler != nullptr ) {
        NYA_BEGIN_PROFILE_SCOPE( "RenderPasses Execution" );
            for ( int passIdx = 0; passIdx < renderPassCount; passIdx++ ) {
                NYA_BEGIN_PROFILE_SCOPE( renderPasses[passIdx].name );
                    graphicsProfiler->beginSection( &cmdList, renderPasses[passIdx].name );
                        renderPasses[passIdx].execute( renderPipelineResources, renderDevice, &cmdList );
                    graphicsProfiler->endSection( &cmdList );
                NYA_END_PROFILE_SCOPE()
            }
        NYA_END_PROFILE_SCOPE()

        graphicsProfiler->onFrame( renderDevice );
    } else {
        for ( int passIdx = 0; passIdx < renderPassCount; passIdx++ ) {
            renderPasses[passIdx].execute( renderPipelineResources, renderDevice, &cmdList );
        }
    }
#else
    for ( int passIdx = 0; passIdx < renderPassCount; passIdx++ ) {
        renderPasses[passIdx].execute( renderPipelineResources, renderDevice, &cmdList );
    }
#endif

    cmdList.end();

    renderDevice->submitCommandList( &cmdList );

    renderPassCount = 0;
    passGroupCount = 0;
}

void RenderPipeline::submitAndDispatchDrawCmds( DrawCmd* drawCmds, const size_t drawCmdCount )
{
    NYA_PROFILE_FUNCTION

    renderPipelineResources.dispatchToBuckets( drawCmds, drawCmdCount );
}

void RenderPipeline::setViewport( const Viewport& viewport, const CameraData* camera )
{
    hasViewportChanged = ( activeViewport != viewport );
    activeViewport = viewport;

    renderPipelineBuilder.setPipelineViewport( viewport );

    if ( camera != nullptr ) {
        renderPipelineResources.setPipelineViewport( viewport, camera );
    }
}

void RenderPipeline::setMSAAQuality( const uint32_t samplerCount )
{
    renderPipelineBuilder.setMSAAQuality( samplerCount );
}

void RenderPipeline::setImageQuality( const float imageQuality )
{
    renderPipelineBuilder.setImageQuality( imageQuality );
    renderPipelineResources.setImageQuality( imageQuality );

    pipelineImageQuality = imageQuality;
}

void RenderPipeline::importPersistentRenderTarget( const nyaStringHash_t resourceHashcode, RenderTarget* renderTarget )
{
    renderPipelineResources.importPersistentRenderTarget( resourceHashcode, renderTarget );
}

void RenderPipeline::importPersistentBuffer( const nyaStringHash_t resourceHashcode, Buffer* buffer )
{
    renderPipelineResources.importPersistentBuffer( resourceHashcode, buffer );
}

#if NYA_DEVBUILD
const char* RenderPipeline::getProfilingSummary() const
{
    const std::string& proflingString = graphicsProfiler->getProfilingSummaryString();
    return ( !proflingString.empty() ) ? proflingString.c_str() : nullptr;
}
#endif