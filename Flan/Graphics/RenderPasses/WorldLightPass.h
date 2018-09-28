/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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

#include <Graphics/RenderableEntityManager.h>
#include <Graphics/DrawCommand.h>
#include <Framework/Material.h>

#include <Core/Profiler.h>

using namespace flan::rendering;

#define TEXTURE_FILTERING_OPTION_LIST( option )\
    option( BILINEAR )\
    option( TRILINEAR )\
    option( ANISOTROPIC_8 )\
    option( ANISOTROPIC_16 )

FLAN_ENV_OPTION_LIST( TextureFiltering, TEXTURE_FILTERING_OPTION_LIST )

FLAN_ENV_VAR( TextureFiltering,
    "Defines texture filtering quality [Bilinear/Trilinear/Anisotropic (8)/Anisotropic (16)]",
    BILINEAR,
    eTextureFiltering )

static fnPipelineMutableResHandle_t AddOpaqueLightPass( RenderPipeline* renderPipeline, const bool enableMSAA = false, const bool isRenderingProbe = false )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Opaque Lighting Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            // Color RT
            auto mainRenderTarget = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) );
            if ( mainRenderTarget == -1 ) {
                RenderPassTextureDesc passRenderTargetDesc = {};
                passRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
                passRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
                passRenderTargetDesc.description.depth = 1;
                passRenderTargetDesc.description.mipCount = 1;
                passRenderTargetDesc.description.arraySize = 1;
                passRenderTargetDesc.useGlobalDimensions = true;
                passRenderTargetDesc.useGlobalMultisamplingState = enableMSAA;
                passRenderTargetDesc.initialState = RenderPassTextureDesc::CLEAR;

                passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

                renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ), passData.output[0] );
            } else {
                passData.output[0] = mainRenderTarget;
            }

            // Velocity RT
            auto mainVelocityTarget = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainVelocityRT" ) );
            if ( mainVelocityTarget == -1 ) {
                RenderPassTextureDesc velocityRenderTargetDesc = {};
                velocityRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
                velocityRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16_FLOAT;
                velocityRenderTargetDesc.description.depth = 1;
                velocityRenderTargetDesc.description.mipCount = 1;
                velocityRenderTargetDesc.description.arraySize = 1;
                velocityRenderTargetDesc.useGlobalDimensions = true;
                velocityRenderTargetDesc.useGlobalMultisamplingState = enableMSAA;
                velocityRenderTargetDesc.initialState = RenderPassTextureDesc::CLEAR;

                passData.output[1] = renderPipelineBuilder->allocateTexture( velocityRenderTargetDesc );

                renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "MainVelocityRT" ), passData.output[1] );
            } else {
                passData.output[1] = mainVelocityTarget;
            }

            // Read Depth Buffer
            passData.input[0] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainDepthRT" ) );
            passData.input[1] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "CascadedShadowMappingShadowMap" ) );

            // Read Light Index Buffer
            passData.buffers[0] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "ForwardPlusLightIndexBuffer" ) );

            // Constant Buffer
            BufferDesc passBuffer = {};
            passBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            passBuffer.Size = sizeof( glm::mat4 );

            passData.buffers[1] = renderPipelineBuilder->allocateBuffer( passBuffer );

            BufferDesc rtDimensionBuffer;
            rtDimensionBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            rtDimensionBuffer.Size = sizeof( glm::uvec4 );

            passData.buffers[2] = renderPipelineBuilder->allocateBuffer( rtDimensionBuffer );

            BufferDesc cameraBuffer = {};
            cameraBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            cameraBuffer.Size = sizeof( Camera::Data );

            passData.buffers[3] = renderPipelineBuilder->allocateBuffer( cameraBuffer );

            BufferDesc atmosphereBuffer = {};
            atmosphereBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            atmosphereBuffer.Size = sizeof( AtmosphereModule::Parameters );

            passData.buffers[4] = renderPipelineBuilder->allocateBuffer( atmosphereBuffer );

            // Texture (geometry stuff) Sampler
            SamplerDesc matSamplerDesc;
            matSamplerDesc.addressU = eSamplerAddress::SAMPLER_ADDRESS_WRAP;
            matSamplerDesc.addressV = eSamplerAddress::SAMPLER_ADDRESS_WRAP;
            matSamplerDesc.addressW = eSamplerAddress::SAMPLER_ADDRESS_WRAP;

            switch ( TextureFiltering ) {
            case eTextureFiltering::ANISOTROPIC_16:
                matSamplerDesc.filter = eSamplerFilter::SAMPLER_FILTER_ANISOTROPIC_16;
                break;

            case eTextureFiltering::ANISOTROPIC_8:
                matSamplerDesc.filter = eSamplerFilter::SAMPLER_FILTER_ANISOTROPIC_8;
                break;

            case eTextureFiltering::TRILINEAR:
                matSamplerDesc.filter = eSamplerFilter::SAMPLER_FILTER_TRILINEAR;
                break;

            default:
            case eTextureFiltering::BILINEAR:
                matSamplerDesc.filter = eSamplerFilter::SAMPLER_FILTER_BILINEAR;
                break;
            }

            passData.samplers[0] = renderPipelineBuilder->allocateSampler( matSamplerDesc );


            SamplerDesc shadowComparisonSamplerDesc;
            shadowComparisonSamplerDesc.addressU = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            shadowComparisonSamplerDesc.addressV = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            shadowComparisonSamplerDesc.addressW = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            shadowComparisonSamplerDesc.filter = eSamplerFilter::SAMPLER_FILTER_COMPARISON_BILINEAR;
            shadowComparisonSamplerDesc.comparisonFunction = eComparisonFunction::COMPARISON_FUNCTION_LEQUAL;

            passData.samplers[1] = renderPipelineBuilder->allocateSampler( shadowComparisonSamplerDesc );

            SamplerDesc bilinearSamplerDesc;
            bilinearSamplerDesc.addressU = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = flan::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.samplers[2] = renderPipelineBuilder->allocateSampler( bilinearSamplerDesc );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind the light buffer (read only)
            auto lightIndexBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            lightIndexBuffer->bindReadOnly( cmdList, TEXTURE_SLOT_INDEX_LIGHT_INDEX_BUFFER, eShaderStage::SHADER_STAGE_PIXEL );

            // Retrieve Wellknown stuff
            auto brdfInputs = renderPipelineResources->getWellKnownImportedResource<BRDFInputs>();

            auto renderableEntities = renderPipelineResources->getWellKnownImportedResource<RenderableEntityManager::EntityBuffer>()->buffer;
            auto atmosphereData = renderPipelineResources->getWellKnownImportedResource<AtmosphereModule::Parameters>();

            brdfInputs->envProbeCapture->bind( cmdList, TEXTURE_SLOT_INDEX_ACTIVE_ENVMAP, SHADER_STAGE_PIXEL );
            brdfInputs->envProbeDiffuse->bind( cmdList, TEXTURE_SLOT_INDEX_ACTIVE_ENVMAP_DIFFUSE, SHADER_STAGE_PIXEL );
            brdfInputs->envProbeSpecular->bind( cmdList, TEXTURE_SLOT_INDEX_ACTIVE_ENVMAP_SPECULAR, SHADER_STAGE_PIXEL );
            brdfInputs->dfgLut->bind( cmdList, TEXTURE_SLOT_INDEX_DFG_LUT_DEFAULT, SHADER_STAGE_PIXEL );
            renderableEntities->bind( cmdList, CBUFFER_INDEX_LIGHTBUFFER, SHADER_STAGE_PIXEL );

            // Bind Output Buffers
            auto colorBuffer = renderPipelineResources->getRenderTarget( passData.output[0] );
            auto velocityBuffer = renderPipelineResources->getRenderTarget( passData.output[1] );

            auto depthBuffer = renderPipelineResources->getRenderTarget( passData.input[0] );
            auto csmShadowMap = renderPipelineResources->getRenderTarget( passData.input[1] );

            RenderTarget* renderTargets[2] = {
                colorBuffer,
                velocityBuffer
            };

            cmdList->bindRenderTargetsCmd( renderTargets, depthBuffer, 2 );

            csmShadowMap->bind( cmdList, TEXTURE_SLOT_INDEX_CSM_TEST, SHADER_STAGE_PIXEL );

            // Bind buffer
            glm::uvec2 rtDimensions;

            auto& pipelineDimensions = renderPipelineResources->getActiveViewportGeometry();
            cmdList->setViewportCmd( pipelineDimensions );

            rtDimensions.x = pipelineDimensions.Width;
            rtDimensions.y = pipelineDimensions.Height;

            auto rtBufferData = renderPipelineResources->getBuffer( passData.buffers[2] );
            rtBufferData->updateAsynchronous( cmdList, &rtDimensions, sizeof( glm::uvec2 ) );
            rtBufferData->bind( cmdList, 2, SHADER_STAGE_PIXEL );

            // Get Constant Buffer 
            auto modelMatrixBuffer = renderPipelineResources->getBuffer( passData.buffers[1] );
            modelMatrixBuffer->bind( cmdList, CBUFFER_INDEX_MATRICES, SHADER_STAGE_VERTEX );

            // Bind Camera Buffer
            auto cameraCbuffer = renderPipelineResources->getBuffer( passData.buffers[3] );
            auto passCamera = renderPipelineResources->getActiveCamera();
            cameraCbuffer->updateAsynchronous( cmdList, &passCamera, sizeof( Camera::Data ) );
            cameraCbuffer->bind( cmdList, 0 );

            auto atmosphereBuffer = renderPipelineResources->getBuffer( passData.buffers[4] );
            atmosphereBuffer->updateAsynchronous( cmdList, atmosphereData, sizeof( AtmosphereModule::Parameters ) );
            atmosphereBuffer->bind( cmdList, CBUFFER_INDEX_ATMOSPHERE );

            // Bind Samplers
            auto matInputSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            auto shadowSampler = renderPipelineResources->getSampler( passData.samplers[1] );
            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[2] );

            // Material input sampler
            for ( uint32_t i = 0; i < 8; i++ ) {
                matInputSampler->bind( cmdList, i );
            }

            //matInputSampler->bind( cmdList, 17 );

            bilinearSampler->bind( cmdList, 8 );
            bilinearSampler->bind( cmdList, 10 );
            bilinearSampler->bind( cmdList, 12 );
            shadowSampler->bind( cmdList, 15 );

            // Atmosphere sampler bind (if using atmospheric scattering)
            bilinearSampler->bind( cmdList, TEXTURE_SLOT_INDEX_ATMOSPHERE_SCATTERING );
            bilinearSampler->bind( cmdList, TEXTURE_SLOT_INDEX_ATMOSPHERE_TRANSMITTANCE );

            // Render opaque geometry
            int cmdCount = 0;
            auto* opaqueBucketList = renderPipelineResources->getLayerBucket( DrawCommandKey::Layer::LAYER_WORLD, DrawCommandKey::WORLD_VIEWPORT_LAYER_DEFAULT, cmdCount );

            glm::mat4x4* previousModelMatrix = nullptr;
            for ( int i = 0; i < cmdCount; i++ ) {
                const auto& drawCmd = opaqueBucketList[i];
                drawCmd.vao->bind( cmdList );

                if ( drawCmd.modelMatrix != previousModelMatrix ) {
                    modelMatrixBuffer->updateAsynchronous( cmdList, drawCmd.modelMatrix, sizeof( glm::mat4x4 ) );
                    previousModelMatrix = drawCmd.modelMatrix;
                }

                if ( !isRenderingProbe )
                    drawCmd.material->bind( cmdList );
                else
                    drawCmd.material->bindForProbeRendering( cmdList );

                cmdList->drawIndexedCmd( drawCmd.indiceBufferCount, drawCmd.indiceBufferOffset );
            }

            cmdList->bindBackbufferCmd();
            lightIndexBuffer->unbind( cmdList );
            csmShadowMap->unbind( cmdList );
        }
    );

    return -1;
}

FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( WorldLightPass, [=]( RenderPipeline* renderPipeline ) { return AddOpaqueLightPass( renderPipeline, false, false ); } )
FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( WorldLightMSAAPass, [=]( RenderPipeline* renderPipeline ) { return AddOpaqueLightPass( renderPipeline, true, false ); } )

FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( WorldLightProbePass, [=]( RenderPipeline* renderPipeline ) { return AddOpaqueLightPass( renderPipeline, false, true ); } )
FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( WorldLightProbeMSAAPass, [=]( RenderPipeline* renderPipeline ) { return AddOpaqueLightPass( renderPipeline, true, true ); } )
