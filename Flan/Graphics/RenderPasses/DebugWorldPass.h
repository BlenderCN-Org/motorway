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
#include <Graphics/TextureSlotIndexes.h>
#include <Graphics/CBufferIndexes.h>
#include <Shaders/Shared.h>
#include <Shared.h>

#include <Graphics/DrawCommand.h>
#include <Framework/Material.h>

static fnPipelineMutableResHandle_t AddDebugWorldPass( RenderPipeline* renderPipeline, const bool enableMSAA = false )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "World Debug Draw Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            // Color RT
            auto mainRenderTarget = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) );
            if ( mainRenderTarget == -1 ) {
                RenderPassTextureDesc passRenderTargetDesc = {};
                passRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
                passRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
                passRenderTargetDesc.description.depth = 1;
                passRenderTargetDesc.description.mipCount = 1;
                passRenderTargetDesc.description.arraySize = 1;
                passRenderTargetDesc.useGlobalDimensions = true;
                passRenderTargetDesc.useGlobalMultisamplingState = enableMSAA;
                passRenderTargetDesc.initialState = RenderPassTextureDesc::CLEAR;

                passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

                renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ), passData.output[0] );
            } else {
                passData.output[0] = mainRenderTarget;
            }

            // Read Depth Buffer
            passData.input[0] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainDepthRT" ) );

            // Constant Buffer
            BufferDesc passBuffer = {};
            passBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            passBuffer.Size = sizeof( glm::mat4 );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( passBuffer );

            BufferDesc cameraBuffer = {};
            cameraBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            cameraBuffer.Size = sizeof( Camera::Data );

            passData.buffers[1] = renderPipelineBuilder->allocateBuffer( cameraBuffer );

        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Output Buffers
            auto colorBuffer = renderPipelineResources->getRenderTarget( passData.output[0] );

            auto depthBuffer = renderPipelineResources->getRenderTarget( passData.input[0] );

            cmdList->bindRenderTargetsCmd( &colorBuffer, depthBuffer );

            // Get Constant Buffer 
            auto modelMatrixBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            modelMatrixBuffer->bind( cmdList, CBUFFER_INDEX_MATRICES, SHADER_STAGE_VERTEX );

            // Bind Camera Buffer
            auto cameraCbuffer = renderPipelineResources->getBuffer( passData.buffers[1] );
            auto passCamera = renderPipelineResources->getActiveCamera();
            cameraCbuffer->updateAsynchronous( cmdList, &passCamera, sizeof( Camera::Data ) );
            cameraCbuffer->bind( cmdList, 2 );

            auto& pipelineDimensions = renderPipelineResources->getActiveViewportGeometry();
            cmdList->setViewportCmd( pipelineDimensions );

            // Render opaque geometry
            int cmdCount = 0;
            auto* debugBucketList = renderPipelineResources->getLayerBucket( DrawCommandKey::Layer::LAYER_DEBUG, DrawCommandKey::DEBUG_VIEWPORT_LAYER_DEFAULT, cmdCount );

            glm::mat4x4* previousModelMatrix = nullptr;
            for ( int i = 0; i < cmdCount; i++ ) {
                const auto& drawCmd = debugBucketList[i];
                drawCmd.vao->bind( cmdList );

                if ( drawCmd.modelMatrix != previousModelMatrix ) {
                    modelMatrixBuffer->updateAsynchronous( cmdList, drawCmd.modelMatrix, sizeof( glm::mat4x4 ) );
                    previousModelMatrix = drawCmd.modelMatrix;
                }

                drawCmd.material->bind( cmdList );

                cmdList->drawIndexedCmd( drawCmd.indiceBufferCount, drawCmd.indiceBufferOffset );
            }

            cmdList->bindBackbufferCmd();
        }
    );

    return -1;
}

FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( DebugWorldPass, [=]( RenderPipeline* renderPipeline ) { return AddDebugWorldPass( renderPipeline, false ); } )
FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( DebugWorldMSAAPass, [=]( RenderPipeline* renderPipeline ) { return AddDebugWorldPass( renderPipeline, true ); } )
