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
#include "Shared.h"
#include "GrassRenderingModule.h"

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>
#include <Rendering/Texture.h>
#include <Rendering/VertexArrayObject.h>

#include <Graphics/CBufferIndexes.h>
#include <Graphics/GraphicsAssetManager.h>
#include <Framework/Material.h>

#include <Core/Factory.h>

#include <glm/gtc/matrix_transform.hpp>

GrassRenderingModule::GrassRenderingModule()
    : textureAllocator( nullptr )
    , grassMapTexture( nullptr )
    , randomnessTexture( nullptr )
    , topDownRenderTarget( nullptr )
{

}

GrassRenderingModule::~GrassRenderingModule()
{
    flan::core::free( textureAllocator, randomnessTexture );
    flan::core::free( textureAllocator, topDownRenderTarget );

    grassMapTexture = nullptr;
}

void GrassRenderingModule::create( RenderDevice* renderDevice, BaseAllocator* allocator )
{
    textureAllocator = allocator;

    float randomValues[64 * 64 * 4] = { 0.0f };
    for ( int i = 0; i < 64 * 64 * 4; i++ ) {
        randomValues[i] = static_cast< float >( rand() % 255 ) / 255.0f;
    }

    TextureDescription description;
    description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    description.arraySize = 1;
    description.depth = 1;
    description.format = IMAGE_FORMAT_R32G32B32A32_FLOAT;
    description.height = 64;
    description.width = 64;
    description.mipCount = 1;
    description.samplerCount = 1;

    randomnessTexture = flan::core::allocate<Texture>( allocator );
    randomnessTexture->createAsTexture2D( renderDevice, description, randomValues, sizeof( float ) * 64 * 64 * 4 );
    randomnessTexture->setResourceName( renderDevice, "Grass Random Noise" );


    // Allocate top down rendertarget (since the resource is permanent and shared between viewports in a same world)
    TextureDescription topDownDesc;
    topDownDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    topDownDesc.arraySize = 1;
    topDownDesc.depth = 1;
    topDownDesc.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
    topDownDesc.width = 1024.0f;
    topDownDesc.height = 1024.0f;
    topDownDesc.mipCount = 1;
    topDownDesc.samplerCount = 1;

    topDownRenderTarget = flan::core::allocate<RenderTarget>( allocator );
    topDownRenderTarget->createAsRenderTarget2D( renderDevice, topDownDesc );
}

void GrassRenderingModule::loadCachedResources( RenderDevice* renderDevice, GraphicsAssetManager* graphicsAssetManager )
{
    grassMapTexture = graphicsAssetManager->getTexture( FLAN_STRING( "GameData/Textures/grassmap_test.dds" ) );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent( FLAN_STRING_HASH( "TopDownWorldCapture" ),
        [=]( RenderPipeline* renderPipeline ) {
            return addTopDownTerrainCapturePass( renderPipeline );
        } );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent( FLAN_STRING_HASH( "GrassGeneration" ),
        [=]( RenderPipeline* renderPipeline ) {
        return addGrassGenerationPass( renderPipeline );
    } );
    
}

fnPipelineMutableResHandle_t GrassRenderingModule::addTopDownTerrainCapturePass( RenderPipeline* renderPipeline )
{
    struct InstanceBuffer
    {
        glm::mat4x4 modelMatrix[512];
        float lodDitheringAlpha;
        uint32_t __PADDING__[3];
    };

    auto topDownRTResource = renderPipeline->importRenderTarget( topDownRenderTarget );

    auto data = renderPipeline->addRenderPass(
        "Terrain Top Down Capture Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            passData.output[0] = renderPipelineBuilder->readRenderTarget( topDownRTResource );

            BufferDesc cameraBuffer = {};
            cameraBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            cameraBuffer.Size = sizeof( Camera::Data );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( cameraBuffer );

            BufferDesc passBuffer = {};
            passBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            passBuffer.Size = sizeof( InstanceBuffer );

            passData.buffers[1] = renderPipelineBuilder->allocateBuffer( passBuffer );

            using namespace flan::rendering;
            SamplerDesc matDisplacementSamplerDesc;
            matDisplacementSamplerDesc.addressU = eSamplerAddress::SAMPLER_ADDRESS_WRAP;
            matDisplacementSamplerDesc.addressV = eSamplerAddress::SAMPLER_ADDRESS_WRAP;
            matDisplacementSamplerDesc.addressW = eSamplerAddress::SAMPLER_ADDRESS_WRAP;
            matDisplacementSamplerDesc.filter = eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.samplers[0] = renderPipelineBuilder->allocateSampler( matDisplacementSamplerDesc );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            auto viewport = cmdList->getViewportCmd();
            cmdList->setViewportCmd( renderPipelineResources->getActiveViewport() );

            const Camera::Data& cameraData = renderPipelineResources->getActiveCamera();

            // Retrieve and update constant buffer
            auto constantBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            constantBuffer->updateAsynchronous( cmdList, &cameraData, sizeof( Camera::Data ) );
            constantBuffer->bind( cmdList, 0, SHADER_STAGE_ALL );

            auto modelMatrixBuffer = renderPipelineResources->getBuffer( passData.buffers[1] );
            modelMatrixBuffer->bind( cmdList, CBUFFER_INDEX_MATRICES, SHADER_STAGE_ALL );

            // Set Ouput Target
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &ouputRenderTarget );

            auto hmapSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            hmapSampler->bind( cmdList, 8 );

            // Render opaque geometry
            int cmdCount = 0;
            auto* opaqueBucketList = renderPipelineResources->getLayerBucket( DrawCommandKey::Layer::LAYER_WORLD, DrawCommandKey::WORLD_VIEWPORT_LAYER_DEFAULT, cmdCount );

            InstanceBuffer instance;
            glm::mat4x4* previousModelMatrix = nullptr;
            for ( int i = 0; i < cmdCount; i++ ) {
                const auto& drawCmd = opaqueBucketList[i];
                drawCmd.vao->bind( cmdList );

                if ( drawCmd.instanceCount > 1 ) {
                    memcpy( instance.modelMatrix, drawCmd.modelMatrix, drawCmd.instanceCount * sizeof( glm::mat4x4 ) );
                    previousModelMatrix = nullptr;
                } else {
                    if ( drawCmd.modelMatrix != previousModelMatrix ) {
                        instance.modelMatrix[0] = *drawCmd.modelMatrix;
                        previousModelMatrix = drawCmd.modelMatrix;
                    }
                }

                instance.lodDitheringAlpha = drawCmd.alphaDitheringValue;
                modelMatrixBuffer->updateAsynchronous( cmdList, &instance, sizeof( InstanceBuffer ) );

            
                if ( drawCmd.instanceCount > 1 ) {
                    drawCmd.material->bindInstanced( cmdList );

                    cmdList->drawInstancedIndexedCmd( drawCmd.indiceBufferCount, drawCmd.indiceBufferOffset, drawCmd.instanceCount );
                } else {
                    drawCmd.material->bindTopDown( cmdList );
                    
                    cmdList->drawIndexedCmd( drawCmd.indiceBufferCount, drawCmd.indiceBufferOffset );
                }
            }
                
            cmdList->bindBackbufferCmd();
            cmdList->setViewportCmd( viewport );
            constantBuffer->unbind( cmdList );
            ouputRenderTarget->unbind( cmdList );
        } );

    return data.output[0];
}

