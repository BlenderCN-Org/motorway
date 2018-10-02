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

#include "Shared.h"
#include "AutomaticExposureModule.h"

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#include <Core/Factory.h>

AutomaticExposureModule::AutomaticExposureModule()
    : exposureTarget( 0 )
{
    // Default Exposure Infos
    autoExposureInfos.EngineLuminanceFactor = 0.70f;
    autoExposureInfos.TargetLuminance = 0.5f;
    autoExposureInfos.MinLuminanceLDR = 0.0f;
    autoExposureInfos.MaxLuminanceLDR = 1.0f;
    autoExposureInfos.MiddleGreyLuminanceLDR = 1.0f;
    autoExposureInfos.EV = 0.0f;
    autoExposureInfos.Fstop = 0.0f;
    autoExposureInfos.PeakHistogramValue = 0;

    // Default Camera Exposure Settings
    parameters._white_level = 1.00f;
    parameters._clip_shadows = 0.00f;				// (0.0) Shadow cropping in histogram (first buckets will be ignored, leading to brighter image)
    parameters._clip_highlights = 0.10f;			    // (1.0) Highlights cropping in histogram (last buckets will be ignored, leading to darker image)
    parameters._EV = 0.0f;							// (0.0) Your typical EV setting
    parameters._fstop_bias = 0.0f;					// (0.0) F-stop number bias to override automatic computation (NOTE: This will NOT change exposure, only the F number)
    parameters._reference_camera_fps = 30.0f;		// (30.0) Default camera at 30 FPS
    parameters._adapt_min_luminance = 0.03f;		// (0.03) Prevents the auto-exposure to adapt to luminances lower than this
    parameters._adapt_max_luminance = 2000.0f;		// (2000.0) Prevents the auto-exposure to adapt to luminances higher than this
    parameters._adapt_speed_up = 0.99f;	            // (0.99) Adaptation speed from low to high luminances
    parameters._adapt_speed_down = 0.99f;	        // (0.99) Adaptation speed from high to low luminances
}

AutomaticExposureModule::~AutomaticExposureModule()
{

}

fnPipelineMutableResHandle_t AutomaticExposureModule::addExposureComputePass( RenderPipeline* renderPipeline )
{
    renderPipeline->addPipelineSetupPass(
        [&]( RenderPipeline* renderPipeline, RenderPipelineBuilder* renderPipelineBuilder ) {
            auto inputRenderTarget = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) );

            auto histogrammBuffer = addBinComputePass( renderPipeline, inputRenderTarget );
            auto mergedBuffer = addHistogramMergePass( renderPipeline, histogrammBuffer );
            autoExposureComputePass( renderPipeline, mergedBuffer );

            currentExposureBuffer.exposureBuffer = &autoExposureBuffer[exposureTarget];
            renderPipeline->importWellKnownResource( &currentExposureBuffer );
        }
    );

    return -1;
}

void AutomaticExposureModule::loadCachedResources( RenderDevice* renderDevice, GraphicsAssetManager* graphicsAssetManager )
{
    // Create Ping-Pong Auto Exposure Buffers
    BufferDesc autoExposureBufferDescription;
    autoExposureBufferDescription.Type = BufferDesc::STRUCTURED_BUFFER;
    autoExposureBufferDescription.ViewFormat = IMAGE_FORMAT_R32_UINT;
    autoExposureBufferDescription.SingleElementSize = 1;
    autoExposureBufferDescription.Size = sizeof( autoExposureInfos );
    autoExposureBufferDescription.Stride = 1;

    for ( int i = 0; i < 2; i++ ) {
        autoExposureBuffer[i].create( renderDevice, autoExposureBufferDescription );
    }

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent( FLAN_STRING_HASH( "AutoExposurePass" ),
                                                                        [=]( RenderPipeline* renderPipeline ) {
                                                                            return addExposureComputePass( renderPipeline );
                                                                        } );
}

