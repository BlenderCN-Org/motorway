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

#include <Shared.h>

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>
#include <Rendering/PipelineState.h>
#include <Graphics/RenderPipeline.h>
#include <Graphics/TextureSlotIndexes.h>

static fnPipelineMutableResHandle_t AddDownsamplingPass( RenderPipeline* renderPipeline, const fnPipelineMutableResHandle_t renderTarget, const float downsamplingFactor = 2.0f )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Downsample Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            RenderPassTextureDesc passRenderTargetDesc = {};
            passRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
            passRenderTargetDesc.description.depth = 1;
            passRenderTargetDesc.description.mipCount = 1;
            passRenderTargetDesc.description.samplerCount = 1;

            passRenderTargetDesc.lamdaComputeDimension = [=]( const uint32_t width, const uint32_t height ) {
                return glm::uvec2( width / downsamplingFactor, height / downsamplingFactor );
            };
            passRenderTargetDesc.resourceToCopy = renderTarget;
            passRenderTargetDesc.copyResource = true;

            passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

            // Read target to downsample
            passData.input[0] = renderPipelineBuilder->readRenderTarget( renderTarget );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "DownsamplePass" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTriangle" );
            passPipelineState.pixelStage = FLAN_STRING( "DownsampleFast" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            passPipelineState.rasterizerState.cullMode = flan::rendering::eCullMode::CULL_MODE_NONE;
            passPipelineState.depthStencilState.enableDepthTest = false;
            passPipelineState.depthStencilState.enableDepthWrite = false;

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
            auto viewport = cmdList->getViewportCmd();

            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );
            
            // Bind Sampler
            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            bilinearSampler->bind( cmdList, 0 );

            // Get base Input Target
            auto inputRenderTarget = renderPipelineResources->getRenderTarget( passData.input[0] );
            inputRenderTarget->bind( cmdList, 0, SHADER_STAGE_PIXEL );

            // Set Ouput Target
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &ouputRenderTarget );

            // Update Viewport
            auto texDescription = ouputRenderTarget->getDescription();
            cmdList->setViewportCmd( texDescription.width, texDescription.height );

            cmdList->unbindVertexArrayCmd();

            // Downsample
            cmdList->drawCmd( 3 );

            cmdList->bindBackbufferCmd();
            inputRenderTarget->unbind( cmdList );
            cmdList->setViewportCmd( viewport );
        }
    );

    return RenderPass.output[0];
}

static fnPipelineMutableResHandle_t AddStabilizedDownsamplingPass( RenderPipeline* renderPipeline, const fnPipelineMutableResHandle_t renderTarget, const unsigned int downsamplingFactor = 2 )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Weighted Stabilized Downsample Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            RenderPassTextureDesc passRenderTargetDesc = {};
            passRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
            passRenderTargetDesc.description.depth = 1;
            passRenderTargetDesc.description.mipCount = 1;
            passRenderTargetDesc.description.samplerCount = 1;
            passRenderTargetDesc.description.arraySize = 1;
            passRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;

            passRenderTargetDesc.lamdaComputeDimension = [=]( const uint32_t width, const uint32_t height ) {
                return glm::uvec2( width / downsamplingFactor, height / downsamplingFactor );
            };
            passRenderTargetDesc.resourceToCopy = renderTarget;

            passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

            // Read target to downsample
            passData.input[0] = renderPipelineBuilder->readRenderTarget( renderTarget );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "DownsampleWeightedStabilized" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTriangle" );
            passPipelineState.pixelStage = FLAN_STRING( "DownsampleWeightedStabilized" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            passPipelineState.rasterizerState.cullMode = flan::rendering::eCullMode::CULL_MODE_NONE;
            passPipelineState.depthStencilState.enableDepthTest = false;
            passPipelineState.depthStencilState.enableDepthWrite = false;

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
            auto viewport = cmdList->getViewportCmd();

            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Bind Sampler
            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            bilinearSampler->bind( cmdList, 3 );

            // Get base Input Target
            auto inputRenderTarget = renderPipelineResources->getRenderTarget( passData.input[0] );
            inputRenderTarget->bind( cmdList, TEXTURE_SLOT_INDEX_LINEAR_SAMPLED, SHADER_STAGE_PIXEL );

            // Set Ouput Target
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &ouputRenderTarget );

            // Update Viewport
            auto texDescription = ouputRenderTarget->getDescription();
            cmdList->setViewportCmd( texDescription.width, texDescription.height );

            cmdList->unbindVertexArrayCmd();

            // Downsample
            cmdList->drawCmd( 3 );

            cmdList->bindBackbufferCmd();
            inputRenderTarget->unbind( cmdList );
            cmdList->setViewportCmd( viewport );
        }
    );

    return RenderPass.output[0];
}

