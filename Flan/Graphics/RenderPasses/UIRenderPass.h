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
#include <Shared.h>

#include <glm/gtc/matrix_transform.hpp>

static fnPipelineMutableResHandle_t AddHUDRenderPass( RenderPipeline* renderPipeline )
{
    using namespace flan::rendering;

    struct HUDBufferInfos
    {
        glm::mat4x4 orthoProjMatrix;
        glm::mat4x4 modelMatrix;
    };

    auto RenderPass = renderPipeline->addRenderPass(
        "Primitive 2D Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            // Read Render Target to extract bright parts from
            passData.output[0] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainPresentColorRT" ) );

            // Constant Buffer
            BufferDesc primitiveBuffer = {};
            primitiveBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            primitiveBuffer.Size = sizeof( HUDBufferInfos );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( primitiveBuffer );

            SamplerDesc bilinearSamplerDesc;
            bilinearSamplerDesc.addressU = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = flan::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.samplers[0] = renderPipelineBuilder->allocateSampler( bilinearSamplerDesc );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &ouputRenderTarget );

            // Retrieve Parameters Buffer
            auto modelMatrixBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            modelMatrixBuffer->bind( cmdList, 0, SHADER_STAGE_VERTEX );

            const auto backbufferSize = renderPipelineResources->getActiveViewport();

            HUDBufferInfos bufferInfos;
            bufferInfos.orthoProjMatrix = glm::orthoLH( 0.0f, static_cast<float>( backbufferSize.Width ), static_cast<float>( backbufferSize.Height ), 0.0f, -1.0f, 1.0f );

            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[0] );

            // Material input sampler
            for ( uint32_t i = 0; i < 8; i++ ) {
                bilinearSampler->bind( cmdList, i );
            }

            int cmdCount = 0;
            auto* hudBucketList = renderPipelineResources->getLayerBucket( DrawCommandKey::Layer::LAYER_HUD, DrawCommandKey::HUD_VIEWPORT_LAYER_DEFAULT, cmdCount );

            glm::mat4x4* previousModelMatrix = nullptr;
            for ( int i = 0; i < cmdCount; i++ ) {
                const auto& drawCmd = hudBucketList[i];
                drawCmd.vao->bind( cmdList );

                if ( drawCmd.modelMatrix != previousModelMatrix ) {
                    bufferInfos.modelMatrix = *drawCmd.modelMatrix;
                    modelMatrixBuffer->updateAsynchronous( cmdList, &bufferInfos, sizeof( HUDBufferInfos ) );
                    previousModelMatrix = drawCmd.modelMatrix;
                }

                drawCmd.material->bind( cmdList );

                cmdList->drawIndexedCmd( drawCmd.indiceBufferCount, drawCmd.indiceBufferOffset );
            }
        }
    );

    return RenderPass.output[0];
}

FLAN_REGISTER_RENDERPASS( HUDRenderPass, AddHUDRenderPass )
