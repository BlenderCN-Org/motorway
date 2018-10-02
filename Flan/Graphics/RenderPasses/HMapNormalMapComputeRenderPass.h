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

#include <Shaders/Shared.h>
#include <Rendering/CommandList.h>

#include <Graphics/ComputeHelpers.h>

static fnPipelineResHandle_t AddHMapNormalMapComputePass( RenderPipeline* renderPipeline, Texture* heightmap )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "HMap NormalMap Compute",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "HMap Normal Map Compute" );
            passPipelineState.computeStage = FLAN_STRING( "HeightmapNormalGeneration" );

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            // Texture UAV
            auto& normalMapDesc = heightmap->getDescription();
            BufferDesc bufferDesc;
            bufferDesc.Type = BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D;
            bufferDesc.ViewFormat = IMAGE_FORMAT_R16G16B16A16_FLOAT;
            bufferDesc.Width = normalMapDesc.width;
            bufferDesc.Height = normalMapDesc.height;
            bufferDesc.Depth = 1;
            bufferDesc.MipCount = 1;

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( bufferDesc );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Bind Normal Map
            heightmap->bind( cmdList, 0, SHADER_STAGE_COMPUTE );

            auto& normalMapDesc = heightmap->getDescription();
            auto buffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            buffer->bind( cmdList, 0, SHADER_STAGE_COMPUTE );
            cmdList->dispatchComputeCmd( DispatchSize( 16, normalMapDesc.width ), DispatchSize( 16, normalMapDesc.height ), 1 );
            buffer->unbind( cmdList );

            heightmap->unbind( cmdList );
        }
    );

    return RenderPass.buffers[0];
}
