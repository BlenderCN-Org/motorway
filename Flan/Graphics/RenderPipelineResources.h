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

#include <Rendering/BlendState.h>
#include "RenderPass.h"

#include <vector>
#include <unordered_map>

#include <Rendering/Texture.h>
#include <Rendering/Buffer.h>
#include <Rendering/Sampler.h>

#include "DrawCommand.h"

class RasterizerState;
class DepthStencilState;
class PipelineState;
class RenderTarget;
class Buffer;
class ShaderStageManager;
struct SamplerDesc;

class RenderPipelineResources
{
public:

public:
                                            RenderPipelineResources();
                                            RenderPipelineResources( RenderPipelineResources& ) = delete;
                                            ~RenderPipelineResources();

    void                                    destroy( RenderDevice* renderDevice );
    PipelineState*                          getPipelineState( const fnPipelineResHandle_t pipelineStateHashcode ) const;
    RenderTarget*                           getRenderTarget( const fnPipelineMutableResHandle_t renderTargetHandle ) const;
    Buffer*                                 getBuffer( const fnPipelineMutableResHandle_t bufferHandle ) const;
    Sampler*                                getSampler( const fnPipelineMutableResHandle_t bufferHandle ) const;

    void                                    allocateOrReusePipelineState( RenderDevice* renderDevice, ShaderStageManager* shaderStageManager, fnPipelineResHandle_t pipelineStateHandle, const RenderPassPipelineStateDesc& pipelineStateDesc );
    void                                    allocateOrReuseRenderTarget( RenderDevice* renderDevice,  CommandList* cmdList, fnPipelineResHandle_t renderTargetHandle, const RenderPassTextureDesc& textureDescription );
    void                                    allocateOrReuseBuffer( RenderDevice* renderDevice, fnPipelineResHandle_t bufferHandle, const BufferDesc& bufferDescription );
    void                                    allocateOrReuseSampler( RenderDevice* renderDevice, fnPipelineResHandle_t samplerHandle, const SamplerDesc& samplerDescription );

    template<typename T>
    void importWellKnownResource( T* resource )
    {
        static_assert( std::is_pod<T>(), "Only POD resources are allowed" );

        importedResources.insert( std::make_pair( FLAN_STRING_HASH( typeid( T ).name() ), resource ) );
    }
    
    template<typename T>
    T* getWellKnownImportedResource() const
    {
        auto it = importedResources.find( FLAN_STRING_HASH( typeid( T ).name() ) );
        return ( it != importedResources.end() ) ? (T*)it->second : nullptr;
    }

    fnPipelineMutableResHandle_t            importRenderTarget( RenderTarget* renderTarget );
    RenderTarget*                           retrieveRenderTarget( fnPipelineResHandle_t resourceHandle );

    void                                    reset();
    void                                    clearBuckets();

    void                                    submitViewport( const Viewport& viewport );
    const Viewport&                         getActiveViewport() const;
    const Viewport&                         getActiveViewportGeometry() const;

    void                                    submitCamera( const Camera::Data& cameraData );
    const Camera::Data&                     getActiveCamera() const;

    void                                    submitRenderPassesArgs( const std::map<fnStringHash_t, void*>& args );
    const void*                             getRenderPassArgs( const fnStringHash_t renderPassHashcode ) const;

    void                                    addToLayerBucket( const DrawCommandKey::Layer layer, const uint8_t viewportLayer, const DrawCommandInfos& geometryDrawCall );
    const DrawCommandInfos*                 getLayerBucket( const DrawCommandKey::Layer layer, const uint8_t viewportLayer, int& cmdCount ) const;

    void                                    setTimeDelta( const float timeDelta );
    float                                   getTimeDelta() const;

private:
    std::vector<std::unique_ptr<RenderTarget>>                  allocatedRenderTargets;
    std::vector<std::unique_ptr<Buffer>>                        allocatedBuffers;
    std::map<fnStringHash_t, std::unique_ptr<PipelineState>>    allocatedPipelineStates;
    std::vector<std::unique_ptr<RasterizerState>>               allocatedRasterizerStates;
    std::vector<std::unique_ptr<DepthStencilState>>             allocatedDepthStencilStates;
    std::vector<std::unique_ptr<BlendState>>                    allocatedBlendStates;
    std::vector<std::unique_ptr<Sampler>>                       allocatedSamplers;

    fnPipelineResHandle_t                                       importedResourcesIndexing;

    std::vector<int32_t>                                        reservedRenderTargetsIndexes;
    std::map<fnStringHash_t, void*>                             renderPassesArgs;

    std::unordered_map<fnPipelineResHandle_t, RenderTarget*>    reservedRenderTargets;
    std::unordered_map<fnPipelineResHandle_t, RenderTarget*>    importedRenderTargets;
    std::unordered_map<fnPipelineResHandle_t, PipelineState*>   reservedPipelineStates;
    std::unordered_map<fnPipelineResHandle_t, Buffer*>          reservedBuffers;
    std::unordered_map<fnPipelineResHandle_t, Sampler*>         reservedSamplers;
    std::unordered_map<fnStringHash_t, void*>                   importedResources;

    float rendererTimeDelta;

    DrawCommandInfos layerBuckets[4 * 4][1024];
    int layerBucketsCmdCount[4 * 4];

    Viewport activeViewport;
    Viewport activeViewportGeometry;

    Camera::Data activeCamera;

private:
    RasterizerState* findSuitableRasterizerState( RenderDevice* renderDevice, const RasterizerStateDesc& desc );
    DepthStencilState* findSuitableDepthStencilState( RenderDevice* renderDevice, const DepthStencilStateDesc& desc );
    BlendState* findSuitableBlendState( RenderDevice* renderDevice, const BlendStateDesc& desc );
};
