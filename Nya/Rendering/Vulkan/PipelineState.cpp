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

#if NYA_VULKAN
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#include "RenderDevice.h"
#include "CommandList.h"
#include "Shader.h"
#include "ComparisonFunctions.h"

#include <vulkan/vulkan.h>

using namespace nya::rendering;

static constexpr VkPrimitiveTopology VK_PRIMITIVE_TOPOLOGY[ePrimitiveTopology::PRIMITIVE_TOPOLOGY_COUNT] =
{
    VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
    VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
};

static constexpr VkBlendFactor VK_BLEND_SOURCE[eBlendSource::BLEND_SOURCE_COUNT] =
{
    VK_BLEND_FACTOR_ZERO,
    VK_BLEND_FACTOR_ONE,

    VK_BLEND_FACTOR_SRC_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,

    VK_BLEND_FACTOR_SRC_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,

    VK_BLEND_FACTOR_DST_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,

    VK_BLEND_FACTOR_DST_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,

    VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,

    VK_BLEND_FACTOR_CONSTANT_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
};

static constexpr VkBlendOp VK_BLEND_OPERATION[eBlendOperation::BLEND_OPERATION_COUNT] =
{
    VK_BLEND_OP_ADD,
    VK_BLEND_OP_SUBTRACT,
    VK_BLEND_OP_MIN,
    VK_BLEND_OP_MAX,
};

static constexpr VkStencilOp VK_STENCIL_OPERATION[eStencilOperation::STENCIL_OPERATION_COUNT] =
{
    VK_STENCIL_OP_KEEP,
    VK_STENCIL_OP_ZERO,
    VK_STENCIL_OP_REPLACE,
    VK_STENCIL_OP_INCREMENT_AND_WRAP,
    VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    VK_STENCIL_OP_DECREMENT_AND_WRAP,
    VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    VK_STENCIL_OP_INVERT
};

static constexpr VkPolygonMode VK_FM[eFillMode::FILL_MODE_COUNT] =
{
    VkPolygonMode::VK_POLYGON_MODE_FILL,
    VkPolygonMode::VK_POLYGON_MODE_LINE,
};

static constexpr VkCullModeFlags VK_CM[eCullMode::CULL_MODE_COUNT] =
{
    VK_CULL_MODE_NONE,
    VK_CULL_MODE_FRONT_BIT,
    VK_CULL_MODE_BACK_BIT,
    VK_CULL_MODE_FRONT_AND_BACK,
};

struct PipelineState
{
    VkPipeline          pipelineObject;
    VkPipelineBindPoint bindPoint;
};

