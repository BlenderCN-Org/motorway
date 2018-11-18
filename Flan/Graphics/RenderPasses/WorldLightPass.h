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
    struct InstanceBuffer
    {
        glm::mat4x4 modelMatrix[512];
        float lodDitheringAlpha;
        uint32_t __PADDING__[3];
    };

    struct PassBuffer
    {
        glm::uvec2 backbufferDimension;
        float      brushRadius;
        uint32_t   toggleBrush;
        glm::vec3  mouseCoordinates;
        uint32_t   __PADDING__;
        glm::vec3  brushColor;
        uint32_t   __PADDING2__;
    };

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

            // Thin GBuffer RT
            RenderPassTextureDesc thinGBufferRenderTargetDesc = {};
            thinGBufferRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            thinGBufferRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
            thinGBufferRenderTargetDesc.description.depth = 1;
            thinGBufferRenderTargetDesc.description.mipCount = 1;
            thinGBufferRenderTargetDesc.description.arraySize = 1;
            thinGBufferRenderTargetDesc.useGlobalDimensions = true;
            thinGBufferRenderTargetDesc.useGlobalMultisamplingState = enableMSAA;
            thinGBufferRenderTargetDesc.initialState = RenderPassTextureDesc::CLEAR;

            passData.output[2] = renderPipelineBuilder->allocateTexture( thinGBufferRenderTargetDesc );

            renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "ThinGBufferRT" ), passData.output[2] );
            
            // Read Depth Buffer
            passData.input[0] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainDepthRT" ) );
            passData.input[1] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "CascadedShadowMappingShadowMap" ) );

            // Read Light Index Buffer
            passData.buffers[0] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "ForwardPlusLightIndexBuffer" ) );

            // Constant Buffer
            BufferDesc passBuffer = {};
            passBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            passBuffer.Size = sizeof( InstanceBuffer );

            passData.buffers[1] = renderPipelineBuilder->allocateBuffer( passBuffer );

            BufferDesc rtDimensionBuffer;
            rtDimensionBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            rtDimensionBuffer.Size = sizeof( PassBuffer );

            passData.buffers[2] = renderPipelineBuilder->allocateBuffer( rtDimensionBuffer );

            BufferDesc cameraBuffer = {};
            cameraBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            cameraBuffer.Size = sizeof( Camera::Data );

            passData.buffers[3] = renderPipelineBuilder->allocateBuffer( cameraBuffer );

            BufferDesc atmosphereBuffer = {};
            atmosphereBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            atmosphereBuffer.Size = sizeof( AtmosphereModule::Parameters );

            passData.buffers[4] = renderPipelineBuilder->allocateBuffer( atmosphereBuffer );

            BufferDesc terrainStreamingBuffer = {};
            terrainStreamingBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            terrainStreamingBuffer.Size = sizeof( TerrainStreaming::terrainMaterialStreaming );

            passData.buffers[5] = renderPipelineBuilder->allocateBuffer( terrainStreamingBuffer );
            
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
            shadowComparisonSamplerDesc.addressU = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_BORDER;
            shadowComparisonSamplerDesc.addressV = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_BORDER;
            shadowComparisonSamplerDesc.addressW = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_BORDER;
            shadowComparisonSamplerDesc.filter = eSamplerFilter::SAMPLER_FILTER_COMPARISON_TRILINEAR;
            shadowComparisonSamplerDesc.comparisonFunction = eComparisonFunction::COMPARISON_FUNCTION_LEQUAL;

            passData.samplers[1] = renderPipelineBuilder->allocateSampler( shadowComparisonSamplerDesc );

            SamplerDesc bilinearSamplerDesc;
            bilinearSamplerDesc.addressU = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = flan::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.samplers[2] = renderPipelineBuilder->allocateSampler( bilinearSamplerDesc );

            SamplerDesc matDisplacementSamplerDesc;
            matDisplacementSamplerDesc.addressU = eSamplerAddress::SAMPLER_ADDRESS_WRAP;
            matDisplacementSamplerDesc.addressV = eSamplerAddress::SAMPLER_ADDRESS_WRAP;
            matDisplacementSamplerDesc.addressW = eSamplerAddress::SAMPLER_ADDRESS_WRAP;
            matDisplacementSamplerDesc.filter = eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.samplers[3] = renderPipelineBuilder->allocateSampler( matDisplacementSamplerDesc );
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
            auto thinGBuffer = renderPipelineResources->getRenderTarget( passData.output[2] );

            auto depthBuffer = renderPipelineResources->getRenderTarget( passData.input[0] );
            auto csmShadowMap = renderPipelineResources->getRenderTarget( passData.input[1] );

            RenderTarget* renderTargets[3] = {
                colorBuffer,
                velocityBuffer,
                thinGBuffer
            };

            cmdList->bindRenderTargetsCmd( renderTargets, depthBuffer, 3 );

            csmShadowMap->bind( cmdList, TEXTURE_SLOT_INDEX_CSM_TEST, SHADER_STAGE_PIXEL );

            // Bind buffer
            auto& pipelineDimensions = renderPipelineResources->getActiveViewportGeometry();
            cmdList->setViewportCmd( pipelineDimensions );

            PassBuffer passBuffer;
            passBuffer.backbufferDimension.x = pipelineDimensions.Width;
            passBuffer.backbufferDimension.y = pipelineDimensions.Height;

            FLAN_IMPORT_VAR_PTR( dev_TerrainMousePosition, glm::vec3 )
            FLAN_IMPORT_VAR_PTR( g_TerrainEditorEditionRadius, int )
            FLAN_IMPORT_VAR_PTR( panelId, int )
            FLAN_IMPORT_VAR_PTR( IsDevMenuVisible, bool )
                
            passBuffer.mouseCoordinates = *dev_TerrainMousePosition;
            passBuffer.brushRadius = *g_TerrainEditorEditionRadius;
            passBuffer.toggleBrush = ( *panelId == 2 && *IsDevMenuVisible ) ? 1 : 0;

            auto rtBufferData = renderPipelineResources->getBuffer( passData.buffers[2] );
            rtBufferData->updateAsynchronous( cmdList, &passBuffer, sizeof( PassBuffer ) );
            rtBufferData->bind( cmdList, 2, SHADER_STAGE_PIXEL );

            // Get Constant Buffer 
            auto modelMatrixBuffer = renderPipelineResources->getBuffer( passData.buffers[1] );
            modelMatrixBuffer->bind( cmdList, CBUFFER_INDEX_MATRICES, SHADER_STAGE_ALL );

            // Bind Camera Buffer
            auto cameraCbuffer = renderPipelineResources->getBuffer( passData.buffers[3] );
            auto passCamera = renderPipelineResources->getActiveCamera();
            cameraCbuffer->updateAsynchronous( cmdList, &passCamera, sizeof( Camera::Data ) );
            cameraCbuffer->bind( cmdList, 0 );

            auto atmosphereBuffer = renderPipelineResources->getBuffer( passData.buffers[4] );
            atmosphereBuffer->updateAsynchronous( cmdList, atmosphereData, sizeof( AtmosphereModule::Parameters ) );
            atmosphereBuffer->bind( cmdList, CBUFFER_INDEX_ATMOSPHERE );

            auto terrrainStreamingData = renderPipelineResources->getWellKnownImportedResource<TerrainStreaming>();
            auto terrainStreamingBuffer = renderPipelineResources->getBuffer( passData.buffers[5] );
            terrainStreamingBuffer->updateAsynchronous( cmdList, &terrrainStreamingData->terrainMaterialStreaming, sizeof( TerrainStreaming::terrainMaterialStreaming ) );
            terrainStreamingBuffer->bind( cmdList, 7, SHADER_STAGE_PIXEL | SHADER_STAGE_TESSELATION_CONTROL | SHADER_STAGE_TESSELATION_EVALUATION );

            terrrainStreamingData->baseColorStreamed->bind( cmdList, 6, SHADER_STAGE_PIXEL | SHADER_STAGE_TESSELATION_CONTROL | SHADER_STAGE_TESSELATION_EVALUATION );
            terrrainStreamingData->normalStreamed->bind( cmdList, 7, SHADER_STAGE_PIXEL );

            // Bind Samplers
            auto matInputSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            auto shadowSampler = renderPipelineResources->getSampler( passData.samplers[1] );
            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[2] );
            auto displacementSampler = renderPipelineResources->getSampler( passData.samplers[3] );

            // Material input sampler
            for ( uint32_t i = 0; i < 8; i++ ) {
                matInputSampler->bind( cmdList, i );
            }

            //matInputSampler->bind( cmdList, 17 );

            bilinearSampler->bind( cmdList, 8 );
            displacementSampler->bind( cmdList, 9 );
            bilinearSampler->bind( cmdList, 10 );
            bilinearSampler->bind( cmdList, 12 );
            shadowSampler->bind( cmdList, 15 );

            // Atmosphere sampler bind (if using atmospheric scattering)
            bilinearSampler->bind( cmdList, TEXTURE_SLOT_INDEX_ATMOSPHERE_SCATTERING );
            bilinearSampler->bind( cmdList, TEXTURE_SLOT_INDEX_ATMOSPHERE_TRANSMITTANCE );

            // Render opaque geometry
            int cmdCount = 0;
            auto* opaqueBucketList = renderPipelineResources->getLayerBucket( DrawCommandKey::Layer::LAYER_WORLD, DrawCommandKey::WORLD_VIEWPORT_LAYER_DEFAULT, cmdCount );

            InstanceBuffer instance;
            glm::mat4x4* previousModelMatrix = nullptr;
            for ( int i = 0; i < cmdCount; i++ ) {
                const auto& drawCmd = opaqueBucketList[i];
                drawCmd.vao->bind( cmdList );

                if ( drawCmd.instanceCount > 1 ) {
                    memcpy( instance.modelMatrix, drawCmd.modelMatrix, drawCmd.instanceCount * sizeof( glm::mat4x4 ) );
                    previousModelMatrix = nullptr;
                } else {
                    if ( drawCmd.modelMatrix != previousModelMatrix ) {
                        instance.modelMatrix[0] = *drawCmd.modelMatrix;
                        previousModelMatrix = drawCmd.modelMatrix;
                    }
                }
                    
                instance.lodDitheringAlpha = drawCmd.alphaDitheringValue;
                modelMatrixBuffer->updateAsynchronous( cmdList, &instance, sizeof( InstanceBuffer ) );
                  
                if ( drawCmd.instanceCount > 1 ) {
                    if ( !isRenderingProbe )
                        drawCmd.material->bindInstanced( cmdList );
                    else
                        drawCmd.material->bindInstancedForProbeRendering( cmdList );

                    cmdList->drawInstancedIndexedCmd( drawCmd.indiceBufferCount, drawCmd.indiceBufferOffset, drawCmd.instanceCount );
                } else {
                    if ( !isRenderingProbe )
                        drawCmd.material->bind( cmdList );
                    else
                        drawCmd.material->bindForProbeRendering( cmdList );

                    cmdList->drawIndexedCmd( drawCmd.indiceBufferCount, drawCmd.indiceBufferOffset );
                }
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
