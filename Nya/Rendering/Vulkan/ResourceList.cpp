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
#include "Texture.h"
#include "RenderTarget.h"

#include <Maths/Helpers.h>

#include <vulkan/vulkan.h>

using namespace nya::rendering;

void RenderDevice::updateResourceList( PipelineState* pipelineState, const ResourceList& resourceList )
{
    VkWriteDescriptorSet writeDescriptorSets[64];
    VkDescriptorImageInfo descriptorImageInfos[64];
    VkDescriptorBufferInfo descriptorBufferInfos[64];

    for ( uint32_t i = 0u; i < pipelineState->descriptorBindingCount; i++ ) {
        switch ( pipelineState->descriptorBindingTypes[i] ) {
        case VK_DESCRIPTOR_TYPE_SAMPLER: {
            VkDescriptorImageInfo& descriptorImageInfo = descriptorImageInfos[i];
            descriptorImageInfo.sampler = resourceList.resource[i].sampler->samplerState;

            VkWriteDescriptorSet& samplerWriteDescriptorSet = writeDescriptorSets[i];
            samplerWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            samplerWriteDescriptorSet.pNext = nullptr;
            samplerWriteDescriptorSet.dstSet = pipelineState->descriptorSet;
            samplerWriteDescriptorSet.dstBinding = pipelineState->descriptorBindings[i];
            samplerWriteDescriptorSet.dstArrayElement = 0u;
            samplerWriteDescriptorSet.descriptorCount = 1u;
            samplerWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            samplerWriteDescriptorSet.pImageInfo = &descriptorImageInfos[i];
        } break;

        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
            VkDescriptorImageInfo& descriptorImageInfo = descriptorImageInfos[i];
            descriptorImageInfo.imageView = resourceList.resource[i].renderTarget->textureRenderTargetView;
            descriptorImageInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;

            VkWriteDescriptorSet& samplerWriteDescriptorSet = writeDescriptorSets[i];
            samplerWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            samplerWriteDescriptorSet.pNext = nullptr;
            samplerWriteDescriptorSet.dstSet = pipelineState->descriptorSet;
            samplerWriteDescriptorSet.dstBinding = pipelineState->descriptorBindings[i];
            samplerWriteDescriptorSet.dstArrayElement = 0u;
            samplerWriteDescriptorSet.descriptorCount = 1u;
            samplerWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            samplerWriteDescriptorSet.pImageInfo = &descriptorImageInfos[i];
        } break;

        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: {
            VkDescriptorImageInfo& descriptorImageInfo = descriptorImageInfos[i];
            descriptorImageInfo.imageView = resourceList.resource[i].texture->imageView;
            descriptorImageInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet& samplerWriteDescriptorSet = writeDescriptorSets[i];
            samplerWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            samplerWriteDescriptorSet.pNext = nullptr;
            samplerWriteDescriptorSet.dstSet = pipelineState->descriptorSet;
            samplerWriteDescriptorSet.dstBinding = pipelineState->descriptorBindings[i];
            samplerWriteDescriptorSet.dstArrayElement = 0u;
            samplerWriteDescriptorSet.descriptorCount = 1u;
            samplerWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            samplerWriteDescriptorSet.pImageInfo = &descriptorImageInfos[i];
        } break;

        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
            VkDescriptorBufferInfo& descriptorBufferInfo = descriptorBufferInfos[i];
            descriptorBufferInfo.buffer = resourceList.resource[i].buffer->bufferObject;
            descriptorBufferInfo.offset = 0ull;
            descriptorBufferInfo.range = VK_WHOLE_SIZE;

            VkWriteDescriptorSet& uboWriteDescriptorSet = writeDescriptorSets[i];
            uboWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            uboWriteDescriptorSet.pNext = nullptr;
            uboWriteDescriptorSet.dstSet = pipelineState->descriptorSet;
            uboWriteDescriptorSet.dstBinding = pipelineState->descriptorBindings[i];
            uboWriteDescriptorSet.dstArrayElement = 0u;
            uboWriteDescriptorSet.descriptorCount = 1u;
            uboWriteDescriptorSet.descriptorType = pipelineState->descriptorBindingTypes[i];
            uboWriteDescriptorSet.pBufferInfo = &descriptorBufferInfos[i];
        } break;

        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: {
            VkWriteDescriptorSet& tboWriteDescriptorSet = writeDescriptorSets[i];
            tboWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            tboWriteDescriptorSet.pNext = nullptr;
            tboWriteDescriptorSet.dstSet = pipelineState->descriptorSet;
            tboWriteDescriptorSet.dstBinding = pipelineState->descriptorBindings[i];
            tboWriteDescriptorSet.dstArrayElement = 0u;
            tboWriteDescriptorSet.descriptorCount = 1u;
            tboWriteDescriptorSet.descriptorType = pipelineState->descriptorBindingTypes[i];
            tboWriteDescriptorSet.pTexelBufferView = &resourceList.resource[i].buffer->bufferView;
        } break;
        default:
            break;
        }
    }

    vkUpdateDescriptorSets( renderContext->device, pipelineState->descriptorBindingCount, writeDescriptorSets, 0u, nullptr );

}
#endif
