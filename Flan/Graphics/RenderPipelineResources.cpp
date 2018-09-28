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
#include "RenderPipelineResources.h"

#include "ShaderStageManager.h"

#include <Rendering/RenderTarget.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

RenderPipelineResources::RenderPipelineResources()
    : layerBuckets{}
    , layerBucketsCmdCount{ 0 }
    , rendererTimeDelta( 0.0f )
{
    importedResourcesIndexing = ~0u;
}

RenderPipelineResources::~RenderPipelineResources()
{

}

void RenderPipelineResources::destroy( RenderDevice* renderDevice )
{
    for ( int i = 0; i < allocatedRenderTargets.size(); i++ ) {
        allocatedRenderTargets[i]->destroy( renderDevice );
    }

    for ( int j = 0; j < allocatedBuffers.size(); j++ ) {
        allocatedBuffers[j]->destroy( renderDevice );
    }

    for ( auto it = allocatedPipelineStates.begin(); 
        it != allocatedPipelineStates.end(); 
        it++ ) {
        it->second->destroy( renderDevice );
    }

    for ( int l = 0; l < allocatedRasterizerStates.size(); l++ ) {
        allocatedRasterizerStates[l]->destroy( renderDevice );
    }

    for ( int m = 0; m < allocatedDepthStencilStates.size(); m++ ) {
        allocatedDepthStencilStates[m]->destroy( renderDevice );
    }

    for ( int n = 0; n < allocatedBlendStates.size(); n++ ) {
        allocatedBlendStates[n]->destroy( renderDevice );
    }
}

PipelineState* RenderPipelineResources::getPipelineState( const fnPipelineResHandle_t pipelineStateHandle ) const
{
    auto iterator = reservedPipelineStates.find( pipelineStateHandle );

    if ( iterator == reservedPipelineStates.end() ) {
        FLAN_CWARN << "Could not find pipeline state resource with handle " << FLAN_TO_STRING( pipelineStateHandle ) << std::endl;
        return nullptr;
    }

    return iterator->second;
}

RenderTarget* RenderPipelineResources::getRenderTarget( const fnPipelineResHandle_t renderTargetHandle ) const
{
    auto iterator = reservedRenderTargets.find( renderTargetHandle );

    if ( iterator == reservedRenderTargets.end() ) {
        FLAN_CWARN << "Could not find render pass resource with handle " << FLAN_TO_STRING( renderTargetHandle ) << std::endl;
        return nullptr;
    }

    return iterator->second;
}

Sampler* RenderPipelineResources::getSampler( const fnPipelineResHandle_t renderTargetHandle ) const
{
    auto iterator = reservedSamplers.find( renderTargetHandle );

    if ( iterator == reservedSamplers.end() ) {
        FLAN_CWARN << "Could not find render pass resource with handle " << FLAN_TO_STRING( renderTargetHandle ) << std::endl;
        return nullptr;
    }

    return iterator->second;
}

Buffer* RenderPipelineResources::getBuffer( const fnPipelineResHandle_t bufferHandle ) const
{
    auto iterator = reservedBuffers.find( bufferHandle );

    if ( iterator == reservedBuffers.end() ) {
        FLAN_CWARN << "Could not find buffer resource with handle " << FLAN_TO_STRING( bufferHandle ) << std::endl;
        return nullptr;
    }

    return iterator->second;
}