PipelineState* RenderDevice::createPipelineState( const PipelineStateDesc& description )
{
    PipelineState* pipelineState = nya::core::allocate<PipelineState>( memoryAllocator );

    // Blend State
    const BlendStateDesc& blendStateDescription = description.blendState;

    VkPipelineColorBlendStateCreateInfo blendCreateInfos = {};
    blendCreateInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendCreateInfos.pNext = nullptr;
    blendCreateInfos.flags = 0;

    blendCreateInfos.attachmentCount = 8;
    blendCreateInfos.logicOpEnable = VK_FALSE;

    blendCreateInfos.blendConstants[0] = 1.0f;
    blendCreateInfos.blendConstants[1] = 1.0f;
    blendCreateInfos.blendConstants[2] = 1.0f;
    blendCreateInfos.blendConstants[3] = 1.0f;

    VkPipelineColorBlendAttachmentState blendAttachments[8];
    for ( int i = 0; i < 8; i++ ) {
        VkPipelineColorBlendAttachmentState& blendAttachementState = blendAttachments[i];

        blendAttachementState.blendEnable = ( blendStateDescription.enableBlend ) ? VK_TRUE : VK_FALSE;

        // Reset write mask
        blendAttachementState.colorWriteMask = 0;

        if ( blendStateDescription.writeMask[0] )
            blendAttachementState.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
        else if ( blendStateDescription.writeMask[1] )
            blendAttachementState.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
        else if ( blendStateDescription.writeMask[2] )
            blendAttachementState.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
        else if ( blendStateDescription.writeMask[3] )
            blendAttachementState.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;

        blendAttachementState.srcColorBlendFactor = VK_BLEND_SOURCE[blendStateDescription.blendConfColor.source];
        blendAttachementState.dstColorBlendFactor = VK_BLEND_SOURCE[blendStateDescription.blendConfColor.dest];
        blendAttachementState.colorBlendOp = VK_BLEND_OPERATION[blendStateDescription.blendConfColor.operation];

        blendAttachementState.srcAlphaBlendFactor = VK_BLEND_SOURCE[blendStateDescription.blendConfAlpha.source];
        blendAttachementState.dstAlphaBlendFactor = VK_BLEND_SOURCE[blendStateDescription.blendConfAlpha.dest];
        blendAttachementState.alphaBlendOp = VK_BLEND_OPERATION[blendStateDescription.blendConfAlpha.operation];
    }

    blendCreateInfos.pAttachments = blendAttachments;


    // Depth/Stencil state
    const DepthStencilStateDesc& depthStencilDescription = description.depthStencilState;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfos;
    depthStencilStateInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateInfos.pNext = nullptr;
    depthStencilStateInfos.flags = 0u;
    depthStencilStateInfos.depthTestEnable = static_cast< VkBool32 >( depthStencilDescription.enableDepthTest );
    depthStencilStateInfos.depthWriteEnable = static_cast< VkBool32 >( depthStencilDescription.enableDepthWrite );
    depthStencilStateInfos.depthCompareOp = VK_COMPARISON_FUNCTION[depthStencilDescription.depthComparisonFunc];
    depthStencilStateInfos.depthBoundsTestEnable = depthStencilDescription.enableDepthBoundsTest;
    depthStencilStateInfos.stencilTestEnable = depthStencilDescription.enableStencilTest;

    VkStencilOpState frontStencilState;
    frontStencilState.failOp = VK_STENCIL_OPERATION[depthStencilDescription.front.failOp];
    frontStencilState.passOp = VK_STENCIL_OPERATION[depthStencilDescription.front.passOp];
    frontStencilState.depthFailOp = VK_STENCIL_OPERATION[depthStencilDescription.front.zFailOp];
    frontStencilState.compareOp = VK_COMPARISON_FUNCTION[depthStencilDescription.front.comparisonFunc];
    frontStencilState.compareMask = depthStencilDescription.stencilReadMask;
    frontStencilState.writeMask = depthStencilDescription.stencilWriteMask;
    frontStencilState.reference = depthStencilDescription.stencilRefValue;
    depthStencilStateInfos.front = frontStencilState;

    VkStencilOpState backStencilState;
    backStencilState.failOp = VK_STENCIL_OPERATION[depthStencilDescription.back.failOp];
    backStencilState.passOp = VK_STENCIL_OPERATION[depthStencilDescription.back.passOp];
    backStencilState.depthFailOp = VK_STENCIL_OPERATION[depthStencilDescription.back.zFailOp];
    backStencilState.compareOp = VK_COMPARISON_FUNCTION[depthStencilDescription.back.comparisonFunc];
    backStencilState.compareMask = depthStencilDescription.stencilReadMask;
    backStencilState.writeMask = depthStencilDescription.stencilWriteMask;
    backStencilState.reference = depthStencilDescription.stencilRefValue;
    depthStencilStateInfos.back = backStencilState;

    depthStencilStateInfos.minDepthBounds = depthStencilDescription.depthBoundsMin;
    depthStencilStateInfos.maxDepthBounds = depthStencilDescription.depthBoundsMax;

    // Input Assembly (Vertex Input State)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = nullptr;
    inputAssembly.flags = 0u;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY[description.primitiveTopology];
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // "Input Layout"
    VkPipelineVertexInputStateCreateInfo vertexInputInfos;
    vertexInputInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfos.pNext = VK_NULL_HANDLE;
    vertexInputInfos.flags = 0;

    // Rasterizer state
    const RasterizerStateDesc& rasterizerDescription = description.rasterizerState;

    VkPipelineRasterizationStateCreateInfo rasterizerStateInfos;
    rasterizerStateInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerStateInfos.pNext = nullptr;
    rasterizerStateInfos.flags = 0;
    rasterizerStateInfos.depthClampEnable = static_cast< VkBool32 >( rasterizerDescription.depthBiasClamp != 0.0f );
    rasterizerStateInfos.rasterizerDiscardEnable = VK_FALSE;
    rasterizerStateInfos.polygonMode = VK_FM[rasterizerDescription.fillMode];
    rasterizerStateInfos.cullMode = VK_CM[rasterizerDescription.cullMode];
    rasterizerStateInfos.frontFace = ( rasterizerDescription.useTriangleCCW ) ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterizerStateInfos.depthBiasEnable = static_cast< VkBool32 >( rasterizerDescription.depthBias != 0.0f );
    rasterizerStateInfos.depthBiasConstantFactor = rasterizerDescription.depthBias;
    rasterizerStateInfos.depthBiasClamp = rasterizerDescription.depthBiasClamp;
    rasterizerStateInfos.depthBiasSlopeFactor = rasterizerDescription.slopeScale;
    rasterizerStateInfos.lineWidth = 1.0f;

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

    // Build pipeline state descriptor
    const bool isComputePipeline = ( description.computeShader != nullptr );
    if ( !isComputePipeline ) {
        VkGraphicsPipelineCreateInfo pipelineInfo;
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = nullptr;
        pipelineInfo.flags = 0u;

        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pTessellationState = nullptr;
        pipelineInfo.pViewportState = nullptr;
        pipelineInfo.pRasterizationState = &rasterizerStateInfos;
        pipelineInfo.pMultisampleState = nullptr;
        pipelineInfo.pDepthStencilState = &depthStencilStateInfos;
        pipelineInfo.pColorBlendState = &blendCreateInfos;
        pipelineInfo.pDynamicState = &dynamicState;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        pipelineState->bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    } else {
        VkComputePipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = nullptr;
        pipelineInfo.flags = 0u;

        VkPipelineShaderStageCreateInfo shaderStageInfos = {};
        shaderStageInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfos.pNext = nullptr;
        shaderStageInfos.flags = 0u;
        shaderStageInfos.stage = description.computeShader->shaderStage;
        shaderStageInfos.module = description.computeShader->shaderModule;
        shaderStageInfos.pName = "main";
        shaderStageInfos.pSpecializationInfo = nullptr;

        pipelineInfo.stage = shaderStageInfos;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        pipelineState->bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    }

    return pipelineState;
}

void RenderDevice::destroyPipelineState( PipelineState* pipelineState )
{
    // vkDestroyPipelineLayout( renderContext->device, pipelineState->nativePipelineLayout, nullptr );
    vkDestroyPipeline( renderContext->device, pipelineState->pipelineObject, nullptr );

    nya::core::free( memoryAllocator, pipelineState );
}

void CommandList::bindPipelineState( PipelineState* pipelineState )
{
    vkCmdBindPipeline( CommandListObject->cmdBuffer, pipelineState->bindPoint, pipelineState->pipelineObject );
}
#endif
