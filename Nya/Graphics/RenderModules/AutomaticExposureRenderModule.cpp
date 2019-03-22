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
#include "AutomaticExposureRenderModule.h"

#include <Graphics/RenderPipeline.h>
#include <Graphics/ShaderCache.h>
#include <Graphics/GraphicsAssetCache.h>

#include <Rendering/ImageFormat.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#include <Framework/Cameras/Camera.h>

#include "AtmosphereSettings.h"
#include "AtmosphereConstants.h"

using namespace nya::rendering;

AutomaticExposureModule::AutomaticExposureModule()
    : autoExposureBuffer{ nullptr }
    , exposureTarget( 0 )
    , binComputePso( nullptr )
    , mergeHistoPso( nullptr )
    , tileHistoComputePso( nullptr )
{
    // Default Exposure Infos
    autoExposureInfos.EngineLuminanceFactor = 1.0f;
    autoExposureInfos.TargetLuminance = 0.5f;
    autoExposureInfos.MinLuminanceLDR = 0.0f;
    autoExposureInfos.MaxLuminanceLDR = 1.0f;
    autoExposureInfos.MiddleGreyLuminanceLDR = 1.0f;
    autoExposureInfos.EV = 0.0f;
    autoExposureInfos.Fstop = 0.0f;
    autoExposureInfos.PeakHistogramValue = 0;

    // Default Camera Exposure Settings
    parameters._white_level = 1.00f;
    parameters._clip_shadows = 0.00f;
    parameters._clip_highlights = 1.00f;
    parameters._EV = 0.0f;
    parameters._fstop_bias = 0.0f;
    parameters._reference_camera_fps = 30.0f;
    parameters._adapt_min_luminance = 0.03f;
    parameters._adapt_max_luminance = 2000.0f;
    parameters._adapt_speed_up = 0.99f;
    parameters._adapt_speed_down = 0.99f;
}

AutomaticExposureModule::~AutomaticExposureModule()
{

}

ResHandle_t AutomaticExposureModule::computeExposure( RenderPipeline* renderPipeline, ResHandle_t lightRenderTarget, const nyaVec2f& screenSize )
{
    // Update persistent buffer pointers
    renderPipeline->importPersistentBuffer( NYA_STRING_HASH( "AutoExposure/ReadBuffer" ), autoExposureBuffer[exposureTarget] );
    renderPipeline->importPersistentBuffer( NYA_STRING_HASH( "AutoExposure/WriteBuffer" ), autoExposureBuffer[( exposureTarget == 0 ) ? 1 : 0] );

    const nyaVec2u screenDimensions = nyaVec2u( screenSize );

    auto histoBins = addBinComputePass( renderPipeline, lightRenderTarget, screenDimensions );
    auto mergedBuffer = addHistogramMergePass( renderPipeline, histoBins, screenDimensions );
    auto autoExposureInfos = addExposureComputePass( renderPipeline, mergedBuffer );

    // Swap buffers
    exposureTarget = ( ++exposureTarget % 2 );

    return autoExposureInfos;
}

void AutomaticExposureModule::destroy( RenderDevice* renderDevice )
{
    renderDevice->destroyBuffer( autoExposureBuffer[0] );
    renderDevice->destroyBuffer( autoExposureBuffer[1] );

    renderDevice->destroyPipelineState( binComputePso );
    renderDevice->destroyPipelineState( mergeHistoPso );
    renderDevice->destroyPipelineState( tileHistoComputePso );
}

void AutomaticExposureModule::loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache )
{
    // Create Ping-Pong Auto Exposure Buffers
    BufferDesc autoExposureBufferDescription;
    autoExposureBufferDescription.type = BufferDesc::STRUCTURED_BUFFER;
    autoExposureBufferDescription.viewFormat = IMAGE_FORMAT_R32_UINT;
    autoExposureBufferDescription.singleElementSize = 1;
    autoExposureBufferDescription.size = sizeof( autoExposureInfos );
    autoExposureBufferDescription.stride = 1;

    for ( int i = 0; i < 2; i++ ) {
        autoExposureBuffer[i] = renderDevice->createBuffer( autoExposureBufferDescription, &autoExposureInfos );
    }

    PipelineStateDesc psoDesc = {};

    psoDesc.computeShader = shaderCache->getOrUploadStage( "AutoExposure/BinCompute", SHADER_STAGE_COMPUTE );
    binComputePso = renderDevice->createPipelineState( psoDesc );

    psoDesc.computeShader = shaderCache->getOrUploadStage( "AutoExposure/HistogramMerge", SHADER_STAGE_COMPUTE );
    mergeHistoPso = renderDevice->createPipelineState( psoDesc );
    
    psoDesc.computeShader = shaderCache->getOrUploadStage( "AutoExposure/TileHistogramCompute", SHADER_STAGE_COMPUTE );
    tileHistoComputePso = renderDevice->createPipelineState( psoDesc );
}

