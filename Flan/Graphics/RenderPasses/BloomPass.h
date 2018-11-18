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

#include <Core/Factory.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/PipelineState.h>
#include <Graphics/GraphicsAssetManager.h>
#include <Graphics/RenderPipeline.h>
#include <Graphics/CBufferIndexes.h>
#include <Shared.h>

#include "DownsamplingPass.h"

static fnPipelineMutableResHandle_t AddBloomBrightPass( RenderPipeline* renderPipeline, const fnPipelineMutableResHandle_t renderTarget )
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
            passRenderTargetDesc.resourceToCopy = renderTarget;
            passRenderTargetDesc.copyResource = true;

            passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

            // Read target to downsample
            passData.input[0] = renderPipelineBuilder->readRenderTarget( renderTarget );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "BrightPassBloom" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTriangle" );
            passPipelineState.pixelStage = FLAN_STRING( "BrightPass" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            passPipelineState.rasterizerState.cullMode = flan::rendering::eCullMode::CULL_MODE_NONE;
            passPipelineState.depthStencilState.enableDepthTest = false;
            passPipelineState.depthStencilState.enableDepthWrite = false;

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            auto viewport = cmdList->getViewportCmd();

            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

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

static fnPipelineMutableResHandle_t AddBloomPass( RenderPipeline* renderPipeline )
{
    renderPipeline->addPipelineSetupPass(
        [&]( RenderPipeline* renderPipeline, RenderPipelineBuilder* renderPipelineBuilder ) {
            auto mip0 = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) );

            auto brightPassResult = AddBloomBrightPass( renderPipeline, mip0 );

            auto mip1 = AddStabilizedDownsamplingPass( renderPipeline, brightPassResult, 2 );
            auto mip2 = AddWeightedDownsamplingPass( renderPipeline, brightPassResult, 4 );
            auto mip3 = AddWeightedDownsamplingPass( renderPipeline, brightPassResult, 8 );
            auto mip4 = AddWeightedDownsamplingPass( renderPipeline, brightPassResult, 16 );
            auto mip5 = AddWeightedDownsamplingPass( renderPipeline, brightPassResult, 32 );

            mip4 = AddWeightedUpsamplingPass( renderPipeline, mip4, 1.0f, mip5 );
            mip3 = AddWeightedUpsamplingPass( renderPipeline, mip3, 1.0f, mip4 );
            mip2 = AddWeightedUpsamplingPass( renderPipeline, mip2, 1.0f, mip3 );
            mip1 = AddWeightedUpsamplingPass( renderPipeline, mip1, 1.0f, mip2 );

            brightPassResult = AddWeightedUpsamplingPass( renderPipeline, brightPassResult, 1.0f, mip1 );

            renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "BloomRT" ), brightPassResult );
        }
    );

    return -1;
}

FLAN_REGISTER_RENDERPASS( BloomPass, AddBloomPass )
