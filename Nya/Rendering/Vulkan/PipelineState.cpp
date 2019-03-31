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
#include "ImageHelpers.h"
#include "PipelineState.h"

#include <vulkan/vulkan.h>
#include <string.h>

using namespace nya::rendering;

static constexpr VkPrimitiveTopology VK_PRIMITIVE_TOPOLOGY[ePrimitiveTopology::PRIMITIVE_TOPOLOGY_COUNT] =
{
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
    VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
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
    VK_POLYGON_MODE_FILL,
    VK_POLYGON_MODE_LINE,
};

static constexpr VkCullModeFlags VK_CM[eCullMode::CULL_MODE_COUNT] =
{
    VK_CULL_MODE_NONE,
    VK_CULL_MODE_FRONT_BIT,
    VK_CULL_MODE_BACK_BIT,
    VK_CULL_MODE_FRONT_AND_BACK,
};

static constexpr VkDescriptorType VK_DT[ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_COUNT] =
{
    VK_DESCRIPTOR_TYPE_SAMPLER,
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
    VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
};

static constexpr uint32_t VK_DT_OFFSET[ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_COUNT] =
{
    0u,
    64u,
    192u,
    128u,
    256u,
    256u,
    384u,
};

void CreateShaderStageDescriptor( VkPipelineShaderStageCreateInfo& shaderStageInfos, const Shader* shader )
{
    shaderStageInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfos.pNext = nullptr;
    shaderStageInfos.flags = 0u;
    shaderStageInfos.stage = shader->shaderStage;
    shaderStageInfos.module = shader->shaderModule;

    switch ( shader->shaderStage ) {
    case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT:
        shaderStageInfos.pName = "EntryPointVS";
        break;
    case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT:
        shaderStageInfos.pName = "EntryPointCS";
        break;
    case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT:
        shaderStageInfos.pName = "EntryPointPS";
        break;
    case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
        shaderStageInfos.pName = "EntryPointDS";
        break;
    case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
        shaderStageInfos.pName = "EntryPointHS";
        break;
    default:
        shaderStageInfos.pName = "main";
        break;
    }

    shaderStageInfos.pSpecializationInfo = nullptr;
}

