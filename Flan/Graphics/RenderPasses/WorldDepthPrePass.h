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

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>
#include <Rendering/PipelineState.h>
#include <Graphics/GraphicsAssetManager.h>
#include <Graphics/RenderPipeline.h>
#include <Framework/Material.h>
#include <Shared.h>

using namespace flan::rendering;

static fnPipelineMutableResHandle_t AddOpaqueZPrePass( RenderPipeline* renderPipeline, const bool enableMSAA = false )
{
    struct MatricesBuffer
    {
        glm::mat4x4 modelMatrix[512];
        glm::mat4 ViewProjectionShadowMatrix;
        float     lodBlendAlpha;
        uint32_t  __PADDING__[3];
    };

    auto RenderPass = renderPipeline->addRenderPass(
        "Opaque Z Pre-pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            auto mainDepthRenderTarget = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainDepthRT" ) );

            if ( mainDepthRenderTarget == -1 ) {
                RenderPassTextureDesc passRenderTargetDesc = {};
                passRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
                passRenderTargetDesc.description.format = IMAGE_FORMAT_R32_TYPELESS;
                passRenderTargetDesc.description.mipCount = 1;
                passRenderTargetDesc.description.arraySize = 1;
                passRenderTargetDesc.description.flags.isDepthResource = 1;

                passRenderTargetDesc.useGlobalMultisamplingState = enableMSAA;
                passRenderTargetDesc.useGlobalDimensions = true;
                passRenderTargetDesc.initialState = RenderPassTextureDesc::CLEAR_DEPTH_REVERSED_Z;

                passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

                renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "MainDepthRT" ), passData.output[0] );
            } else {
                passData.output[0] = mainDepthRenderTarget;
            }

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState;
            passPipelineState.hashcode = FLAN_STRING_HASH( "ZPrepass" );
            passPipelineState.vertexStage = FLAN_STRING( "DepthWrite" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            passPipelineState.rasterizerState.cullMode = flan::rendering::eCullMode::CULL_MODE_FRONT;
            passPipelineState.rasterizerState.useTriangleCCW = true;
            passPipelineState.depthStencilState.depthComparisonFunc = flan::rendering::eComparisonFunction::COMPARISON_FUNCTION_GREATER;
            passPipelineState.rasterizerState.fillMode = flan::rendering::eFillMode::FILL_MODE_SOLID;
            passPipelineState.depthStencilState.enableDepthWrite = true;
            passPipelineState.depthStencilState.enableDepthTest = true;
            passPipelineState.depthStencilState.enableStencilTest = false;

            passPipelineState.inputLayout = {
                { 0, IMAGE_FORMAT_R32G32B32_FLOAT, 0, 0, 0, false, "POSITION" },
                { 0, IMAGE_FORMAT_R32G32B32_FLOAT, 0, 0, 0, true, "NORMAL" },
                { 0, IMAGE_FORMAT_R32G32_FLOAT, 0, 0, 0, true, "TEXCOORD" },
                { 0, IMAGE_FORMAT_R32G32B32_FLOAT, 0, 0, 0, true, "TANGENT" },
                { 0, IMAGE_FORMAT_R32G32B32_FLOAT, 0, 0, 0, true, "BINORMAL" },
            };

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            // Constant Buffer
            BufferDesc passBuffer = {};
            passBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            passBuffer.Size = sizeof( MatricesBuffer );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( passBuffer );

            BufferDesc cameraBuffer = {};
            cameraBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            cameraBuffer.Size = sizeof( Camera::Data );

            passData.buffers[3] = renderPipelineBuilder->allocateBuffer( cameraBuffer );

            BufferDesc terrainStreamingBuffer = {};
            terrainStreamingBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            terrainStreamingBuffer.Size = sizeof( TerrainStreaming::terrainMaterialStreaming );

            passData.buffers[5] = renderPipelineBuilder->allocateBuffer( terrainStreamingBuffer );

            // Texture (geometry stuff) Sampler
            SamplerDesc matSamplerDesc;
            matSamplerDesc.addressU = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            matSamplerDesc.addressV = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            matSamplerDesc.addressW = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            matSamplerDesc.filter = eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.samplers[0] = renderPipelineBuilder->allocateSampler( matSamplerDesc );

            SamplerDesc matDisplacementSamplerDesc;
            matDisplacementSamplerDesc.addressU = eSamplerAddress::SAMPLER_ADDRESS_WRAP;
            matDisplacementSamplerDesc.addressV = eSamplerAddress::SAMPLER_ADDRESS_WRAP;
            matDisplacementSamplerDesc.addressW = eSamplerAddress::SAMPLER_ADDRESS_WRAP;
            matDisplacementSamplerDesc.filter = eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.samplers[1] = renderPipelineBuilder->allocateSampler( matDisplacementSamplerDesc );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Update viewport
            auto& pipelineDimensions = renderPipelineResources->getActiveViewportGeometry();
            cmdList->setViewportCmd( pipelineDimensions );

            // Bind Render Target
            auto depthRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( nullptr, depthRenderTarget, 0 );

            // Get Constant Buffer 
            auto matricesConstantBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            matricesConstantBuffer->bind( cmdList, CBUFFER_INDEX_MATRICES );

            auto& cameraData = renderPipelineResources->getActiveCamera();

            // Bind Camera Buffer
            auto cameraCbuffer = renderPipelineResources->getBuffer( passData.buffers[3] );
            cameraCbuffer->updateAsynchronous( cmdList, &cameraData, sizeof( Camera::Data ) );
            cameraCbuffer->bind( cmdList, 0 );

            MatricesBuffer matrices;
            matrices.ViewProjectionShadowMatrix = cameraData.viewProjectionMatrix;

            matricesConstantBuffer->updateAsynchronous( cmdList, &matrices, sizeof( MatricesBuffer ) );

            auto matInputSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            matInputSampler->bind( cmdList, 8 );

            auto matInputDisplSampler = renderPipelineResources->getSampler( passData.samplers[1] );
            matInputDisplSampler->bind( cmdList, 9 );
            auto terrrainStreamingData = renderPipelineResources->getWellKnownImportedResource<TerrainStreaming>();
            terrrainStreamingData->baseColorStreamed->bind( cmdList, 6, SHADER_STAGE_TESSELATION_CONTROL | SHADER_STAGE_TESSELATION_EVALUATION );

            auto terrainStreamingBuffer = renderPipelineResources->getBuffer( passData.buffers[5] );
            terrainStreamingBuffer->updateAsynchronous( cmdList, &terrrainStreamingData->terrainMaterialStreaming, sizeof( TerrainStreaming::terrainMaterialStreaming ) );
            terrainStreamingBuffer->bind( cmdList, 7, SHADER_STAGE_PIXEL | SHADER_STAGE_TESSELATION_CONTROL | SHADER_STAGE_TESSELATION_EVALUATION );

            // Render Opaque Geometry Only
            int cmdCount = 0;
            auto* opaqueBucketList = renderPipelineResources->getLayerBucket( DrawCommandKey::Layer::LAYER_DEPTH, DrawCommandKey::DEPTH_VIEWPORT_LAYER_DEFAULT, cmdCount );

            glm::mat4x4* previousModelMatrix = nullptr;
            for ( int i = 0; i < cmdCount; i++ ) {
                const auto& drawCmd = opaqueBucketList[i];
                drawCmd.vao->bind( cmdList );

                if ( drawCmd.instanceCount > 1 ) {
                    memcpy( matrices.modelMatrix, drawCmd.modelMatrix, drawCmd.instanceCount * sizeof( glm::mat4x4 ) );
                    previousModelMatrix = nullptr;
                } else {
                    if ( drawCmd.modelMatrix != previousModelMatrix ) {
                        matrices.modelMatrix[0] = *drawCmd.modelMatrix;
                        previousModelMatrix = drawCmd.modelMatrix;
                    }
                }

                matrices.lodBlendAlpha = drawCmd.alphaDitheringValue;
                matricesConstantBuffer->updateAsynchronous( cmdList, &matrices, sizeof( MatricesBuffer ) );

                if ( drawCmd.instanceCount > 1 ) {
                    if ( !drawCmd.material->bindInstancedReversedDepthOnly( cmdList ) ) {
                        continue;
                    }

                    cmdList->drawInstancedIndexedCmd( drawCmd.indiceBufferCount, drawCmd.indiceBufferOffset, drawCmd.instanceCount );
                } else {
                    if ( !drawCmd.material->bindReversedDepthOnly( cmdList ) ) {
                        continue;
                    }

                    cmdList->drawIndexedCmd( drawCmd.indiceBufferCount, drawCmd.indiceBufferOffset );
                }
            }

            // Unbind everything for extra safety
            cmdList->bindBackbufferCmd();
        }
    );

    return RenderPass.output[0];
}

FLAN_REGISTER_RENDERPASS( WorldDepthPass, AddOpaqueZPrePass )
FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( WorldDepthMSAAPass, [=]( RenderPipeline* renderPipeline ) { return AddOpaqueZPrePass( renderPipeline, true ); } )