fnPipelineMutableResHandle_t AutomaticExposureModule::addBinComputePass( RenderPipeline* renderPipeline, const fnPipelineMutableResHandle_t inputRenderTarget )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Automatic Exposure Bin Compute Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            // Read Render Target to extract bright parts from
            passData.input[0] = renderPipelineBuilder->readRenderTarget( inputRenderTarget );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "TileHistogramComputePass" );
            passPipelineState.computeStage = FLAN_STRING( "TileHistogramCompute" );
            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            // Per Tile Histogram
            auto viewport = renderPipelineBuilder->getActiveViewport();
            auto bufferDimension = ( ( viewport.Height + 3 ) >> 2 ) * 128;
            const auto perTileHistogramBufferSize = sizeof( unsigned int ) * static_cast<std::size_t>( bufferDimension );

            // Create Per-Tile Histogram Buffer
            BufferDesc perTileHistogramsBufferDescription;
            perTileHistogramsBufferDescription.Type = BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER;
            perTileHistogramsBufferDescription.ViewFormat = IMAGE_FORMAT_R32_UINT;
            perTileHistogramsBufferDescription.Size = perTileHistogramBufferSize;
            perTileHistogramsBufferDescription.SingleElementSize = sizeof( unsigned int );
            perTileHistogramsBufferDescription.Stride = static_cast<std::size_t>( bufferDimension ); 

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( perTileHistogramsBufferDescription );

            BufferDesc rtDimensionBuffer;
            rtDimensionBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            rtDimensionBuffer.Size = sizeof( glm::uvec4 );

            passData.buffers[1] = renderPipelineBuilder->allocateBuffer( rtDimensionBuffer );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Bind Input Target
            auto inputRenderTarget = renderPipelineResources->getRenderTarget( passData.input[0] );
            const auto& inputRTDesc = inputRenderTarget->getDescription();
            const auto inputRenderTargetHeight = inputRTDesc.height;

            // Compute bins
            // Bind Resources
            auto binsBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            binsBuffer->bind( cmdList, 0, SHADER_STAGE_COMPUTE );

            // Bind buffer
            glm::uvec2 rtDimensions;

            auto& pipelineDimensions = renderPipelineResources->getActiveViewport();
            cmdList->setViewportCmd( pipelineDimensions );

            rtDimensions.x = pipelineDimensions.Width;
            rtDimensions.y = pipelineDimensions.Height;

            auto rtBufferData = renderPipelineResources->getBuffer( passData.buffers[1] );
            rtBufferData->updateAsynchronous( cmdList, &rtDimensions, sizeof( glm::uvec2 ) );
            rtBufferData->bind( cmdList, 2, SHADER_STAGE_COMPUTE );

            // Bind Main RenderTarget
            inputRenderTarget->bind( cmdList, 0, SHADER_STAGE_COMPUTE );

            // Start GPU Compute
            cmdList->dispatchComputeCmd( 1, static_cast<unsigned int>( ceilf( inputRenderTargetHeight / 4.0f ) ), 1 );

            // Unbind resources (better be safe with compute shaders!)
            inputRenderTarget->unbind( cmdList );
            binsBuffer->unbind( cmdList );
        }
    );

    return RenderPass.buffers[0];
}

