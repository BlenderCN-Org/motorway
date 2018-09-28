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

static fnPipelineMutableResHandle_t AddMSAADepthResolvePass( RenderPipeline* renderPipeline, const unsigned int sampleCount = 1 )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "MSAA Depth Resolve Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            auto depthBuffer = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainDepthRT" ) );

            RenderPassTextureDesc passRenderTargetDesc = {};
            passRenderTargetDesc.resourceToCopy = depthBuffer;
            passRenderTargetDesc.copyResource = true;
            
            passRenderTargetDesc.description.samplerCount = 1;
            passRenderTargetDesc.description.mipCount = 1;
            passRenderTargetDesc.description.flags.isDepthResource = 1;

            passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );
            renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "MainResolvedDepthRT" ), passData.output[0] );

            // Read input render target
            passData.input[0] = renderPipelineBuilder->readRenderTarget( depthBuffer );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "MSAA Depth Resolve Pass" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTriangle" );

            if ( sampleCount == 2 ) {
                passPipelineState.pixelStage = FLAN_STRING( "MSAADepthResolve2" );
            } else if ( sampleCount == 4 ) {
                passPipelineState.pixelStage = FLAN_STRING( "MSAADepthResolve4" );
            } else if ( sampleCount == 8 ) {
                passPipelineState.pixelStage = FLAN_STRING( "MSAADepthResolve8" );
            } else {
                passPipelineState.pixelStage = FLAN_STRING( "MSAADepthResolve1" );
            }

            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            passPipelineState.depthStencilState.enableDepthTest = true;
            passPipelineState.depthStencilState.enableDepthWrite = true;
            passPipelineState.depthStencilState.depthComparisonFunc = flan::rendering::eComparisonFunction::COMPARISON_FUNCTION_ALWAYS;

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Set Ouput Target
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( nullptr, ouputRenderTarget, 0 );

            // Bind Input Target
            auto inputRenderTarget = renderPipelineResources->getRenderTarget( passData.input[0] );
            inputRenderTarget->bind( cmdList, 0, SHADER_STAGE_PIXEL );

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

FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( MSAADepthResolvePass1, [=]( RenderPipeline* renderPipeline ) { return AddMSAADepthResolvePass( renderPipeline, 1 ); } )
FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( MSAADepthResolvePass2, [=]( RenderPipeline* renderPipeline ) { return AddMSAADepthResolvePass( renderPipeline, 2 ); } )
FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( MSAADepthResolvePass4, [=]( RenderPipeline* renderPipeline ) { return AddMSAADepthResolvePass( renderPipeline, 4 ); } )
FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( MSAADepthResolvePass8, [=]( RenderPipeline* renderPipeline ) { return AddMSAADepthResolvePass( renderPipeline, 8 ); } )
