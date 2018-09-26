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

// Computes a compute shader dispatch size given a thread group size, and number of elements to process
uint32_t DispatchSize( uint32_t tgSize, uint32_t numElements )
{
    uint32_t dispatchSize = numElements / tgSize;
    dispatchSize += ( numElements % tgSize > 0 ) ? 1 : 0;
    return dispatchSize;
}

struct VMFResolveOutput
{
    static constexpr int MAX_MIP_COUNT = 16;

    int generatedMipCount;
    fnPipelineResHandle_t roughnessMipMaps[MAX_MIP_COUNT];
};

static VMFResolveOutput AddVMFMapComputePass( RenderPipeline* renderPipeline, Texture* normalMap, const float roughness = 0.05f )
{
    struct PassBuffer
    {
        float       inputTexWidth;
        float       inputTexHeight;
        float       outputTexWidth;
        float       outputTexHeight;
        uint32_t    mipLevel;
        float       roughness;
        float       scaleFactor;
        uint32_t    __PADDING__;
    };

    FLAN_IS_MEMORY_ALIGNED( 16, PassBuffer )

    auto RenderPass = renderPipeline->addRenderPass(
        "VMF Solver",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "VMF Solver" );
            passPipelineState.computeStage = FLAN_STRING( "VMFSolver" );

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            // Constant Buffer
            BufferDesc bufferDesc;
            bufferDesc.Type = BufferDesc::CONSTANT_BUFFER;
            bufferDesc.Size = sizeof( PassBuffer );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( bufferDesc );

            auto& normalMapDesc = normalMap->getDescription();
            auto width = normalMapDesc.width, height = normalMapDesc.height;
            for ( unsigned int mipLevel = 0; mipLevel < normalMapDesc.mipCount; mipLevel++ ) {
                // Texture UAV
                BufferDesc bufferDesc;
                bufferDesc.Type = BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D;
                bufferDesc.ViewFormat = IMAGE_FORMAT_R8_UNORM;
                bufferDesc.Width = width;
                bufferDesc.Height = height;
                bufferDesc.Depth = 1;
                bufferDesc.MipCount = 1;

                passData.buffers[( mipLevel + 1 )] = renderPipelineBuilder->allocateBuffer( bufferDesc );

                width >>= 1;
                height >>= 1;
            }
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Bind Normal Map
            normalMap->bind( cmdList, 0, SHADER_STAGE_COMPUTE );

            auto& normalMapDesc = normalMap->getDescription();

            auto buffer = renderPipelineResources->getBuffer( passData.buffers[0] );

            PassBuffer passBufferData;
            passBufferData.inputTexWidth = normalMapDesc.width;
            passBufferData.inputTexHeight = normalMapDesc.height;
            passBufferData.outputTexWidth = normalMapDesc.width;
            passBufferData.outputTexHeight = normalMapDesc.height;
            passBufferData.mipLevel = 0;
            passBufferData.roughness = roughness;
            passBufferData.scaleFactor = std::pow( 10.0f, 0.5f );
            buffer->updateAsynchronous( cmdList, &passBufferData, sizeof( PassBuffer ) );

            buffer->bind( cmdList, 0, SHADER_STAGE_COMPUTE );

            auto width = normalMapDesc.width, height = normalMapDesc.height;
            for ( unsigned int mipLevel = 0; mipLevel < normalMapDesc.mipCount; mipLevel++ ) {
                auto roughnessUAV = renderPipelineResources->getBuffer( passData.buffers[( mipLevel + 1 )] );

                passBufferData.mipLevel = mipLevel;
                passBufferData.outputTexWidth = width;
                passBufferData.outputTexHeight = height;
                buffer->updateAsynchronous( cmdList, &passBufferData, sizeof( PassBuffer ) );

                roughnessUAV->bind( cmdList, 0, SHADER_STAGE_COMPUTE );

                cmdList->dispatchComputeCmd( DispatchSize( 16, width ), DispatchSize( 16, height ), 1 );

                roughnessUAV->unbind( cmdList );

                width >>= 1;
                height >>= 1;
            }
        }
    );

    const auto& normalMapDesc = normalMap->getDescription();
    
    VMFResolveOutput output;
    output.generatedMipCount = normalMapDesc.mipCount;
    for ( unsigned int mipLevel = 0; mipLevel < normalMapDesc.mipCount; mipLevel++ ) {
        output.roughnessMipMaps[mipLevel] = RenderPass.buffers[( mipLevel + 1 )];
    }

    return output;
}

