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

#include "MSAAResolvePass.h"

static fnPipelineMutableResHandle_t AddSubsurfaceScatteringBlurPass( RenderPipeline* renderPipeline, const fnPipelineMutableResHandle_t colorRT, const fnPipelineMutableResHandle_t depthRT, const fnPipelineMutableResHandle_t thinGBuffer, bool isFirstPass = true )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Subsurface Scattering Blur Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            RenderPassTextureDesc passRenderTargetDesc = {};
            passRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
            passRenderTargetDesc.description.depth = 1;
            passRenderTargetDesc.description.mipCount = 1;
            passRenderTargetDesc.description.samplerCount = 1;
            passRenderTargetDesc.description.arraySize = 1;
            passRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            passRenderTargetDesc.copyResource = true;
            passRenderTargetDesc.resourceToCopy = colorRT;

            passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

            // Read target to downsample
            passData.input[0] = renderPipelineBuilder->readRenderTarget( colorRT );
            passData.input[1] = renderPipelineBuilder->readRenderTarget( depthRT );
            passData.input[2] = renderPipelineBuilder->readRenderTarget( thinGBuffer );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "SubsurfaceScatteringBlur" + ( isFirstPass ) ? " (1)" : " (2)" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTriangle" );
            passPipelineState.pixelStage = isFirstPass ? FLAN_STRING( "SubsurfaceScatteringBlur1" ) : FLAN_STRING( "SubsurfaceScatteringBlur@" );
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
            auto depthRenderTarget = renderPipelineResources->getRenderTarget( passData.input[1] );
            auto gBufferRenderTarget = renderPipelineResources->getRenderTarget( passData.input[2] );
            inputRenderTarget->bind( cmdList, 0, SHADER_STAGE_PIXEL );
            depthRenderTarget->bind( cmdList, 1, SHADER_STAGE_PIXEL );
            gBufferRenderTarget->bind( cmdList, 2, SHADER_STAGE_PIXEL );

            // Set Ouput Target
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &ouputRenderTarget );

            // Update Viewport
            auto texDescription = ouputRenderTarget->getDescription();
            cmdList->setViewportCmd( texDescription.width, texDescription.height );

            cmdList->unbindVertexArrayCmd();

            cmdList->drawCmd( 3 );

            cmdList->bindBackbufferCmd();
            inputRenderTarget->unbind( cmdList );
            cmdList->setViewportCmd( viewport );
        }
    );

    return RenderPass.output[0];
}

static fnPipelineMutableResHandle_t AddSubsurfaceScatteringBlurPass( RenderPipeline* renderPipeline, const bool useMSAA = false )
{
    renderPipeline->addPipelineSetupPass(
        [&]( RenderPipeline* renderPipeline, RenderPipelineBuilder* renderPipelineBuilder ) {
            auto colorRenderTarget = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) );
            auto depthRenderTarget = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( useMSAA ? "MainResolvedDepthRT" : "MainDepthRT" ) );
            auto thinGBufferRenderTarget = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "ThinGBufferRT" ) );

            auto resolvedThinGBuffer = AddFastMSAAResolvePass( renderPipeline, thinGBufferRenderTarget );
            auto outputFirstPass = AddSubsurfaceScatteringBlurPass( renderPipeline, colorRenderTarget, depthRenderTarget, resolvedThinGBuffer, true );
            auto output = AddSubsurfaceScatteringBlurPass( renderPipeline, outputFirstPass, depthRenderTarget, resolvedThinGBuffer, false );

            renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ), output );
            renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "MainGBufferRT" ), resolvedThinGBuffer );
        }
    );

    return -1;
}

FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( SubsurfaceScatteringPassMSAA, [=]( RenderPipeline* renderPipeline ) { return AddSubsurfaceScatteringBlurPass( renderPipeline, true ); } )
FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( SubsurfaceScatteringPass, [=]( RenderPipeline* renderPipeline ) { return AddSubsurfaceScatteringBlurPass( renderPipeline, false ); } )
