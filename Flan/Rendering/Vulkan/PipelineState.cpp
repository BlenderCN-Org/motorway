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
#include <Shared.h>

#if FLAN_VULKAN
#include "PipelineState.h"

#include "RenderContext.h"
#include "Shader.h"

#include "PrimtiveTopologies.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "BlendState.h"

NativePipelineStateObject* flan::rendering::CreatePipelineStateImpl( NativeRenderContext* nativeRenderContext, const PipelineStateDesc& description )
{
    NativePipelineStateObject* nativePipelineStateObject = new NativePipelineStateObject();

    // Build Vertex Input State
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = VK_NULL_HANDLE;
    inputAssembly.flags = 0;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY[description.primitiveTopology];
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Don't bake viewport dimensions (allow viewport resizing at runtime; more convenient)
    static constexpr VkDynamicState dynamicStates[1] = {
        VK_DYNAMIC_STATE_VIEWPORT,
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = VK_NULL_HANDLE;
    dynamicState.flags = 0;
    dynamicState.dynamicStateCount = 1;
    dynamicState.pDynamicStates = dynamicStates;

    const bool isComputePipeline = ( description.computeStage != nullptr );

    if ( !isComputePipeline ) {
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = VK_NULL_HANDLE;
        pipelineInfo.flags = 0;

        // Build shader stages array
        if ( description.vertexStage != nullptr ) {
            nativePipelineStateObject->shaderStages.push_back( &description.vertexStage->getNativeShaderObject()->nativeStageInfos );
        }
        if ( description.pixelStage != nullptr ) {
            nativePipelineStateObject->shaderStages.push_back( &description.pixelStage->getNativeShaderObject()->nativeStageInfos );
        }
        if ( description.tesselationControlStage != nullptr ) {
            nativePipelineStateObject->shaderStages.push_back( &description.tesselationControlStage->getNativeShaderObject()->nativeStageInfos );
        }
        if ( description.tesselationEvalStage != nullptr ) {
            nativePipelineStateObject->shaderStages.push_back( &description.tesselationEvalStage->getNativeShaderObject()->nativeStageInfos );
        }

        pipelineInfo.stageCount = static_cast<uint32_t>( nativePipelineStateObject->shaderStages.size() );
        pipelineInfo.pStages = nativePipelineStateObject->shaderStages.front();
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pTessellationState = nullptr;
        pipelineInfo.pViewportState = nullptr;
        pipelineInfo.pRasterizationState = &description.rasterizerState->getNativeRasterizerStateObject()->nativeRasterizerState;
        pipelineInfo.pMultisampleState = nullptr;

        if ( description.depthStencilState != nullptr )
            pipelineInfo.pDepthStencilState = &description.depthStencilState->getNativeDepthStencilStateObject()->nativeDepthStencilState;
        else
            pipelineInfo.pDepthStencilState = VK_NULL_HANDLE;

        if ( description.blendState != nullptr )
            pipelineInfo.pColorBlendState = &description.blendState->getNativeBlendStateObject()->nativeBlendState;
        else
            pipelineInfo.pColorBlendState = VK_NULL_HANDLE;

        pipelineInfo.pDynamicState = &dynamicState;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;
    } else {
        VkComputePipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = VK_NULL_HANDLE;
        pipelineInfo.flags = 0;

        pipelineInfo.stage = description.computeStage->getNativeShaderObject()->nativeStageInfos;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;
    }

    return nativePipelineStateObject;
}

void flan::rendering::DestroyPipelineStateImpl( NativeRenderContext* nativeRenderContext, NativePipelineStateObject* PipelineStateObject )
{
    PipelineStateObject->shaderStages.clear();

    vkDestroyPipelineLayout( nativeRenderContext->device, PipelineStateObject->nativePipelineLayout, nullptr );
    vkDestroyPipeline( nativeRenderContext->device, PipelineStateObject->nativePipelineObject, nullptr );
}
#endif