void RenderPipelineResources::allocateOrReusePipelineState( RenderDevice* renderDevice, ShaderStageManager* shaderStageManager, fnPipelineResHandle_t pipelineStateHandle, const RenderPassPipelineStateDesc& pipelineStateDesc )
{
    auto iterator = allocatedPipelineStates.find( pipelineStateDesc.hashcode );

    // Pipeline State has already been loaded
    if ( iterator != allocatedPipelineStates.end() ) {
        reservedPipelineStates.insert( std::make_pair( pipelineStateHandle, iterator->second.get() ) );
        return;
    }

    PipelineStateDesc psoDescription;

    // Shader stages
    if ( !pipelineStateDesc.vertexStage.empty() ) {
        psoDescription.vertexStage = shaderStageManager->getOrUploadStage( pipelineStateDesc.vertexStage.c_str(), SHADER_STAGE_VERTEX );
    }

    if ( !pipelineStateDesc.pixelStage.empty() ) {
        psoDescription.pixelStage = shaderStageManager->getOrUploadStage( pipelineStateDesc.pixelStage.c_str(), SHADER_STAGE_PIXEL );
    }

    if ( !pipelineStateDesc.tesselationControlStage.empty() ) {
        psoDescription.tesselationControlStage = shaderStageManager->getOrUploadStage( pipelineStateDesc.tesselationControlStage.c_str(), SHADER_STAGE_TESSELATION_CONTROL );
    }

    if ( !pipelineStateDesc.tesselationEvalStage.empty() ) {
        psoDescription.tesselationEvalStage = shaderStageManager->getOrUploadStage( pipelineStateDesc.tesselationEvalStage.c_str(), SHADER_STAGE_TESSELATION_EVALUATION );
    }

    if ( !pipelineStateDesc.computeStage.empty() ) {
        psoDescription.computeStage = shaderStageManager->getOrUploadStage( pipelineStateDesc.computeStage.c_str(), SHADER_STAGE_COMPUTE );
    }

    // Primitive Topology
    psoDescription.primitiveTopology = pipelineStateDesc.primitiveTopology;

    // Find (or create) reusable objects
    psoDescription.rasterizerState = findSuitableRasterizerState( renderDevice, pipelineStateDesc.rasterizerState );
    psoDescription.depthStencilState = findSuitableDepthStencilState( renderDevice, pipelineStateDesc.depthStencilState );
    psoDescription.blendState = findSuitableBlendState( renderDevice, pipelineStateDesc.blendState );

    // TODO Pool Input Layout aswell? (keep the shader name somewhere so that we know which bytecode the input layout uses?)
    psoDescription.inputLayout = pipelineStateDesc.inputLayout;

    PipelineState* pipelineState = new PipelineState();
    pipelineState->create( renderDevice, psoDescription );

    allocatedPipelineStates.insert( std::make_pair( pipelineStateDesc.hashcode, std::unique_ptr<PipelineState>( pipelineState ) ) );
    reservedPipelineStates.insert( std::make_pair( pipelineStateHandle, pipelineState ) );
}

inline bool IsRenderTargetSuitable( const RenderPassTextureDesc& renderPassTexture, const RenderTarget* renderTargetTexture )
{
    return ( renderPassTexture.description == renderTargetTexture->getDescription() );
}

void RenderPipelineResources::allocateOrReuseRenderTarget( RenderDevice* renderDevice, CommandList* cmdList, fnPipelineResHandle_t renderTargetHandle, const RenderPassTextureDesc& textureDescription )
{
    // TODO This should be pooled instead
    // This would be much more efficient especially for DX11 era API
    for ( int32_t i = 0; i < allocatedRenderTargets.size(); i++ ) {
        // TODO Optimize me :( (a binary search might be more efficient?)
        if ( std::find( reservedRenderTargetsIndexes.begin(), reservedRenderTargetsIndexes.end(), i ) != reservedRenderTargetsIndexes.end() ) {
            continue;
        }

        auto& availableRenderTarget = allocatedRenderTargets[i];
        if ( IsRenderTargetSuitable( textureDescription, availableRenderTarget.get() ) ) {
            // Clear the render target if asked to 
            if ( textureDescription.initialState != RenderPassTextureDesc::DONT_CARE ) {
                if ( textureDescription.description.flags.isDepthResource ) {
                    cmdList->bindRenderTargetsCmd( nullptr, availableRenderTarget.get(), 0 );
                    cmdList->setClearDepthValueCmd( ( textureDescription.initialState == RenderPassTextureDesc::CLEAR_DEPTH_REVERSED_Z ) ? 0.0f : 1.0f );
                    cmdList->clearDepthRenderTargetCmd();
                } else {
                    RenderTarget* renderTargets[1] = { availableRenderTarget.get() };
                    cmdList->bindRenderTargetsCmd( renderTargets );
                    cmdList->clearColorRenderTargetCmd();
                }
            } 

            reservedRenderTargetsIndexes.push_back( i );
            reservedRenderTargets.insert( std::make_pair( renderTargetHandle, availableRenderTarget.get() ) );
            return;
        }
    }

    auto createdTarget = new RenderTarget();
    switch ( textureDescription.description.dimension ) {
    case TextureDescription::DIMENSION_TEXTURE_1D:
        createdTarget->createAsRenderTarget1D( renderDevice, textureDescription.description );
        break;
    case TextureDescription::DIMENSION_TEXTURE_2D:
        createdTarget->createAsRenderTarget2D( renderDevice, textureDescription.description );
        break;
    case TextureDescription::DIMENSION_TEXTURE_3D:
        createdTarget->createAsRenderTarget3D( renderDevice, textureDescription.description );
        break;
    }

    allocatedRenderTargets.push_back( std::unique_ptr<RenderTarget>( createdTarget ) );
    reservedRenderTargetsIndexes.push_back( static_cast<int32_t>( allocatedRenderTargets.size() - 1 ) );
    reservedRenderTargets.insert( std::make_pair( renderTargetHandle, createdTarget ) );
}

