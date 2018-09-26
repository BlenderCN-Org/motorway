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

static fnPipelineMutableResHandle_t AddFXAAPass( RenderPipeline* renderPipeline )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "FXAA Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            RenderPassTextureDesc passRenderTargetDesc = {};
            passRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
            passRenderTargetDesc.description.depth = 1;
            passRenderTargetDesc.description.mipCount = 1;
            passRenderTargetDesc.description.samplerCount = 1;
            passRenderTargetDesc.description.arraySize = 1;
            passRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            passRenderTargetDesc.useGlobalDimensions = true;
            
            passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

            passData.input[0] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainPresentColorRT" ) );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "FXAA Pass" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTriangle" );
            passPipelineState.pixelStage = FLAN_STRING( "FXAA" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            passPipelineState.depthStencilState.enableDepthTest = false;
            passPipelineState.depthStencilState.enableDepthWrite = false;

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Set Ouput Target
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &ouputRenderTarget );

            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Bind Input Target
            auto inputRenderTarget = renderPipelineResources->getRenderTarget( passData.input[0] );
            inputRenderTarget->bind( cmdList, 2 );

            // Bind Dummy Vertex Array
            cmdList->unbindVertexArrayCmd();

            cmdList->drawCmd( 3 );

            inputRenderTarget->unbind( cmdList );
            cmdList->bindBackbufferCmd();
        }
    );

    return RenderPass.output[0];
}

FLAN_REGISTER_RENDERPASS( FXAAPass, AddFXAAPass )
