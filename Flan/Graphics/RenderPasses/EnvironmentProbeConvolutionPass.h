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
#include <Graphics/TextureSlotIndexes.h>
#include <Graphics/CBufferIndexes.h>
#include <Shaders/Shared.h>
#include <Shared.h>

struct EnvironmentProbeOutput
{
    fnPipelineMutableResHandle_t StandardDiffuseLD;
    fnPipelineMutableResHandle_t StandardSpecularLD;
};

static EnvironmentProbeOutput AddEnvironmentProbeConvolutionPass( RenderPipeline* renderPipeline, fnPipelineMutableResHandle_t colorRenderTarget, uint32_t envProbeIndex, uint32_t cubemapFaceIndex = 0, uint32_t mipLevel = 0 )
{
    struct ConvolutionParameters
    {
        glm::vec3	CubeDirectionX;
        float		RoughnessValue;

        glm::vec3	CubeDirectionY;
        float       Width;

        glm::vec3	CubeDirectionZ;
        uint32_t    ProbeIndex;
    };

    auto RenderPass = renderPipeline->addRenderPass(
        "Environment Probe Convolution Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            RenderPassTextureDesc passRenderTargetDesc = {};
            passRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
            passRenderTargetDesc.description.depth = 1;
            passRenderTargetDesc.description.mipCount = 1;
            passRenderTargetDesc.description.samplerCount = 1;
            passRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            passRenderTargetDesc.useGlobalDimensions = true;
            passRenderTargetDesc.initialState = RenderPassTextureDesc::CLEAR;

            // LD Diffuse Render Target
            passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

            // LD Specular Render Target
            passData.output[1] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

            passData.input[0] = renderPipelineBuilder->readRenderTarget( colorRenderTarget );

            // Constant Buffer
            BufferDesc convolutionBuffer = {};
            convolutionBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            convolutionBuffer.Size = sizeof( ConvolutionParameters );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( convolutionBuffer );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "Environment Probe Convolution Pass" );
            passPipelineState.vertexStage = FLAN_STRING( "IBLConvolution" );
            passPipelineState.pixelStage  = FLAN_STRING( "IBLConvolution" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

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

            // Retrieve Convolution Buffer
            auto convolutionBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            
            ConvolutionParameters convolutionParameters;
            switch ( cubemapFaceIndex % 6 ) {
            // X
            case 0:
                convolutionParameters.CubeDirectionX = { 0, 0, -1 };
                convolutionParameters.CubeDirectionY = { 0, 1, 0 };
                convolutionParameters.CubeDirectionZ = { 1, 0, 0 };
                break;

            case 1:
                convolutionParameters.CubeDirectionX = { 0, 0, 1 };
                convolutionParameters.CubeDirectionY = { 0, 1, 0 };
                convolutionParameters.CubeDirectionZ = { -1, 0, 0 };
                break;

            // Y
            case 2:
                convolutionParameters.CubeDirectionX = { 1, 0, 0 };
                convolutionParameters.CubeDirectionY = { 0, 0, -1 };
                convolutionParameters.CubeDirectionZ = { 0, 1, 0 };
                break;

            case 3:
                convolutionParameters.CubeDirectionX = { 1, 0, 0 };
                convolutionParameters.CubeDirectionY = { 0, 0, 1 };
                convolutionParameters.CubeDirectionZ = { 0, -1, 0 };
                break;

            // Z
            case 4:
                convolutionParameters.CubeDirectionX = { 1, 0, 0 };
                convolutionParameters.CubeDirectionY = { 0, 1, 0 };
                convolutionParameters.CubeDirectionZ = { 0, 0, 1 };
                break;

            case 5:
                convolutionParameters.CubeDirectionX = { -1, 0, 0 };
                convolutionParameters.CubeDirectionY = { 0, 1, 0 };
                convolutionParameters.CubeDirectionZ = { 0, 0, -1 };
                break;
            };
            convolutionParameters.ProbeIndex = envProbeIndex;

            const auto& activeViewport = renderPipelineResources->getActiveViewport();

            convolutionParameters.Width = static_cast<float>( activeViewport.Width );
            convolutionParameters.RoughnessValue = mipLevel / glm::max( 1.0f, glm::log2<float>( convolutionParameters.Width ) );

            convolutionBuffer->updateAsynchronous( cmdList, &convolutionParameters, sizeof( ConvolutionParameters ) );
            convolutionBuffer->bind( cmdList, 0, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL );

            // Bind Sampler
            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            bilinearSampler->bind( cmdList, 2 );

            // Set Ouput Target
            auto ouputDiffuseRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            auto ouputSpecularRenderTarget = renderPipelineResources->getRenderTarget( passData.output[1] );

            RenderTarget* renderTargets[2] = {
                ouputDiffuseRenderTarget,
                ouputSpecularRenderTarget,
            };

            // Retrieve input
            auto envProbeColor = renderPipelineResources->getRenderTarget( passData.input[0] );
            envProbeColor->bind( cmdList, 2, SHADER_STAGE_PIXEL );

            cmdList->bindRenderTargetsCmd( renderTargets, nullptr, 2 );

            // Update viewport
            Viewport envProbeViewport = {};
            envProbeViewport.Width = activeViewport.Width;
            envProbeViewport.Height = activeViewport.Height;
            envProbeViewport.X = 0;
            envProbeViewport.Y = 0;
            envProbeViewport.MinDepth = 0.0f;
            envProbeViewport.MaxDepth = 1.0f;

            cmdList->setViewportCmd( envProbeViewport );

            // Convolute!
            cmdList->drawCmd( 4 );

            envProbeColor->unbind( cmdList );
        }
    );

    return {
        RenderPass.output[0],
        RenderPass.output[1]
    };
}
