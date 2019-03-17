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

#include "ResourceList.h"

#include "RenderDevice.h"
#include "CommandList.h"

#include "Sampler.h"

#include <vulkan/vulkan.h>

using namespace nya::rendering;

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

ResourceList& RenderDevice::allocateResourceList( const ResourceListDesc& description ) const
{
    static constexpr int MAX_RES_COUNT = 64;

    const size_t resListIdx = renderContext->resListPoolIndex;

    ++renderContext->resListPoolIndex;
    if ( renderContext->resListPoolIndex >= renderContext->resListPoolCapacity ) {
        renderContext->resListPoolIndex = 0;
    }

    ResourceList& resList = renderContext->resListPool[resListIdx];
    resList = { 0 };

    VkDescriptorSetLayoutBinding descriptorSetBinding[MAX_RES_COUNT + MAX_RES_COUNT + MAX_RES_COUNT + MAX_RES_COUNT];

    for ( int i = 0; i < MAX_RES_COUNT; i++ ) {
        const auto& sampler = description.samplers[i];

        VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding = descriptorSetBinding[i];
        descriptorSetLayoutBinding.binding = sampler.bindPoint;
        descriptorSetLayoutBinding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptorSetLayoutBinding.descriptorCount = 1u;
        descriptorSetLayoutBinding.stageFlags = GetDescriptorStageFlags( sampler.stageBind );
        descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
    }

    for ( int i = 0; i < MAX_RES_COUNT; i++ ) {
        const auto& constantBuffer = description.constantBuffers[i];

        VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding = descriptorSetBinding[MAX_RES_COUNT + i];
        descriptorSetLayoutBinding.binding = constantBuffer.bindPoint;
        descriptorSetLayoutBinding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorSetLayoutBinding.descriptorCount = 1u;
        descriptorSetLayoutBinding.stageFlags = GetDescriptorStageFlags( constantBuffer.stageBind );
        descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
    }

    for ( int i = 0; i < MAX_RES_COUNT; i++ ) {
        const auto& buffer = description.buffers[i];

        VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding = descriptorSetBinding[MAX_RES_COUNT + MAX_RES_COUNT + i];
        descriptorSetLayoutBinding.binding = buffer.bindPoint;
        descriptorSetLayoutBinding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        descriptorSetLayoutBinding.descriptorCount = 1u;
        descriptorSetLayoutBinding.stageFlags = GetDescriptorStageFlags( buffer.stageBind );
        descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
    }

    for ( int i = 0; i < MAX_RES_COUNT; i++ ) {
        const auto& buffer = description.uavBuffers[i];

        VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding = descriptorSetBinding[MAX_RES_COUNT + MAX_RES_COUNT + MAX_RES_COUNT + i];
        descriptorSetLayoutBinding.binding = buffer.bindPoint;
        descriptorSetLayoutBinding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorSetLayoutBinding.descriptorCount = 1u;
        descriptorSetLayoutBinding.stageFlags = GetDescriptorStageFlags( buffer.stageBind );
        descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.pNext = nullptr;
    descriptorSetLayoutInfo.flags = 0u;
    descriptorSetLayoutInfo.bindingCount = ( MAX_RES_COUNT * 4 );
    descriptorSetLayoutInfo.pBindings = descriptorSetBinding;

    vkCreateDescriptorSetLayout( renderContext->device, &descriptorSetLayoutInfo, nullptr, &resList.descriptorSetLayout );

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = renderContext->descriptorPool;
    allocInfo.descriptorSetCount = 1u;
    allocInfo.pSetLayouts = &resList.descriptorSetLayout;

    vkAllocateDescriptorSets( renderContext->device, &allocInfo, &resList.descriptorSet );

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.flags = 0u;
    pipelineLayoutInfo.setLayoutCount = 0u;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0u;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    vkCreatePipelineLayout( renderContext->device, &pipelineLayoutInfo, nullptr, &resList.pipelineLayout );

    return resList;
}

void CommandList::bindResourceList( ResourceList* resourceList )
{
    vkCmdBindDescriptorSets( 
        NativeCommandList->cmdBuffer, 
        NativeCommandList->resourcesBindPoint, 
        resourceList->pipelineLayout, 
        0u, 
        1u, 
        &resourceList->descriptorSet,
        0u, 
        nullptr );
}
#endif