fnPipelineMutableResHandle_t GrassRenderingModule::addGrassGenerationPass( RenderPipeline* renderPipeline )
{
    struct PerPassBuffer
    {
        glm::vec4   mainCameraFrustumPlanes[6];
        glm::vec3   mainCameraPositionWorldSpace;
        float       topDownMapSize;
        glm::vec2   terrainOriginWorldSpace;
        float       grassMapSize;
        uint32_t    __PADDING__;
    };

    // For reference only, everything should be GPU sided
    struct Instance
    {
        glm::vec3   position;
        float       specular;
        glm::vec3   albedo;
        uint32_t    vertexOffsetAndSkew;
        glm::vec2   rotation;
        glm::vec2   scale;
    };
    FLAN_IS_MEMORY_ALIGNED( 16, Instance );

    auto topDownRTResource = renderPipeline->importRenderTarget( topDownRenderTarget );
    auto data = renderPipeline->addRenderPass(
        "Grass Generation Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "GrassGenerationPass" );
            passPipelineState.computeStage = FLAN_STRING( "GrassGeneration" );
            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            BufferDesc grassAppendBuffer;
            grassAppendBuffer.Type = BufferDesc::APPEND_STRUCTURED_BUFFER;
            grassAppendBuffer.Size = sizeof( Instance ); // 1 grass blade per pixel as max density
            grassAppendBuffer.Stride = ( 512 * 512 );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( grassAppendBuffer );

            BufferDesc perPassBuffer;
            perPassBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            perPassBuffer.Size = sizeof( PerPassBuffer );

            passData.buffers[1] = renderPipelineBuilder->allocateBuffer( perPassBuffer );

            // Retrieve current frame top down render target
            passData.input[0] = renderPipelineBuilder->readRenderTarget( topDownRTResource );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Bind Resources
            grassMapTexture->bind( cmdList, 0, SHADER_STAGE_COMPUTE );

            // TODO Top Down texture might be null during the first frames (which kinda suck tbh)
            auto topDownTexture = renderPipelineResources->getRenderTarget( passData.input[0] );
            if ( topDownTexture != nullptr ) {
                topDownTexture->bind( cmdList, 1, SHADER_STAGE_COMPUTE );
            }

            randomnessTexture->bind( cmdList, 2, SHADER_STAGE_COMPUTE );

            // Bind append buffer
            auto grassAppendBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            grassAppendBuffer->bind( cmdList, 0, SHADER_STAGE_COMPUTE );

            const auto& camera = renderPipelineResources->getActiveCamera();

            // Bind cbuffer
            PerPassBuffer perPassBuffer;
            perPassBuffer.grassMapSize = 2048.0f;
            perPassBuffer.topDownMapSize = 1024.0f;
            perPassBuffer.terrainOriginWorldSpace = glm::vec2( 0.0f, 0.0f );
            perPassBuffer.mainCameraPositionWorldSpace = camera.worldPosition;
            memcpy( perPassBuffer.mainCameraFrustumPlanes, camera.frustum.planes, 6 * sizeof( glm::vec4 ) );

            auto passConstantBuffer = renderPipelineResources->getBuffer( passData.buffers[1] );
            passConstantBuffer->updateAsynchronous( cmdList, &perPassBuffer, sizeof( PerPassBuffer ) );
            passConstantBuffer->bindReadOnly( cmdList, 0, SHADER_STAGE_COMPUTE );

            // Start GPU Compute
            cmdList->dispatchComputeCmd( 1024.0f / 32, 1024.0f / 32, 1 );
        } 
    );

    return data.buffers[0];
}
