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

static fnPipelineMutableResHandle_t AddMSAAResolvePass( RenderPipeline* renderPipeline, const int sampleCount = 1, const bool useTemporalAA = false, fnPipelineResHandle_t previousFrameRT = -1 )
{
    struct ResolveConstantBuffer
    {
        glm::vec2   InputTextureDimension;
        float       FilterSize;
        int32_t     SampleRadius;
    };

    auto RenderPass = renderPipeline->addRenderPass(
        "MSAA Resolve Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            auto colorBuffer = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) );
            auto depthBuffer = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainDepthRT" ) );
            auto velocityBuffer = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainVelocityRT" ) );

            RenderPassTextureDesc passRenderTargetDesc = {};
            passRenderTargetDesc.resourceToCopy = colorBuffer;
            passRenderTargetDesc.copyResource = true;
            passRenderTargetDesc.description.mipCount = 1;
            passRenderTargetDesc.description.samplerCount = 1;

            passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );
            renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ), passData.output[0] );

            // Read input render targets
            passData.input[0] = renderPipelineBuilder->readRenderTarget( colorBuffer );
            passData.input[1] = renderPipelineBuilder->readRenderTarget( velocityBuffer );
            passData.input[2] = renderPipelineBuilder->readRenderTarget( depthBuffer );
            passData.input[3] = renderPipelineBuilder->readRenderTarget( previousFrameRT );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "MSAA Resolve Pass" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTriangle" );
            
            if ( sampleCount == 2 ) {
                passPipelineState.hashcode = FLAN_STRING_HASH( "MSAA2 Resolve Pass" );
                passPipelineState.pixelStage = FLAN_STRING( "MSAAResolve2" );
            } else if ( sampleCount == 4 ) {
                passPipelineState.hashcode = FLAN_STRING_HASH( "MSAA4 Resolve Pass" );
                passPipelineState.pixelStage = FLAN_STRING( "MSAAResolve4" );
            } else if ( sampleCount == 8 ) {
                passPipelineState.hashcode = FLAN_STRING_HASH( "MSAA8 Resolve Pass" );
                passPipelineState.pixelStage = FLAN_STRING( "MSAAResolve8" );
            } else {
                passPipelineState.hashcode = FLAN_STRING_HASH( "MSAA1 Resolve Pass" );
                passPipelineState.pixelStage = FLAN_STRING( "MSAAResolve1" );
            }

            if ( useTemporalAA ) {
                passPipelineState.hashcode |= FLAN_STRING_HASH( "TAA" );
                passPipelineState.pixelStage.append( FLAN_STRING( "TAA" ) );
            }

            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            passPipelineState.depthStencilState.enableDepthTest = false;
            passPipelineState.depthStencilState.enableDepthWrite = false;

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            // CBuffer
            BufferDesc passConstantBuffer = {};
            passConstantBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            passConstantBuffer.Size = sizeof( ResolveConstantBuffer );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( passConstantBuffer );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Set Ouput Target
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &ouputRenderTarget );

            // Bind Input Target
            auto inputRenderTarget = renderPipelineResources->getRenderTarget( passData.input[0] );
            auto velocityRenderTarget = renderPipelineResources->getRenderTarget( passData.input[1] );
            auto depthRenderTarget = renderPipelineResources->getRenderTarget( passData.input[2] );
            auto previousRenderTarget = renderPipelineResources->getRenderTarget( passData.input[3] );

            inputRenderTarget->bind( cmdList, 0, SHADER_STAGE_PIXEL );
            velocityRenderTarget->bind( cmdList, 1, SHADER_STAGE_PIXEL );
            depthRenderTarget->bind( cmdList, 2, SHADER_STAGE_PIXEL );
            previousRenderTarget->bind( cmdList, 3, SHADER_STAGE_PIXEL );

            // Bind Constant Buffer
            auto constantBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            auto inputRTDesc = inputRenderTarget->getDescription();

            ResolveConstantBuffer resolveInfos;
            resolveInfos.FilterSize = 2.0f;
            resolveInfos.SampleRadius = static_cast<int32_t>( ( resolveInfos.FilterSize / 2.0f ) + 0.499f );
            resolveInfos.InputTextureDimension = glm::vec2( inputRTDesc.width, inputRTDesc.height );

            // Upload Infos
            constantBuffer->updateAsynchronous( cmdList, &resolveInfos, sizeof( ResolveConstantBuffer ) );
            constantBuffer->bind( cmdList, 0, SHADER_STAGE_PIXEL );


            auto autoExposureBuffer = renderPipelineResources->getWellKnownImportedResource<AutoExposureBuffer>()->exposureBuffer;
            autoExposureBuffer->bindReadOnly( cmdList, 16, SHADER_STAGE_PIXEL );

            // Update viewport
            auto& pipelineDimensions = renderPipelineResources->getActiveViewport();
            cmdList->setViewportCmd( pipelineDimensions );

            // Bind Dummy Vertex Array
            cmdList->unbindVertexArrayCmd();

            cmdList->drawCmd( 3 );

            inputRenderTarget->unbind( cmdList );
            velocityRenderTarget->unbind( cmdList );
            depthRenderTarget->unbind( cmdList );
            previousRenderTarget->unbind( cmdList );
            autoExposureBuffer->unbind( cmdList );

            cmdList->bindBackbufferCmd();
        }
    );

    return RenderPass.output[0];
}