// IMPORTANT It is implicitely assumed that normal map dimensions are the same as the roughness map
// Dimensions mismatch will result as an invalid output
static VMFResolveOutput AddVMFMapComputePass( RenderPipeline* renderPipeline, Texture* normalMap, Texture* roughnessMap )
{
    struct PassBuffer
    {
        float       inputTexWidth;
        float       inputTexHeight;
        float       outputTexWidth;
        float       outputTexHeight;
        uint32_t    mipLevel;
        float       roughness;
        float       scaleFactor;
        uint32_t    __PADDING__;
    };

    FLAN_IS_MEMORY_ALIGNED( 16, PassBuffer )

    auto RenderPass = renderPipeline->addRenderPass(
        "VMF Solver (using precomputed Texture)",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "VMF Solver (using precomputed Texture)" );
            passPipelineState.computeStage = FLAN_STRING( "VMFSolverTextureMapInput" );

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            // Constant Buffer
            BufferDesc bufferDesc;
            bufferDesc.Type = BufferDesc::CONSTANT_BUFFER;
            bufferDesc.Size = sizeof( PassBuffer );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( bufferDesc );

            auto& roughnessMapDesc = roughnessMap->getDescription();
            auto width = roughnessMapDesc.width, height = roughnessMapDesc.height;
            for ( unsigned int mipLevel = 0; mipLevel < roughnessMapDesc.mipCount; mipLevel++ ) {
                // Texture UAV
                BufferDesc bufferDesc;
                bufferDesc.Type = BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D;
                bufferDesc.ViewFormat = IMAGE_FORMAT_R8_UNORM;
                bufferDesc.Width = width;
                bufferDesc.Height = height;
                bufferDesc.Depth = 1;
                bufferDesc.MipCount = 1;

                passData.buffers[( mipLevel + 1 )] = renderPipelineBuilder->allocateBuffer( bufferDesc );

                width >>= 1;
                height >>= 1;
            }
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Bind Normal Map
            normalMap->bind( cmdList, 0, SHADER_STAGE_COMPUTE );
            roughnessMap->bind( cmdList, 1, SHADER_STAGE_COMPUTE );

            auto& normalMapDesc = normalMap->getDescription();

            auto buffer = renderPipelineResources->getBuffer( passData.buffers[0] );

            PassBuffer passBufferData;
            passBufferData.inputTexWidth = normalMapDesc.width;
            passBufferData.inputTexHeight = normalMapDesc.height;
            passBufferData.outputTexWidth = normalMapDesc.width;
            passBufferData.outputTexHeight = normalMapDesc.height;
            passBufferData.mipLevel = 0;
            passBufferData.roughness = 0;
            passBufferData.scaleFactor = std::pow( 10.0f, 0.5f );
            buffer->updateAsynchronous( cmdList, &passBufferData, sizeof( PassBuffer ) );

            buffer->bind( cmdList, 0, SHADER_STAGE_COMPUTE );

            auto width = normalMapDesc.width, height = normalMapDesc.height;
            for ( unsigned int mipLevel = 0; mipLevel < normalMapDesc.mipCount; mipLevel++ ) {
                auto roughnessUAV = renderPipelineResources->getBuffer( passData.buffers[( mipLevel + 1 )] );

                passBufferData.mipLevel = mipLevel;
                passBufferData.outputTexWidth = width;
                passBufferData.outputTexHeight = height;
                buffer->updateAsynchronous( cmdList, &passBufferData, sizeof( PassBuffer ) );

                roughnessUAV->bind( cmdList, 0, SHADER_STAGE_COMPUTE );

                cmdList->dispatchComputeCmd( DispatchSize( 16, width ), DispatchSize( 16, height ), 1 );

                roughnessUAV->unbind( cmdList );

                width >>= 1;
                height >>= 1;
            }
        } 
    );

    const auto& normalMapDesc = normalMap->getDescription();

    VMFResolveOutput output;
    output.generatedMipCount = normalMapDesc.mipCount;
    for ( unsigned int mipLevel = 0; mipLevel < normalMapDesc.mipCount; mipLevel++ ) {
        output.roughnessMipMaps[mipLevel] = RenderPass.buffers[( mipLevel + 1 )];
    }

    return output;
}