void RenderPipelineResources::allocateOrReuseBuffer( RenderDevice* renderDevice, fnPipelineResHandle_t bufferHandle, const BufferDesc& bufferDescription )
{
    for ( auto& availableBuffer : allocatedBuffers ) {
        auto& availableBufferDesc = availableBuffer->getDescription();

        if ( availableBufferDesc.Size == bufferDescription.Size
            && availableBufferDesc.Type == bufferDescription.Type ) {

            // TODO Simplify this crap
            if ( availableBufferDesc.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_1D
                || availableBufferDesc.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D
                || availableBufferDesc.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_3D ) {
                if ( availableBufferDesc.Width != bufferDescription.Width
                    || availableBufferDesc.Height != bufferDescription.Height
                    || availableBufferDesc.ViewFormat != bufferDescription.ViewFormat ) {
                    continue;
                }
            } 

            // NOTE Don't remove the buffer from the allocable array
            reservedBuffers.insert( std::make_pair( bufferHandle, availableBuffer.get() ) );
            return;
        }
    }

    Buffer* createdBuffer = new Buffer();
    createdBuffer->create( renderDevice, bufferDescription );

    allocatedBuffers.push_back( std::unique_ptr<Buffer>( createdBuffer ) );
    reservedBuffers.insert( std::make_pair( bufferHandle, createdBuffer ) );
}

void RenderPipelineResources::allocateOrReuseSampler( RenderDevice* renderDevice, fnPipelineResHandle_t samplerHandle, const SamplerDesc& samplerDescription )
{
    for ( auto& availableSampler : allocatedSamplers ) {
        auto availableBufferDesc = availableSampler->getDescription();

        if ( availableBufferDesc->addressU == samplerDescription.addressU
            && availableBufferDesc->addressV == samplerDescription.addressV
            && availableBufferDesc->addressW == samplerDescription.addressW
            && availableBufferDesc->comparisonFunction == samplerDescription.comparisonFunction
            && availableBufferDesc->filter == samplerDescription.filter
            && availableBufferDesc->maxLOD == samplerDescription.maxLOD
            && availableBufferDesc->minLOD == samplerDescription.minLOD ) {

            // NOTE Don't remove the buffer from the allocable array
            reservedSamplers.insert( std::make_pair( samplerHandle, availableSampler.get() ) );
            return;
        }
    }

    Sampler* createdSampler = new Sampler();
    createdSampler->create( renderDevice, samplerDescription );

    allocatedSamplers.push_back( std::unique_ptr<Sampler>( createdSampler ) );
    reservedSamplers.insert( std::make_pair( samplerHandle, createdSampler ) );
}

fnPipelineMutableResHandle_t RenderPipelineResources::importRenderTarget( RenderTarget* renderTarget )
{
    importedRenderTargets.insert( std::make_pair( --importedResourcesIndexing, renderTarget ) );

    return importedResourcesIndexing;
}

RenderTarget* RenderPipelineResources::retrieveRenderTarget( fnPipelineResHandle_t resourceHandle )
{
    auto it = reservedRenderTargets.find( resourceHandle );
    return it != reservedRenderTargets.end() ? it->second : nullptr;
}

void RenderPipelineResources::reset()
{
    reservedRenderTargets.clear();
    reservedPipelineStates.clear();
    reservedBuffers.clear();
    reservedSamplers.clear();

    for ( auto& importedRenderTarget : importedRenderTargets ) {
        reservedRenderTargets.insert( importedRenderTarget );
    }

    importedRenderTargets.clear();
    reservedRenderTargetsIndexes.clear();
}