fnPipelineMutableResHandle_t AutomaticExposureModule::addHistogramMergePass( RenderPipeline* renderPipeline, const fnPipelineMutableResHandle_t perTileHistoBuffer )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Histogram Merge Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "MergeHistogramPass" );
            passPipelineState.computeStage = FLAN_STRING( "MergeHistogram" );

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            const unsigned int backbufferHistogramSize = sizeof( unsigned int ) * 128;

            // Create Final Histogram Buffer
            BufferDesc backbufferHistogramsBufferDescription;
            backbufferHistogramsBufferDescription.Type = BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER;
            backbufferHistogramsBufferDescription.ViewFormat = IMAGE_FORMAT_R32_UINT;
            backbufferHistogramsBufferDescription.Size = backbufferHistogramSize;
            backbufferHistogramsBufferDescription.SingleElementSize = sizeof( unsigned int );
            backbufferHistogramsBufferDescription.Stride = 128;

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( backbufferHistogramsBufferDescription );
            passData.buffers[1] = renderPipelineBuilder->readBuffer( perTileHistoBuffer );

            BufferDesc rtDimensionBuffer;
            rtDimensionBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            rtDimensionBuffer.Size = sizeof( glm::uvec4 );

            passData.buffers[2] = renderPipelineBuilder->allocateBuffer( rtDimensionBuffer );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Bind buffer
            glm::uvec2 rtDimensions;

            auto& pipelineDimensions = renderPipelineResources->getActiveViewport();
            cmdList->setViewportCmd( pipelineDimensions );

            rtDimensions.x = pipelineDimensions.Width;
            rtDimensions.y = pipelineDimensions.Height;

            auto rtBufferData = renderPipelineResources->getBuffer( passData.buffers[2] );
            rtBufferData->updateAsynchronous( cmdList, &rtDimensions, sizeof( glm::uvec2 ) );
            rtBufferData->bind( cmdList, 2, SHADER_STAGE_COMPUTE );

            // Merge Stuff
            auto histogrammBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            auto perTileBuffer = renderPipelineResources->getBuffer( passData.buffers[1] );

            histogrammBuffer->bind( cmdList, 0, SHADER_STAGE_COMPUTE );
            perTileBuffer->bindReadOnly( cmdList, 1, SHADER_STAGE_COMPUTE );

            // Start GPU Compute
            cmdList->dispatchComputeCmd( 128, 1, 1 );
            
            // Unbind resources (better be safe with compute shaders!)
            histogrammBuffer->unbind( cmdList );
            perTileBuffer->unbind( cmdList );
        }
    );

    return RenderPass.buffers[0];
}

void AutomaticExposureModule::autoExposureComputePass( RenderPipeline* renderPipeline, const fnPipelineMutableResHandle_t mergedHistoBuffer )
{
    renderPipeline->addRenderPass(
        "Auto Exposure Compute Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "AutoExposurePass" );
            passPipelineState.computeStage = FLAN_STRING( "AutoExposureCompute" );

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            BufferDesc constantBufferDesc;
            constantBufferDesc.Type = BufferDesc::CONSTANT_BUFFER;
            constantBufferDesc.Size = sizeof( parameters );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( constantBufferDesc );
            passData.buffers[1] = renderPipelineBuilder->readBuffer( mergedHistoBuffer );

            BufferDesc worldInfoBufferDesc;
            worldInfoBufferDesc.Type = BufferDesc::CONSTANT_BUFFER;
            worldInfoBufferDesc.Size = sizeof( glm::vec4 );

            passData.buffers[2] = renderPipelineBuilder->allocateBuffer( worldInfoBufferDesc );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Finally, compute auto exposure infos
            Buffer* const bufferToRead = &autoExposureBuffer[exposureTarget];
            Buffer* const bufferToWrite = &autoExposureBuffer[( exposureTarget == 0 ) ? 1 : 0];

            bufferToWrite->bind( cmdList, 0, SHADER_STAGE_COMPUTE );

            auto mergedBuffer = renderPipelineResources->getBuffer( passData.buffers[1] );
            mergedBuffer->bindReadOnly( cmdList, 1, SHADER_STAGE_COMPUTE );
            bufferToRead->bindReadOnly( cmdList, 0, SHADER_STAGE_COMPUTE );

            auto autoExposureConstantBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            autoExposureConstantBuffer->updateAsynchronous( cmdList, &parameters, sizeof( parameters ) );
            autoExposureConstantBuffer->bindReadOnly( cmdList, 0, SHADER_STAGE_COMPUTE );

            const float timeDelta = renderPipelineResources->getTimeDelta();
            auto worldInfoBuffer = renderPipelineResources->getBuffer( passData.buffers[2] );
            worldInfoBuffer->updateAsynchronous( cmdList, &timeDelta, sizeof( float ) );
            worldInfoBuffer->bindReadOnly( cmdList, 2, SHADER_STAGE_COMPUTE );

            // Start GPU Compute
            cmdList->dispatchComputeCmd( 1, 1, 1 );
            
            // Unbind resources (better be safe with compute shaders!)
            mergedBuffer->unbind( cmdList );
            bufferToWrite->unbind( cmdList );
            bufferToRead->unbind( cmdList );

            // Flip flop buffers
            exposureTarget = ( exposureTarget == 0 ) ? 1 : 0;
        }
    );
}