MutableResHandle_t AutomaticExposureModule::addBinComputePass( RenderPipeline* renderPipeline, const ResHandle_t inputRenderTarget, const nyaVec2u& screenSize )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t output;

        ResHandle_t screenInfosBuffer;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Automatic Exposure Bin Compute Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            passData.input = renderPipelineBuilder.readRenderTarget( inputRenderTarget );

            // Per Tile Histogram
            const uint32_t bufferDimension = ( static_cast<uint32_t>( screenSize.y + 3 ) >> 2 ) * 128;
            const auto perTileHistogramBufferSize = sizeof( unsigned int ) * static_cast<std::size_t>( bufferDimension );

            // Create Per-Tile Histogram Buffer
            BufferDesc perTileHistogramsBufferDescription;
            perTileHistogramsBufferDescription.type = BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER;
            perTileHistogramsBufferDescription.viewFormat = IMAGE_FORMAT_R32_UINT;
            perTileHistogramsBufferDescription.size = perTileHistogramBufferSize;
            perTileHistogramsBufferDescription.singleElementSize = sizeof( unsigned int );
            perTileHistogramsBufferDescription.stride = static_cast<std::uint32_t>( bufferDimension );

            passData.output = renderPipelineBuilder.allocateBuffer( perTileHistogramsBufferDescription, SHADER_STAGE_COMPUTE );

            BufferDesc screenInfosBuffer;
            screenInfosBuffer.type = BufferDesc::CONSTANT_BUFFER;
            screenInfosBuffer.size = sizeof( nyaVec4f );

            passData.screenInfosBuffer = renderPipelineBuilder.allocateBuffer( screenInfosBuffer, SHADER_STAGE_COMPUTE );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            Buffer* outputBuffer = renderPipelineResources.getBuffer( passData.output );

            Buffer* screenSizeBuffer = renderPipelineResources.getBuffer( passData.screenInfosBuffer );
            cmdList->updateBuffer( screenSizeBuffer, &screenSize, sizeof( nyaVec2u ) );

            ResourceListDesc resListDesc = {};
            resListDesc.uavBuffers[0] = { 0, SHADER_STAGE_COMPUTE, outputBuffer };
            resListDesc.constantBuffers[0] = { 0, SHADER_STAGE_COMPUTE, screenSizeBuffer };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            // RenderPass
            RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );

            RenderPassDesc passDesc = {};
            passDesc.attachements[0].renderTarget = inputTarget;
            passDesc.attachements[0].stageBind = SHADER_STAGE_COMPUTE;
            passDesc.attachements[0].bindMode = RenderPassDesc::READ;
            passDesc.attachements[0].targetState = RenderPassDesc::DONT_CARE;

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            cmdList->bindPipelineState( binComputePso );

            cmdList->dispatchCompute( 1u, static_cast<unsigned int>( ceilf( screenSize.y / 4.0f ) ), 1u );

            renderDevice->destroyRenderPass( renderPass );
        }
    );

    return passData.output;
}