VkShaderStageFlags GetDescriptorStageFlags( const uint32_t shaderStageBindBitfield )
{
    VkShaderStageFlags bindFlags = 0u;

    if ( shaderStageBindBitfield & SHADER_STAGE_VERTEX ) {
        bindFlags |= VK_SHADER_STAGE_VERTEX_BIT;
    }

    if ( shaderStageBindBitfield & SHADER_STAGE_TESSELATION_CONTROL ) {
        bindFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    }

    if ( shaderStageBindBitfield & SHADER_STAGE_TESSELATION_EVALUATION ) {
        bindFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    }

    if ( shaderStageBindBitfield & SHADER_STAGE_PIXEL ) {
        bindFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    if ( shaderStageBindBitfield & SHADER_STAGE_COMPUTE ) {
        bindFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
    }

    return bindFlags;
}

PipelineState* RenderDevice::createPipelineState( const PipelineStateDesc& description )
{
    PipelineState* pipelineState = nya::core::allocate<PipelineState>( memoryAllocator );

    uint32_t bindingCount = 0u;
    VkDescriptorSetLayoutBinding descriptorSetBinding[ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_COUNT * 64];
    for ( unsigned int i = 0u; i < 64u; i++ ) {
        const auto& binding = description.resourceListLayout.resources[i];

        if ( binding.stageBind == 0u ) {
            break;
        }

        VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding = descriptorSetBinding[bindingCount];
        descriptorSetLayoutBinding.binding = VK_DT_OFFSET[binding.type] + static_cast<uint32_t>( binding.bindPoint );
        descriptorSetLayoutBinding.descriptorType = VK_DT[binding.type];
        descriptorSetLayoutBinding.descriptorCount = 1u;
        descriptorSetLayoutBinding.stageFlags = GetDescriptorStageFlags( binding.stageBind );
        descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

        bindingCount++;
    }

    // Resource List
    VkDescriptorSetLayout resourceListDescriptorSetLayout;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.pNext = nullptr;
    descriptorSetLayoutInfo.flags = 0u;

    descriptorSetLayoutInfo.bindingCount = bindingCount;
    descriptorSetLayoutInfo.pBindings = descriptorSetBinding;
    vkCreateDescriptorSetLayout( renderContext->device, &descriptorSetLayoutInfo, nullptr, &resourceListDescriptorSetLayout );

    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = renderContext->descriptorPool;
    allocInfo.descriptorSetCount = 1u;
    allocInfo.pSetLayouts = &resourceListDescriptorSetLayout;

    vkAllocateDescriptorSets( renderContext->device, &allocInfo, &pipelineState->descriptorSet );

    // Allocate pipeline Layout for the current resource list
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.flags = 0u;
    pipelineLayoutInfo.setLayoutCount = 1u;
    pipelineLayoutInfo.pSetLayouts = &resourceListDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0u;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    vkCreatePipelineLayout( renderContext->device, &pipelineLayoutInfo, nullptr, &pipelineState->layout );

    // Build pipeline state descriptor
    if ( !( description.computeShader != nullptr ) ) {
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
        rasterizerStateInfos.rasterizerDiscardEnable = VK_TRUE;
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

        VkVertexInputAttributeDescription vertexInputAttributeDesc[8];

        uint32_t vertexStride = 0u;
        uint32_t i = 0;
        for ( ; ; i++ ) {
            if ( description.inputLayout[i].semanticName == nullptr ) {
                break;
            }

            vertexInputAttributeDesc[i].location = i;
            vertexInputAttributeDesc[i].binding = description.inputLayout[i].vertexBufferIndex;
            vertexInputAttributeDesc[i].format = VK_IMAGE_FORMAT[description.inputLayout[i].format];
            vertexInputAttributeDesc[i].offset = description.inputLayout[i].offsetInBytes;

            vertexStride += VK_IMAGE_FORMAT_SIZE[description.inputLayout[i].format];
        }

        VkVertexInputBindingDescription vertexInputBindingDesc;
        vertexInputBindingDesc.binding = 0;
        vertexInputBindingDesc.stride = vertexStride;
        vertexInputBindingDesc.inputRate = ( description.inputLayout[0].instanceCount ) > 1u ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;

        VkPipelineVertexInputStateCreateInfo vertexInputStateDesc;
        vertexInputStateDesc.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateDesc.pNext = nullptr;
        vertexInputStateDesc.flags = 0u;
        vertexInputStateDesc.vertexBindingDescriptionCount = 1u;
        vertexInputStateDesc.pVertexBindingDescriptions = &vertexInputBindingDesc;
        vertexInputStateDesc.vertexAttributeDescriptionCount = i;
        vertexInputStateDesc.pVertexAttributeDescriptions = vertexInputAttributeDesc;

        // Create Pipeline descriptor
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
        pipelineInfo.pVertexInputState = &vertexInputStateDesc;

        uint32_t pipelineStageCount = 0u;
        VkPipelineShaderStageCreateInfo pipelineStages[SHADER_STAGE_COUNT];

        if ( description.vertexShader != nullptr ) {
            CreateShaderStageDescriptor( pipelineStages[pipelineStageCount++], description.vertexShader );
        }
        if ( description.tesselationEvalShader != nullptr ) {
            CreateShaderStageDescriptor( pipelineStages[pipelineStageCount++], description.tesselationEvalShader );
        }
        if ( description.tesselationControlShader != nullptr ) {
            CreateShaderStageDescriptor( pipelineStages[pipelineStageCount++], description.tesselationControlShader );
        }
        if ( description.pixelShader != nullptr ) {
            CreateShaderStageDescriptor( pipelineStages[pipelineStageCount++], description.pixelShader );
        }

        pipelineInfo.pStages = pipelineStages;
        pipelineInfo.stageCount = pipelineStageCount;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        pipelineInfo.layout = pipelineState->layout;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.pNext = nullptr;
        renderPassInfo.flags = 0u;

        VkSubpassDescription subpassDesc;
        subpassDesc.flags = 0u;
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        VkAttachmentReference readReference[24];
        VkAttachmentReference writeReference[24];

        VkAttachmentReference depthStencilReference;
        depthStencilReference.attachment = VK_ATTACHMENT_UNUSED;

        uint32_t readReferenceCount = 0u;
        uint32_t writeReferenceCount = 0u;

        VkAttachmentDescription attachments[24];
        uint32_t attachmentCount = 0u;
        for ( int i = 0; i < 24; i++ ) {
            const auto& attachment = description.renderPassLayout.attachements[i];

            switch ( attachment.bindMode ) {
            case RenderPassLayoutDesc::READ: {
                VkAttachmentDescription& attachmentDesc = attachments[attachmentCount];
                attachmentDesc.flags = 0u;
                attachmentDesc.format = VK_IMAGE_FORMAT[attachment.viewFormat];
                attachmentDesc.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

                attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkAttachmentReference& attachmentReference = readReference[readReferenceCount++];
                attachmentReference.attachment = attachmentCount;
                attachmentReference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                attachmentCount++;
            } break;

            case RenderPassLayoutDesc::WRITE: {
                VkAttachmentDescription& attachmentDesc = attachments[attachmentCount];
                attachmentDesc.flags = 0u;
                attachmentDesc.format = VK_IMAGE_FORMAT[attachment.viewFormat];
                attachmentDesc.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

                attachmentDesc.loadOp = ( attachment.targetState == RenderPassLayoutDesc::DONT_CARE )
                    ? VK_ATTACHMENT_LOAD_OP_DONT_CARE
                    : VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

                attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                if ( attachment.targetState != RenderPassLayoutDesc::DONT_CARE )
                    memcpy( &pipelineState->clearValues[attachmentCount], attachment.clearValue, sizeof( float ) * 4 );

                VkAttachmentReference& attachmentReference = writeReference[writeReferenceCount++];
                attachmentReference.attachment = attachmentCount;
                attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                attachmentCount++;
               } break;

            case RenderPassLayoutDesc::WRITE_DEPTH: {
                VkAttachmentDescription& attachmentDesc = attachments[attachmentCount];
                attachmentDesc.flags = 0u;
                attachmentDesc.format = VK_IMAGE_FORMAT[attachment.viewFormat];
                attachmentDesc.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

                attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                attachmentDesc.stencilLoadOp = ( attachment.targetState == RenderPassLayoutDesc::DONT_CARE )
                    ? VK_ATTACHMENT_LOAD_OP_DONT_CARE
                    : VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

                attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                if ( attachment.targetState != RenderPassLayoutDesc::DONT_CARE )
                    memcpy( &pipelineState->clearValues[attachmentCount], attachment.clearValue, sizeof( float ) * 4 );

                depthStencilReference.attachment = attachmentCount;
                depthStencilReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                attachmentCount++;
            } break;

            default:
                break;
            }
        }

        subpassDesc.pInputAttachments = readReference;
        subpassDesc.inputAttachmentCount = readReferenceCount;
        subpassDesc.pColorAttachments = writeReference;
        subpassDesc.colorAttachmentCount = writeReferenceCount;
        subpassDesc.pDepthStencilAttachment = ( depthStencilReference.attachment = VK_ATTACHMENT_UNUSED ) ? nullptr : &depthStencilReference;
        subpassDesc.pResolveAttachments = nullptr;
        subpassDesc.preserveAttachmentCount = 0u;
        subpassDesc.pPreserveAttachments = nullptr;

        renderPassInfo.attachmentCount = attachmentCount;
        renderPassInfo.pAttachments = attachments;
        renderPassInfo.subpassCount = 1u;
        renderPassInfo.pSubpasses = &subpassDesc;
        renderPassInfo.dependencyCount = 0u;
        renderPassInfo.pDependencies = nullptr;

        vkCreateRenderPass( renderContext->device, &renderPassInfo, nullptr, &pipelineState->renderPass );

        pipelineInfo.renderPass = pipelineState->renderPass;
        pipelineInfo.subpass = 0u;

        pipelineState->bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        vkCreateGraphicsPipelines( renderContext->device, VK_NULL_HANDLE, 1u, &pipelineInfo, nullptr, &pipelineState->pipelineObject );
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
        shaderStageInfos.pName = "EntryPointCS";
        shaderStageInfos.pSpecializationInfo = nullptr;

        pipelineInfo.stage = shaderStageInfos;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        pipelineInfo.layout = pipelineState->layout;

        pipelineState->bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
        vkCreateComputePipelines( renderContext->device, VK_NULL_HANDLE, 1u, &pipelineInfo, nullptr, &pipelineState->pipelineObject );
    }

    return pipelineState;
}

void RenderDevice::destroyPipelineState( PipelineState* pipelineState )
{
    vkDestroyPipelineLayout( renderContext->device, pipelineState->layout, nullptr );
    vkDestroyPipeline( renderContext->device, pipelineState->pipelineObject, nullptr );
    vkDestroyRenderPass( renderContext->device, pipelineState->renderPass, nullptr );

    nya::core::free( memoryAllocator, pipelineState );
}

void CommandList::bindPipelineState( PipelineState* pipelineState )
{
    vkCmdBindPipeline( CommandListObject->cmdBuffer, pipelineState->bindPoint, pipelineState->pipelineObject );
}
#endif
