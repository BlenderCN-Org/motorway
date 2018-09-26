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

#include <Rendering/CommandList.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/PipelineState.h>
#include <Graphics/RenderPipeline.h>

#include <Graphics/TextureSaveTools.h>

static fnPipelineResHandle_t AddPresentPass( RenderPipeline* renderPipeline )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Present",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            passData.input[0] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainPresentColorRT" ) );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState;
            passPipelineState.hashcode = FLAN_STRING_HASH( "Present" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTrianglePresent" );
            passPipelineState.pixelStage = FLAN_STRING( "CopyTexture" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
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
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Bind Backbuffer
            cmdList->bindBackbufferCmd();

            // Update viewport
            auto& pipelineDimensions = renderPipelineResources->getActiveViewport();
            cmdList->setViewportCmd( pipelineDimensions );

            // Bind Sampler
            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            bilinearSampler->bind( cmdList, 3 );

            // Bind Input Target
            auto inputRenderTarget = renderPipelineResources->getRenderTarget( passData.input[0] );
            inputRenderTarget->bind( cmdList, TEXTURE_SLOT_INDEX_LINEAR_SAMPLED, SHADER_STAGE_PIXEL );

            // Bind Dummy Vertex Array
            cmdList->unbindVertexArrayCmd();

            cmdList->drawCmd( 3 );

            inputRenderTarget->unbind( cmdList );
        }
    );

    return -1;
}

FLAN_REGISTER_RENDERPASS( PresentPass, AddPresentPass )
