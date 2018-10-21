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

#include <Graphics/RenderModules/AutomaticExposureModule.h>

struct CompositionSettings
{
    float BloomExposureCompensation = -16.0f;
    float BloomStrength = 0.03f;
    float WhitePoint = 1.0f;
    float BlackPoint = 0.0f;

    float JunctionPoint = 0.25f;
    float ToeStrength = 0.0f;
    float ShoulderStrength = 1.0f;
    uint32_t __PADDING__;
};

FLAN_DEV_VAR( GraphicsCompositionSettings, "Graphics Composition Step Settings", {}, CompositionSettings )

static fnPipelineMutableResHandle_t AddCompositionPass( RenderPipeline* renderPipeline )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Composition Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            RenderPassTextureDesc passRenderTargetDesc = {};
            passRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
            passRenderTargetDesc.description.depth = 1;
            passRenderTargetDesc.description.mipCount = 1;
            passRenderTargetDesc.description.samplerCount = 1;
            passRenderTargetDesc.description.arraySize = 1;
            passRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            passRenderTargetDesc.copyResource = true;
            passRenderTargetDesc.resourceToCopy = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) );

            passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

            renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "MainPresentColorRT" ), passData.output[0] );

            passData.input[0] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) );
            passData.input[1] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "BloomRT" ) );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "Composition Pass" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTriangle" );
            passPipelineState.pixelStage = FLAN_STRING( "FrameComposition" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            passPipelineState.rasterizerState.cullMode = flan::rendering::eCullMode::CULL_MODE_NONE;
            passPipelineState.depthStencilState.enableDepthTest = false;
            passPipelineState.depthStencilState.enableDepthWrite = false;
            passPipelineState.blendState.enableBlend = false;
            passPipelineState.rasterizerState.useTriangleCCW = true;

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            // Sampler State
            SamplerDesc bilinearSamplerDesc;
            bilinearSamplerDesc.addressU = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = flan::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.samplers[0] = renderPipelineBuilder->allocateSampler( bilinearSamplerDesc );

            // Constant Buffer
            BufferDesc passBuffer = {};
            passBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            passBuffer.Size = sizeof( CompositionSettings );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( passBuffer );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Set Ouput Target
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &ouputRenderTarget );

            // Bind Sampler
            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            bilinearSampler->bind( cmdList, 3 );

            // Get Constant Buffer 
            auto passConstantBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            passConstantBuffer->updateAsynchronous( cmdList, &GraphicsCompositionSettings, sizeof( CompositionSettings ) );
            passConstantBuffer->bind( cmdList, 2 );

            auto autoExposureBuffer = renderPipelineResources->getWellKnownImportedResource<AutoExposureBuffer>()->exposureBuffer; 
            autoExposureBuffer->bindReadOnly( cmdList, 16, SHADER_STAGE_PIXEL );

            // Update viewport
            auto& pipelineDimensions = renderPipelineResources->getActiveViewport();
            cmdList->setViewportCmd( pipelineDimensions );

            // Get base Input Target
            auto inputRenderTarget = renderPipelineResources->getRenderTarget( passData.input[0] );
            auto bloomRenderTarget = renderPipelineResources->getRenderTarget( passData.input[1] );
            inputRenderTarget->bind( cmdList, 2 );
            bloomRenderTarget->bind( cmdList, 3 );

            cmdList->unbindVertexArrayCmd();
            cmdList->drawCmd( 3 );

            bloomRenderTarget->unbind( cmdList );
            inputRenderTarget->unbind( cmdList );
            autoExposureBuffer->unbind( cmdList );
            passConstantBuffer->unbind( cmdList );

            cmdList->bindBackbufferCmd();
        }
    );

    return RenderPass.output[0];
}

FLAN_REGISTER_RENDERPASS( CompositionPass, AddCompositionPass );
