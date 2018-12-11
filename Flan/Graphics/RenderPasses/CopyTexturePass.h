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

#include <Graphics/TextureSaveTools.h>

static fnPipelineMutableResHandle_t AddCopyTexturePass( RenderPipeline* renderPipeline, const bool enableMSAA = false, const uint32_t mipLevel = 0, const uint32_t layer = 0, fnPipelineMutableResHandle_t output = -1,  fnPipelineMutableResHandle_t input = -1 )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Copy Texture",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            passData.input[0] = ( input == -1 ) ? renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) )
                                                 : renderPipelineBuilder->readRenderTarget( input );

            if ( output == -1 ) {
                RenderPassTextureDesc passRenderTargetDesc = {};
                passRenderTargetDesc.resourceToCopy = passData.input[0];
                passRenderTargetDesc.copyResource = true;
                passRenderTargetDesc.initialState = RenderPassTextureDesc::CLEAR;

                passRenderTargetDesc.useGlobalMultisamplingState = enableMSAA;

                passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );
                renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ), passData.output[0] );
            } else {
                passData.output[0] = renderPipelineBuilder->readRenderTarget( output );
            }

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "Copy Texture Pass" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTriangle" );
            passPipelineState.pixelStage = FLAN_STRING( "CopyTexture" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            // Sampler State
            SamplerDesc bilinearSamplerDesc;
            bilinearSamplerDesc.addressU = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = flan::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.samplers[0] = renderPipelineBuilder->allocateSampler( bilinearSamplerDesc );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Set Ouput Target
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );

            const auto& outputRTDesc = ouputRenderTarget->getDescription();

            const bool isLayered = ( layer != 0 || outputRTDesc.flags.isCubeMap );

            if ( !isLayered )
                cmdList->bindRenderTargetsCmd( &ouputRenderTarget, nullptr, 1, mipLevel );
            else
                cmdList->bindRenderTargetsLayeredCmd( &ouputRenderTarget, nullptr, 1, mipLevel, layer );

            // Bind Input Target
            auto inputRenderTarget = renderPipelineResources->getRenderTarget( passData.input[0] );
            inputRenderTarget->bind( cmdList, TEXTURE_SLOT_INDEX_LINEAR_SAMPLED, SHADER_STAGE_PIXEL );

            // Bind Sampler
            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            bilinearSampler->bind( cmdList, 2 );
            bilinearSampler->bind( cmdList, 3 );

            // Update viewport
            auto& pipelineDimensions = renderPipelineResources->getActiveViewportGeometry();
            cmdList->setViewportCmd( pipelineDimensions );

            // Bind Dummy Vertex Array
            cmdList->unbindVertexArrayCmd();

            cmdList->drawCmd( 3 );
            inputRenderTarget->unbind( cmdList );

            cmdList->bindBackbufferCmd();
        }
    );

    return RenderPass.output[0];
}

static fnPipelineMutableResHandle_t AddCopyDepthTexturePass( RenderPipeline* renderPipeline, const bool enableMSAA = false, const uint32_t mipLevel = 0, const uint32_t layer = 0, fnPipelineMutableResHandle_t output = -1,  fnPipelineMutableResHandle_t input = -1 )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Copy Depth Texture",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            passData.input[0] = ( input == -1 ) ? renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainDepthRT" ) )
                                                 : renderPipelineBuilder->readRenderTarget( input );

            if ( output == -1 ) {
                RenderPassTextureDesc passRenderTargetDesc = {};
                passRenderTargetDesc.resourceToCopy = passData.input[0];
                passRenderTargetDesc.copyResource = true;
                passRenderTargetDesc.description.flags.isDepthResource = 1;

                passRenderTargetDesc.useGlobalMultisamplingState = enableMSAA;

                passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );
            } else {
                passData.output[0] = renderPipelineBuilder->readRenderTarget( output );
            }
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            auto inputRenderTarget = renderPipelineResources->getRenderTarget( passData.input[0] );

            inputRenderTarget->copyTo( cmdList, ouputRenderTarget );
        }
    );

    return RenderPass.output[0];
}

static fnPipelineMutableResHandle_t AddCopyTextureUAVPass( RenderPipeline* renderPipeline, const bool enableMSAA = false, const uint32_t mipLevel = 0, const uint32_t layer = 0, fnPipelineMutableResHandle_t output = -1, fnPipelineMutableResHandle_t input = -1 )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Copy Texture (UAV)",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            passData.input[0] = ( input == -1 ) ? renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) )
                : renderPipelineBuilder->readRenderTarget( input );

            if ( output == -1 ) {
                BufferDesc passRenderTargetDesc = {};
                passRenderTargetDesc.Type = BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D;
                passData.output[0] = renderPipelineBuilder->allocateBuffer( passRenderTargetDesc );
            } else {
                passData.output[0] = renderPipelineBuilder->readRenderTarget( output );
            }

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "Copy Texture Pass" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTriangle" );
            passPipelineState.pixelStage = FLAN_STRING( "CopyTexture" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            // Sampler State
            SamplerDesc bilinearSamplerDesc;
            bilinearSamplerDesc.addressU = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = flan::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.samplers[0] = renderPipelineBuilder->allocateSampler( bilinearSamplerDesc );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Set Ouput Target
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );

            const auto& outputRTDesc = ouputRenderTarget->getDescription();

            const bool isLayered = ( layer != 0 || outputRTDesc.flags.isCubeMap );

            if ( !isLayered )
                cmdList->bindRenderTargetsCmd( &ouputRenderTarget, nullptr, 1, mipLevel );
            else
                cmdList->bindRenderTargetsLayeredCmd( &ouputRenderTarget, nullptr, 1, mipLevel, layer );

            // Bind Input Target
            auto inputRenderTarget = renderPipelineResources->getBuffer( passData.input[0] );
            inputRenderTarget->bindReadOnly( cmdList, TEXTURE_SLOT_INDEX_LINEAR_SAMPLED, SHADER_STAGE_PIXEL );

            // Bind Sampler
            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            bilinearSampler->bind( cmdList, 2 );
            bilinearSampler->bind( cmdList, 3 );

            // Update viewport
            auto& pipelineDimensions = renderPipelineResources->getActiveViewport();

            auto& inputDesc = inputRenderTarget->getDescription();
            Viewport copyViewport( pipelineDimensions );
            copyViewport.Width = inputDesc.Width;
            copyViewport.Height = inputDesc.Height;
            cmdList->setViewportCmd( copyViewport );

            // Bind Dummy Vertex Array
            cmdList->unbindVertexArrayCmd();

            cmdList->drawCmd( 3 );
            inputRenderTarget->unbind( cmdList );

            cmdList->bindBackbufferCmd();
        }
    );

    return RenderPass.output[0];
}

FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( CopyTexturePass, [=]( RenderPipeline* renderPipeline ) { return AddCopyTexturePass( renderPipeline, false ); } )
FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( CopyTextureToMSAAPass, [=]( RenderPipeline* renderPipeline ) { return AddCopyTexturePass( renderPipeline, true ); } )
