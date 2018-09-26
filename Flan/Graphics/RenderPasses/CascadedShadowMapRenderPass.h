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
#include <Rendering/PipelineState.h>
#include <Graphics/GraphicsAssetManager.h>
#include <Graphics/RenderPipeline.h>
#include <Graphics/CBufferIndexes.h>
#include <Graphics/TextureSlotIndexes.h>
#include <Shared.h>

#include <Shaders/ShadowMappingShared.h>

static fnPipelineMutableResHandle_t AddCascadedShadowMapCapturePass( RenderPipeline* renderPipeline )
{
    struct MatricesBuffer
    {
        glm::mat4 ModelMatrix;
        glm::mat4 ViewProjectionShadowMatrix;
    };

    auto RenderPass = renderPipeline->addRenderPass(
        "Cascaded Shadow Map Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            // TODO Allocate from a shadow atlas instead of using multiple render target
            RenderPassTextureDesc passRenderTargetDesc = {};
            passRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            passRenderTargetDesc.description.format = IMAGE_FORMAT_R32_TYPELESS;
            passRenderTargetDesc.description.samplerCount = 1;
            passRenderTargetDesc.description.mipCount = 1;
            passRenderTargetDesc.description.arraySize = 1;
            passRenderTargetDesc.description.flags.isDepthResource = 1;
            passRenderTargetDesc.description.width = CSM_SHADOW_MAP_DIMENSIONS * CSM_SLICE_COUNT;
            passRenderTargetDesc.description.height = CSM_SHADOW_MAP_DIMENSIONS;
            passRenderTargetDesc.initialState = RenderPassTextureDesc::CLEAR;

            // Allocate Pass Render Target
            passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );
            renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "CascadedShadowMappingShadowMap" ), passData.output[0] );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "Shadow Map Rendering" );
            passPipelineState.vertexStage = FLAN_STRING( "DepthWrite" );
            passPipelineState.rasterizerState.fillMode = flan::rendering::eFillMode::FILL_MODE_SOLID;
            passPipelineState.rasterizerState.cullMode = flan::rendering::eCullMode::CULL_MODE_NONE;
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            passPipelineState.depthStencilState.depthComparisonFunc = flan::rendering::eComparisonFunction::COMPARISON_FUNCTION_LEQUAL;
            passPipelineState.depthStencilState.enableDepthWrite = true;
            passPipelineState.depthStencilState.enableStencilTest = false;
            passPipelineState.depthStencilState.enableDepthTest = true;
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
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Bind Render Target
            auto depthRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );

            cmdList->bindRenderTargetsCmd( nullptr, depthRenderTarget, 0 );

            // Get Constant Buffer 
            auto matricesConstantBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            matricesConstantBuffer->bind( cmdList, CBUFFER_INDEX_MATRICES, SHADER_STAGE_VERTEX );

            MatricesBuffer matrices;
            matrices.ModelMatrix = glm::mat4( 1 );

            const auto backbufferViewport = cmdList->getViewportCmd();
            auto& cameraData = renderPipelineResources->getActiveCamera();

            for ( int i = 0; i < CSM_SLICE_COUNT; i++ ) {
                cmdList->setViewportCmd( CSM_SHADOW_MAP_DIMENSIONS, CSM_SHADOW_MAP_DIMENSIONS, CSM_SHADOW_MAP_DIMENSIONS * i );
                matrices.ViewProjectionShadowMatrix = cameraData.shadowViewMatrix[i];

                int cmdCount = 0;
                auto* sliceDrawCmds = renderPipelineResources->getLayerBucket( DrawCommandKey::Layer::LAYER_DEPTH, DrawCommandKey::DEPTH_VIEWPORT_LAYER_CSM0 + i, cmdCount );
                glm::mat4x4* previousModelMatrix = nullptr;
                for ( int i = 0; i < cmdCount; i++ ) {
                    const auto& drawCmd = sliceDrawCmds[i];
                    drawCmd.vao->bind( cmdList );

                    if ( previousModelMatrix != drawCmd.modelMatrix ) {
                        matrices.ModelMatrix = *drawCmd.modelMatrix;
                        matricesConstantBuffer->updateAsynchronous( cmdList, &matrices, sizeof( MatricesBuffer ) );

                        previousModelMatrix = drawCmd.modelMatrix;
                    }

                    drawCmd.material->bindDepthOnly( cmdList );

                    cmdList->drawIndexedCmd( drawCmd.indiceBufferCount, drawCmd.indiceBufferOffset );
                }
            }

            // Unbind everything for extra safety
            cmdList->bindBackbufferCmd();
            cmdList->setViewportCmd( backbufferViewport );
        }
    );

    return RenderPass.output[0];
}

FLAN_REGISTER_RENDERPASS( CascadedShadowMapCapture, AddCascadedShadowMapCapturePass )