void RenderPipelineResources::clearBuckets()
{
    for ( int i = 0; i < 4 * 4; i++ ) {
        layerBucketsCmdCount[i] = 0;
    }
}

void RenderPipelineResources::submitViewport( const Viewport& viewport )
{
    activeViewport = viewport;
    activeViewportGeometry = viewport;

    FLAN_IMPORT_VAR_PTR( SSAAMultiplicator, float )
    activeViewportGeometry.Width *= *SSAAMultiplicator;
    activeViewportGeometry.Height *= *SSAAMultiplicator;
}

const Viewport& RenderPipelineResources::getActiveViewport() const
{
    return activeViewport;
}

const Viewport& RenderPipelineResources::getActiveViewportGeometry() const
{
    return activeViewportGeometry;
}

void RenderPipelineResources::submitCamera( const Camera::Data& cameraData )
{
    activeCamera = cameraData;
}

const Camera::Data& RenderPipelineResources::getActiveCamera() const
{
    return activeCamera;
}

void RenderPipelineResources::submitRenderPassesArgs( const std::map<fnStringHash_t, void*>& args )
{
    renderPassesArgs = args;
}

const void* RenderPipelineResources::getRenderPassArgs( const fnStringHash_t renderPassHashcode ) const
{
    auto it = renderPassesArgs.find( renderPassHashcode );
    return it != renderPassesArgs.end() ? it->second : nullptr;
}

void RenderPipelineResources::addToLayerBucket( const DrawCommandKey::Layer layer, const uint8_t viewportLayer, const DrawCommandInfos& geometryDrawCall )
{
    const auto bucketIndex = ( layer * 4 + viewportLayer );

    auto& bucketCmdCount = layerBucketsCmdCount[bucketIndex];
    layerBuckets[bucketIndex][bucketCmdCount++] = geometryDrawCall;
}

const DrawCommandInfos* RenderPipelineResources::getLayerBucket( const DrawCommandKey::Layer layer, const uint8_t viewportLayer, int& cmdCount ) const
{
    const auto bucketIndex = ( layer * 4 + viewportLayer );

    cmdCount = layerBucketsCmdCount[bucketIndex];
    return layerBuckets[bucketIndex];
}

RasterizerState* RenderPipelineResources::findSuitableRasterizerState( RenderDevice* renderDevice, const RasterizerStateDesc& desc )
{
    for ( auto& rasterizerState : allocatedRasterizerStates ) {
        auto& rasterizerStateDesc = rasterizerState->getDescription();

        if ( rasterizerStateDesc == desc ) {
            return rasterizerState.get();
        }
    }

    RasterizerState* createdRasterizerState = new RasterizerState();
    createdRasterizerState->create( renderDevice, desc );

    allocatedRasterizerStates.push_back( std::unique_ptr<RasterizerState>( createdRasterizerState ) );

    return createdRasterizerState;
}

DepthStencilState* RenderPipelineResources::findSuitableDepthStencilState( RenderDevice* renderDevice, const DepthStencilStateDesc& desc )
{
    for ( auto& depthState : allocatedDepthStencilStates ) {
        auto& depthStateDesc = depthState->getDescription();

        if ( depthStateDesc == desc ) {
            return depthState.get();
        }
    }

    DepthStencilState* createdDepthState = new DepthStencilState();
    createdDepthState->create( renderDevice, desc );

    allocatedDepthStencilStates.push_back( std::unique_ptr<DepthStencilState>( createdDepthState ) );

    return createdDepthState;
}

BlendState* RenderPipelineResources::findSuitableBlendState( RenderDevice* renderDevice, const BlendStateDesc& desc )
{
    for ( auto& blendState : allocatedBlendStates ) {
        auto& blendStateDesc = blendState->getDescription();

        if ( blendStateDesc == desc ) {
            return blendState.get();
        }
    }

    BlendState* createdBlendState = new BlendState();
    createdBlendState->create( renderDevice, desc );

    allocatedBlendStates.push_back( std::unique_ptr<BlendState>( createdBlendState ) );

    return createdBlendState;
}

void RenderPipelineResources::setTimeDelta( const float timeDelta )
{
    rendererTimeDelta = timeDelta;
}

float RenderPipelineResources::getTimeDelta() const
{
    return rendererTimeDelta;
}
