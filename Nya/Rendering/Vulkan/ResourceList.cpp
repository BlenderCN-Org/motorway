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

#include "Sampler.h"
#include "Buffer.h"
#include "PipelineState.h"

#include <Maths/Helpers.h>

#include <vulkan/vulkan.h>

using namespace nya::rendering;

/*VkShaderStageFlags GetDescriptorStageFlags( const uint32_t shaderStageBindBitfield )
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

static constexpr int MAX_RES_COUNT = 64;

template<typename T>
void CreateDescriptorSetAndLayout( const VkDevice device, const VkDescriptorPool descriptorPool, const VkDescriptorType descriptorType, const ResourceListDesc::ResourceList<T*>* description, VkDescriptorSet& descriptorSet, VkDescriptorSetLayout& descriptorSetLayout, uint32_t& bindCount )
{
    bindCount = 0u;

    VkDescriptorSetLayoutBinding descriptorSetBinding[MAX_RES_COUNT];

    for ( int i = 0; i < MAX_RES_COUNT; i++ ) {
        const auto& resource = description[i];

        // TODO Assuming we reach the end of the resource list once a resource has no explicit stage binding
        // Is that wise?
        if ( resource.stageBind == 0u ) {
            break;
        }

        VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding = descriptorSetBinding[bindCount++];
        descriptorSetLayoutBinding.binding = static_cast<uint32_t>( resource.bindPoint );
        descriptorSetLayoutBinding.descriptorType = descriptorType;
        descriptorSetLayoutBinding.descriptorCount = 1u;
        descriptorSetLayoutBinding.stageFlags = GetDescriptorStageFlags( resource.stageBind );
        descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.pNext = nullptr;
    descriptorSetLayoutInfo.flags = 0u;
    descriptorSetLayoutInfo.bindingCount = bindCount;
    descriptorSetLayoutInfo.pBindings = descriptorSetBinding;

    vkCreateDescriptorSetLayout( device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout );

    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1u;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    vkAllocateDescriptorSets( device, &allocInfo, &descriptorSet );
}

ResourceList& RenderDevice::allocateResourceList( const ResourceListDesc& description ) const
{
    const size_t resListIdx = renderContext->resListPoolIndex;

    ++renderContext->resListPoolIndex;
    if ( renderContext->resListPoolIndex >= renderContext->resListPoolCapacity ) {
        renderContext->resListPoolIndex = 0;
    }

    ResourceList& resList = renderContext->resListPool[resListIdx];

    // TODO Avoid destroy/create (must be cachable...)
    if ( resList.pipelineLayout != nullptr )
        vkDestroyPipelineLayout( renderContext->device, resList.pipelineLayout, nullptr );

    resList.descriptorSet[0] = nullptr;
    resList.descriptorSet[1] = nullptr;
    resList.descriptorSet[2] = nullptr;
    resList.descriptorSet[3] = nullptr;
    resList.descriptorSet[4] = nullptr;

    resList.pipelineLayout = nullptr;

    VkDescriptorSetLayout resourceListDescriptorSetLayout[5];
    uint32_t bindingCount[5];
    CreateDescriptorSetAndLayout<Sampler>( renderContext->device, renderContext->samplerDescriptorPool, VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER, description.samplers, resList.descriptorSet[0], resourceListDescriptorSetLayout[0], bindingCount[0] );
    CreateDescriptorSetAndLayout<Buffer>( renderContext->device, renderContext->uboDescriptorPool, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, description.constantBuffers, resList.descriptorSet[1], resourceListDescriptorSetLayout[1], bindingCount[1] );
    CreateDescriptorSetAndLayout<Buffer>( renderContext->device, renderContext->utboDescriptorPool, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, description.buffers, resList.descriptorSet[2], resourceListDescriptorSetLayout[2], bindingCount[2] );

    // UAV Buffers first, UAV Textures second
    bindingCount[3] = 0u;
    bindingCount[4] = 0u;

    VkDescriptorSetLayoutBinding descriptorSetBinding[MAX_RES_COUNT], texelDescriptorSetBinding[MAX_RES_COUNT];
    for ( int i = 0; i < MAX_RES_COUNT; i++ ) {
        const auto& resource = description.uavBuffers[i];

        // TODO Assuming we reach the end of the resource list once a resource has no explicit stage binding
        // Is that wise?
        if ( resource.stageBind == 0u ) {
            break;
        }

        // TODO Only Texel Buffers should have a buffer view
        // That's a pretty lame and unsafe way to distinguish uav texture from uav buffers...
        if ( resource.resource->bufferView != nullptr ) {
            VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding = texelDescriptorSetBinding[bindingCount[3]++];
            descriptorSetLayoutBinding.binding = static_cast<uint32_t>( resource.bindPoint );
            descriptorSetLayoutBinding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            descriptorSetLayoutBinding.descriptorCount = 1u;
            descriptorSetLayoutBinding.stageFlags = GetDescriptorStageFlags( resource.stageBind );
            descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
        } else {
            VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding = descriptorSetBinding[bindingCount[4]++];
            descriptorSetLayoutBinding.binding = static_cast<uint32_t>( resource.bindPoint );
            descriptorSetLayoutBinding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorSetLayoutBinding.descriptorCount = 1u;
            descriptorSetLayoutBinding.stageFlags = GetDescriptorStageFlags( resource.stageBind );
            descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
        }
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.pNext = nullptr;
    descriptorSetLayoutInfo.flags = 0u;

    descriptorSetLayoutInfo.bindingCount = bindingCount[3];
    descriptorSetLayoutInfo.pBindings = texelDescriptorSetBinding;
    vkCreateDescriptorSetLayout( renderContext->device, &descriptorSetLayoutInfo, nullptr, &resourceListDescriptorSetLayout[3] );

    descriptorSetLayoutInfo.bindingCount = bindingCount[4];
    descriptorSetLayoutInfo.pBindings = descriptorSetBinding;
    vkCreateDescriptorSetLayout( renderContext->device, &descriptorSetLayoutInfo, nullptr, &resourceListDescriptorSetLayout[4] );

    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = renderContext->stboDescriptorPool;
    allocInfo.descriptorSetCount = 1u;
    allocInfo.pSetLayouts = &resourceListDescriptorSetLayout[3];

    vkAllocateDescriptorSets( renderContext->device, &allocInfo, &resList.descriptorSet[3] );

    allocInfo.descriptorPool = renderContext->sboDescriptorPool;
    allocInfo.descriptorSetCount = 1u;
    allocInfo.pSetLayouts = &resourceListDescriptorSetLayout[4];

    vkAllocateDescriptorSets( renderContext->device, &allocInfo, &resList.descriptorSet[4] );

    // Allocate pipeline Layout for the current resource list
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.flags = 0u;
    pipelineLayoutInfo.setLayoutCount = 5u;
    pipelineLayoutInfo.pSetLayouts = resourceListDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0u;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    vkCreatePipelineLayout( renderContext->device, &pipelineLayoutInfo, nullptr, &resList.pipelineLayout );

    // Update descriptors sets
    uint32_t descriptorSetUpdateCount = 0u;
    VkWriteDescriptorSet descriptorSetUpdates[5];

    // Samplers
    if ( bindingCount[0] > 0u ) {
        VkWriteDescriptorSet& samplerWriteDescriptorSet = descriptorSetUpdates[descriptorSetUpdateCount++];

        samplerWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        samplerWriteDescriptorSet.pNext = nullptr;
        samplerWriteDescriptorSet.dstSet = resList.descriptorSet[0];
        samplerWriteDescriptorSet.dstBinding = 0u;
        samplerWriteDescriptorSet.dstArrayElement = 0u;
        samplerWriteDescriptorSet.descriptorCount = bindingCount[0];
        samplerWriteDescriptorSet.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER;

        VkDescriptorImageInfo samplerDesc[MAX_RES_COUNT];

        uint32_t smallestBindPoint = ( MAX_RES_COUNT - 1 );
        for ( uint32_t i = 0; i < bindingCount[0]; i++ ) {
            const auto& samplerBinding = description.samplers[i];

            samplerDesc[i].sampler = samplerBinding.resource->samplerState;

            smallestBindPoint = nya::maths::min( smallestBindPoint, samplerBinding.bindPoint );
        }

        if ( smallestBindPoint > samplerWriteDescriptorSet.dstBinding ) {
            samplerWriteDescriptorSet.dstBinding = smallestBindPoint;
        }

        samplerWriteDescriptorSet.pImageInfo = samplerDesc;
    }

    // Constant Buffers
    if ( bindingCount[1] > 0u ) {
        VkWriteDescriptorSet& uboWriteDescriptorSet = descriptorSetUpdates[descriptorSetUpdateCount++];

        uboWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboWriteDescriptorSet.pNext = nullptr;
        uboWriteDescriptorSet.dstSet = resList.descriptorSet[1];
        uboWriteDescriptorSet.dstBinding = 0u;
        uboWriteDescriptorSet.dstArrayElement = 0u;
        uboWriteDescriptorSet.descriptorCount = bindingCount[1];
        uboWriteDescriptorSet.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        VkDescriptorBufferInfo uboDesc[MAX_RES_COUNT];

        uint32_t smallestBindPoint = ( MAX_RES_COUNT - 1 );
        for ( uint32_t i = 0; i < bindingCount[1]; i++ ) {
            const auto& bufferBinding = description.constantBuffers[i];
            uboDesc[i].buffer = bufferBinding.resource->bufferObject;
            uboDesc[i].offset = 0ull;
            uboDesc[i].range = VK_WHOLE_SIZE;

            smallestBindPoint = nya::maths::min( smallestBindPoint, bufferBinding.bindPoint );
        }

        if ( smallestBindPoint > uboWriteDescriptorSet.dstBinding ) {
            uboWriteDescriptorSet.dstBinding = smallestBindPoint;
        }

        uboWriteDescriptorSet.pBufferInfo = uboDesc;
    }

    // Generic Buffers
    if ( bindingCount[2] > 0u ) {
        VkWriteDescriptorSet& tboWriteDescriptorSet = descriptorSetUpdates[descriptorSetUpdateCount++];

        tboWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        tboWriteDescriptorSet.pNext = nullptr;
        tboWriteDescriptorSet.dstSet = resList.descriptorSet[2];
        tboWriteDescriptorSet.dstBinding = 0u;
        tboWriteDescriptorSet.dstArrayElement = 0u;
        tboWriteDescriptorSet.descriptorCount = bindingCount[2];
        tboWriteDescriptorSet.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;

        VkBufferView tboDesc[MAX_RES_COUNT];

        uint32_t smallestBindPoint = ( MAX_RES_COUNT - 1 );
        for ( uint32_t i = 0; i < bindingCount[2]; i++ ) {
            const auto& bufferBinding = description.buffers[i];
            tboDesc[i] = bufferBinding.resource->bufferView;
            smallestBindPoint = nya::maths::min( smallestBindPoint, bufferBinding.bindPoint );
        }

        if ( smallestBindPoint > tboWriteDescriptorSet.dstBinding ) {
            tboWriteDescriptorSet.dstBinding = smallestBindPoint;
        }

        tboWriteDescriptorSet.pTexelBufferView = tboDesc;
    }

    // UAV Textures
    if ( bindingCount[3] > 0u ) {
        VkWriteDescriptorSet& stboWriteDescriptorSet = descriptorSetUpdates[descriptorSetUpdateCount++];

        stboWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        stboWriteDescriptorSet.pNext = nullptr;
        stboWriteDescriptorSet.dstSet = resList.descriptorSet[3];
        stboWriteDescriptorSet.dstArrayElement = 0u;
        stboWriteDescriptorSet.descriptorCount = bindingCount[3];
        stboWriteDescriptorSet.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        stboWriteDescriptorSet.dstBinding = 0u;

        VkBufferView stboDesc[MAX_RES_COUNT];

        uint32_t stboBindingIndex = 0u;
        uint32_t smallestBindPoint = ( MAX_RES_COUNT - 1 );
        for ( uint32_t i = 0; i < MAX_RES_COUNT; i++ ) {
            const auto& bufferBinding = description.uavBuffers[i];
            if ( bufferBinding.resource->bufferView == nullptr ) {
                continue;
            }

            stboDesc[stboBindingIndex] = bufferBinding.resource->bufferView;

            stboBindingIndex++;
            smallestBindPoint = nya::maths::min( smallestBindPoint, bufferBinding.bindPoint );

            if ( stboBindingIndex >= bindingCount[3] ) {
                break;
            }
        }

        if ( smallestBindPoint > stboWriteDescriptorSet.dstBinding ) {
            stboWriteDescriptorSet.dstBinding = smallestBindPoint;
        }

        stboWriteDescriptorSet.pTexelBufferView = stboDesc;
    }

    // UAV Buffers
    if ( bindingCount[4] > 0u ) {
        VkWriteDescriptorSet& sboWriteDescriptorSet = descriptorSetUpdates[descriptorSetUpdateCount++];

        sboWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        sboWriteDescriptorSet.pNext = nullptr;
        sboWriteDescriptorSet.dstSet = resList.descriptorSet[4];
        sboWriteDescriptorSet.dstArrayElement = 0u;
        sboWriteDescriptorSet.descriptorCount = bindingCount[4];
        sboWriteDescriptorSet.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        sboWriteDescriptorSet.dstBinding = 0u;

        VkDescriptorBufferInfo sboDesc[MAX_RES_COUNT];

        uint32_t sboBindingIndex = 0u;
        uint32_t smallestBindPoint = ( MAX_RES_COUNT - 1 );
        for ( uint32_t i = 0; i < MAX_RES_COUNT; i++ ) {
            const auto& bufferBinding = description.uavBuffers[i];

            if ( bufferBinding.resource->bufferView != nullptr ) {
                continue;
            }

            sboDesc[sboBindingIndex].buffer = bufferBinding.resource->bufferObject;
            sboDesc[sboBindingIndex].offset = 0ull;
            sboDesc[sboBindingIndex].range = VK_WHOLE_SIZE;

            sboBindingIndex++;
            smallestBindPoint = nya::maths::min( smallestBindPoint, bufferBinding.bindPoint );

            if ( sboBindingIndex >= bindingCount[4] ) {
                break;
            }
        }

        if ( smallestBindPoint > sboWriteDescriptorSet.dstBinding ) {
            sboWriteDescriptorSet.dstBinding = smallestBindPoint;
        }

        sboWriteDescriptorSet.pBufferInfo = sboDesc;
    }

    // Submit descriptors set updates
    vkUpdateDescriptorSets( renderContext->device, descriptorSetUpdateCount, descriptorSetUpdates, 0u, nullptr );

    return resList;
}
*/

void CommandList::bindResourceList( const PipelineState* pipelineState, const ResourceList& resourceList )
{
    vkCmdBindDescriptorSets( 
        CommandListObject->cmdBuffer, 
        CommandListObject->resourcesBindPoint, 
        pipelineState->layout,
        0u, 
        4u,
        pipelineState->descriptorSet,
        0u, 
        nullptr
    );
}
#endif