MutableResHandle_t AutomaticExposureModule::addHistogramMergePass( RenderPipeline* renderPipeline, const ResHandle_t perTileHistoBuffer, const nyaVec2u& screenSize )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t output;

        ResHandle_t screenInfosBuffer;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Automatic Exposure Histogram Merge Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            passData.input = renderPipelineBuilder.readBuffer( perTileHistoBuffer );

            const unsigned int backbufferHistogramSize = sizeof( unsigned int ) * 128;

            // Create Final Histogram Buffer
            BufferDesc backbufferHistogramsBufferDescription;
            backbufferHistogramsBufferDescription.type = BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER;
            backbufferHistogramsBufferDescription.viewFormat = IMAGE_FORMAT_R32_UINT;
            backbufferHistogramsBufferDescription.size = backbufferHistogramSize;
            backbufferHistogramsBufferDescription.singleElementSize = sizeof( unsigned int );
            backbufferHistogramsBufferDescription.stride = 128;

            passData.output = renderPipelineBuilder.allocateBuffer( backbufferHistogramsBufferDescription, SHADER_STAGE_COMPUTE );

            BufferDesc rtDimensionBuffer;
            rtDimensionBuffer.type = BufferDesc::CONSTANT_BUFFER;
            rtDimensionBuffer.size = sizeof( nyaVec4f );

            passData.screenInfosBuffer = renderPipelineBuilder.allocateBuffer( rtDimensionBuffer, SHADER_STAGE_COMPUTE );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            Buffer* inputBuffer = renderPipelineResources.getBuffer( passData.input );
            Buffer* outputBuffer = renderPipelineResources.getBuffer( passData.output );

            Buffer* screenSizeBuffer = renderPipelineResources.getBuffer( passData.screenInfosBuffer );
            cmdList->updateBuffer( screenSizeBuffer, &screenSize, sizeof( nyaVec2u ) );

            ResourceListDesc resListDesc = {};
            resListDesc.uavBuffers[0] = { 0, SHADER_STAGE_COMPUTE, outputBuffer };
            resListDesc.buffers[0] = { 0, SHADER_STAGE_COMPUTE, inputBuffer };
            resListDesc.constantBuffers[0] = { 0, SHADER_STAGE_COMPUTE, screenSizeBuffer };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            cmdList->bindPipelineState( mergeHistoPso );

            cmdList->dispatchCompute( 128u, 1u, 1u );
        }
    );

    return passData.output;
}

ResHandle_t AutomaticExposureModule::addExposureComputePass( RenderPipeline* renderPipeline, const ResHandle_t mergedHistoBuffer )
{
    struct PassData {
        ResHandle_t input;

        ResHandle_t output;
        ResHandle_t lastFrameOutput;

        ResHandle_t parametersBuffer;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Automatic Exposure Compute Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            passData.input = renderPipelineBuilder.readBuffer( mergedHistoBuffer );

            passData.output = renderPipelineBuilder.retrievePersistentBuffer( NYA_STRING_HASH( "AutoExposure/WriteBuffer" ) );
            passData.lastFrameOutput = renderPipelineBuilder.retrievePersistentBuffer( NYA_STRING_HASH( "AutoExposure/ReadBuffer" ) );

            BufferDesc parametersBufferDesc;
            parametersBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            parametersBufferDesc.size = sizeof( parameters );

            passData.parametersBuffer = renderPipelineBuilder.allocateBuffer( parametersBufferDesc, SHADER_STAGE_COMPUTE );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            Buffer* inputBuffer = renderPipelineResources.getBuffer( passData.input );

            Buffer* parametersBuffer = renderPipelineResources.getBuffer( passData.parametersBuffer );
            parameters._delta_time = renderPipelineResources.getDeltaTime() / 1000.0f;
            cmdList->updateBuffer( parametersBuffer, &parameters, sizeof( parameters ) );

            // Finally, compute auto exposure infos
            Buffer* bufferToRead = renderPipelineResources.getPersistentBuffer( passData.lastFrameOutput );
            Buffer* bufferToWrite = renderPipelineResources.getPersistentBuffer( passData.output );

            ResourceListDesc resListDesc = {};
            resListDesc.uavBuffers[0] = { 0, SHADER_STAGE_COMPUTE, bufferToWrite };

            resListDesc.buffers[0] = { 0, SHADER_STAGE_COMPUTE, bufferToRead };
            resListDesc.buffers[1] = { 1, SHADER_STAGE_COMPUTE, inputBuffer };

            resListDesc.constantBuffers[0] = { 0, SHADER_STAGE_COMPUTE, parametersBuffer };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            cmdList->bindPipelineState( tileHistoComputePso );

            cmdList->dispatchCompute( 1u, 1u, 1u );
        }
    );

    return passData.output;
}