static fnPipelineMutableResHandle_t AddWeightedDownsamplingPass( RenderPipeline* renderPipeline, const fnPipelineMutableResHandle_t renderTarget, const unsigned int downsamplingFactor = 2 )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Weighted Downsample Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            RenderPassTextureDesc passRenderTargetDesc = {};
            passRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
            passRenderTargetDesc.description.depth = 1;
            passRenderTargetDesc.description.mipCount = 1;
            passRenderTargetDesc.description.samplerCount = 1;
            passRenderTargetDesc.description.arraySize = 1;
            passRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;

            passRenderTargetDesc.lamdaComputeDimension = [=]( const uint32_t width, const uint32_t height ) {
                return glm::uvec2( width / downsamplingFactor, height / downsamplingFactor );
            };
            passRenderTargetDesc.resourceToCopy = renderTarget;

            passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

            // Read target to downsample
            passData.input[0] = renderPipelineBuilder->readRenderTarget( renderTarget );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "WeightedDownSamplePass" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTriangle" );
            passPipelineState.pixelStage = FLAN_STRING( "DownsampleWeighted" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            passPipelineState.rasterizerState.cullMode = flan::rendering::eCullMode::CULL_MODE_NONE;
            passPipelineState.depthStencilState.enableDepthTest = false;
            passPipelineState.depthStencilState.enableDepthWrite = false;

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
            auto viewport = cmdList->getViewportCmd();

            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Bind Sampler
            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            bilinearSampler->bind( cmdList, 3 );

            // Get base Input Target
            auto inputRenderTarget = renderPipelineResources->getRenderTarget( passData.input[0] );
            inputRenderTarget->bind( cmdList, TEXTURE_SLOT_INDEX_LINEAR_SAMPLED, SHADER_STAGE_PIXEL  );

            // Set Ouput Target
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &ouputRenderTarget );

            // Update Viewport
            auto texDescription = ouputRenderTarget->getDescription();
            cmdList->setViewportCmd( texDescription.width, texDescription.height );

            cmdList->unbindVertexArrayCmd();

            // Downsample
            cmdList->drawCmd( 3 );

            cmdList->bindBackbufferCmd();
            inputRenderTarget->unbind( cmdList );
            cmdList->setViewportCmd( viewport );
        }
    );

    return RenderPass.output[0];
}

static fnPipelineMutableResHandle_t AddWeightedUpsamplingPass( RenderPipeline* renderPipeline, const fnPipelineMutableResHandle_t renderTarget, const float filterRadius = 1.0f, const fnPipelineMutableResHandle_t combinedRenderTarget = -1, const unsigned int upsamplingFactor = 2 )
{
    struct UpsamplingConstantBuffer
    {
        float       InverseTextureWidth;
        float       InverseTextureHeight;
        float       FilterRadius;
        uint32_t    __UNUSED__;
    };

    auto RenderPass = renderPipeline->addRenderPass(
        "Weighted Upsample Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            RenderPassTextureDesc passRenderTargetDesc = {};
            passRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
            passRenderTargetDesc.description.depth = 1;
            passRenderTargetDesc.description.mipCount = 1;
            passRenderTargetDesc.description.samplerCount = 1;
            passRenderTargetDesc.description.arraySize = 1;
            passRenderTargetDesc.initialState = RenderPassTextureDesc::CLEAR;
            passRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;

            passRenderTargetDesc.lamdaComputeDimension = [=]( const uint32_t width, const uint32_t height ) {
                return glm::uvec2( width * upsamplingFactor, height * upsamplingFactor );
            };
            passRenderTargetDesc.resourceToCopy = combinedRenderTarget;

            passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

            // Read target to downsample
            passData.input[0] = renderPipelineBuilder->readRenderTarget( renderTarget );
            passData.input[1] = renderPipelineBuilder->readRenderTarget( combinedRenderTarget );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "WeightedUpSamplePass" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTriangle" );
            passPipelineState.pixelStage = FLAN_STRING( "UpsampleWeighted" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            passPipelineState.rasterizerState.cullMode = flan::rendering::eCullMode::CULL_MODE_NONE;
            passPipelineState.depthStencilState.enableDepthTest = false;
            passPipelineState.depthStencilState.enableDepthWrite = false;

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            // CBuffer
            BufferDesc passConstantBuffer = {};
            passConstantBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            passConstantBuffer.Size = sizeof( UpsamplingConstantBuffer );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( passConstantBuffer );

            // Sampler State
            SamplerDesc bilinearSamplerDesc;
            bilinearSamplerDesc.addressU = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = flan::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.samplers[0] = renderPipelineBuilder->allocateSampler( bilinearSamplerDesc );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            auto viewport = cmdList->getViewportCmd();

            // Bind Sampler
            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            bilinearSampler->bind( cmdList, 3 );

            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Get base Input Target
            auto inputRenderTarget = renderPipelineResources->getRenderTarget( passData.input[0] );
            auto inputAccumulatorRenderTarget = renderPipelineResources->getRenderTarget( passData.input[1] );
            inputRenderTarget->bind( cmdList, TEXTURE_SLOT_INDEX_LINEAR_SAMPLED, SHADER_STAGE_PIXEL );
            inputAccumulatorRenderTarget->bind( cmdList, TEXTURE_SLOT_INDEX_POST_FX_DEFAULT, SHADER_STAGE_PIXEL );

            // Set Ouput Target
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &ouputRenderTarget );

            // Update Viewport
            auto constantBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );

            auto& outputDesc = ouputRenderTarget->getDescription();
            UpsamplingConstantBuffer upsamplingInfos;
            upsamplingInfos.FilterRadius = 1.0f;
            upsamplingInfos.InverseTextureWidth = 1.0f / outputDesc.width;
            upsamplingInfos.InverseTextureHeight = 1.0f / outputDesc.height;

            constantBuffer->updateAsynchronous( cmdList, &upsamplingInfos, sizeof( &upsamplingInfos ) );
            constantBuffer->bind( cmdList, 0, SHADER_STAGE_PIXEL );

            // Update Viewport
            cmdList->setViewportCmd( outputDesc.width, outputDesc.height );

            // Upsample
            cmdList->unbindVertexArrayCmd();

            // Downsample
            cmdList->drawCmd( 3 );

            cmdList->bindBackbufferCmd();
            inputRenderTarget->unbind( cmdList );
            cmdList->setViewportCmd( viewport );
        }
    );

    return RenderPass.output[0];
}
