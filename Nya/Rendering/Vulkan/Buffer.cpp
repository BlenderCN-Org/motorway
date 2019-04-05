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
#include "Buffer.h"

#include <Rendering/ImageFormat.h>

#include "RenderDevice.h"
#include "CommandList.h"

#include "Texture.h"
#include "ImageHelpers.h"

#include <vulkan/vulkan.h>
#include <string.h>

uint32_t _FindMemoryType( RenderContext* renderContext, const uint32_t typeFilter, const VkMemoryPropertyFlags properties )
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties( renderContext->physicalDevice, &memProperties );

    for ( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ ) {
        if ( ( typeFilter & ( 1 << i ) ) && ( memProperties.memoryTypes[i].propertyFlags & properties ) == properties ) {
            return i;
        }
    }

    return ~0u;
}

void _createBuffer(RenderContext* renderContext, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(renderContext->device, &bufferInfo, nullptr, &buffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(renderContext->device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = _FindMemoryType(renderContext, memRequirements.memoryTypeBits, properties);

    vkAllocateMemory(renderContext->device, &allocInfo, nullptr, &bufferMemory);
    vkBindBufferMemory(renderContext->device, buffer, bufferMemory, 0);
}

VkCommandBuffer _beginSingleTimeCommands(RenderContext* renderContext) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = renderContext->graphicsCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(renderContext->device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void _endSingleTimeCommands(RenderContext* renderContext, VkCommandBuffer commandBuffer) {
   vkEndCommandBuffer(commandBuffer);

   VkSubmitInfo submitInfo = {};
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &commandBuffer;

   vkQueueSubmit(renderContext->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
   vkQueueWaitIdle(renderContext->graphicsQueue);

   vkFreeCommandBuffers(renderContext->device, renderContext->graphicsCommandPool, 1, &commandBuffer);
}

void copyBuffer(RenderContext* renderContext, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = _beginSingleTimeCommands(renderContext);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    _endSingleTimeCommands(renderContext,commandBuffer);
}

Buffer* RenderDevice::createBuffer( const BufferDesc& description, const void* initialData )
{
    Buffer* buffer = nya::core::allocate<Buffer>( memoryAllocator );

    VkBuffer nativeBuffer = nullptr;
    VkMemoryPropertyFlags bufferMemoryFlags = 0;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.size = description.size;

    bufferInfo.usage = 0;
    if ( description.type == BufferDesc::CONSTANT_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    } else if ( description.type == BufferDesc::VERTEX_BUFFER
             || description.type == BufferDesc::DYNAMIC_VERTEX_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    } else if ( description.type == BufferDesc::DYNAMIC_INDICE_BUFFER
             || description.type == BufferDesc::INDICE_BUFFER) {
        bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    } else if ( description.type == BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER
             || description.type == BufferDesc::STRUCTURED_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    } else if ( description.type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_1D
             || description.type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D
             || description.type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_3D ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

        // For texel-based buffers, compute the buffer size manually
        bufferInfo.size = 0ull;

        const size_t formatSize = VK_IMAGE_FORMAT_SIZE[description.viewFormat];

        uint32_t width = description.width,
                 height = description.height;
        for ( uint32_t mipIdx = 0u; mipIdx < description.mipCount; mipIdx++ ) {
            bufferInfo.size += width * height * description.depth * formatSize;

            width = description.width >> 1;
            height = description.height >> 1;
        }
    } else if ( description.type == BufferDesc::GENERIC_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
        bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    if ( initialData != nullptr ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto bufferCreationResult = vkCreateBuffer( renderContext->device, &bufferInfo, nullptr, &nativeBuffer );
    if ( bufferCreationResult != VK_SUCCESS ) {
        NYA_CERR << "Failed to create buffer description! (error code: " << bufferCreationResult << ")" << std::endl;
        return nullptr;
    }

    // Allocate memory from the device
    VkDeviceMemory deviceMemory;

    VkMemoryRequirements bufferMemoryRequirements;
    vkGetBufferMemoryRequirements( renderContext->device, nativeBuffer, &bufferMemoryRequirements );

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = bufferMemoryRequirements.size;
    allocInfo.memoryTypeIndex = _FindMemoryType( renderContext, bufferMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

    VkResult allocationResult = vkAllocateMemory( renderContext->device, &allocInfo, nullptr, &deviceMemory );
    if ( allocationResult != VK_SUCCESS ) {
        NYA_CERR << "Failed to allocate VkBuffer memory (error code: " << allocationResult << ")" << std::endl;
        return nullptr;
    }

    vkBindBufferMemory( renderContext->device, nativeBuffer, deviceMemory, 0ull );

    buffer->deviceMemory = deviceMemory;
    buffer->bufferObject = nativeBuffer;
    buffer->stride = description.stride;

    VkBufferView bufferView = nullptr;
    if ( bufferInfo.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT
      || bufferInfo.usage & VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ) {
        VkBufferViewCreateInfo bufferViewDesc;
        bufferViewDesc.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        bufferViewDesc.pNext = nullptr;
        bufferViewDesc.flags = 0u;
        bufferViewDesc.buffer = nativeBuffer;
        bufferViewDesc.format = VK_IMAGE_FORMAT[description.viewFormat];
        bufferViewDesc.offset = 0ull;
        bufferViewDesc.range = VK_WHOLE_SIZE;

        vkCreateBufferView( renderContext->device, &bufferViewDesc, nullptr, &bufferView );
    }
    buffer->bufferView = bufferView;

    if ( initialData != nullptr ) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        _createBuffer(renderContext, bufferInfo.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(renderContext->device, stagingBufferMemory, 0, bufferInfo.size, 0, &data);
            memcpy(data, initialData, bufferInfo.size);
        vkUnmapMemory(renderContext->device, stagingBufferMemory);

        copyBuffer(renderContext, stagingBuffer, nativeBuffer, bufferInfo.size);

        vkDestroyBuffer(renderContext->device, stagingBuffer, nullptr);
        vkFreeMemory(renderContext->device, stagingBufferMemory, nullptr);
    }
    return buffer;
}

void RenderDevice::destroyBuffer( Buffer* buffer )
{
    vkDestroyBuffer( renderContext->device, buffer->bufferObject, nullptr );

    if ( buffer->bufferView != nullptr ) {
        vkDestroyBufferView( renderContext->device, buffer->bufferView, nullptr );
    }

    vkFreeMemory( renderContext->device, buffer->deviceMemory, nullptr );

    nya::core::free( memoryAllocator, buffer );
}

void RenderDevice::setDebugMarker( Buffer* buffer, const char* objectName )
{
    VkDebugMarkerObjectNameInfoEXT dbgMarkerObjName = {};
    dbgMarkerObjName.sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
    dbgMarkerObjName.pNext = nullptr;
    dbgMarkerObjName.objectType = VkDebugReportObjectTypeEXT::VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT;
    dbgMarkerObjName.object = reinterpret_cast<uint64_t>( buffer->bufferObject );
    dbgMarkerObjName.pObjectName = objectName;

    //vkDebugMarkerSetObjectNameEXT( renderContext->device, &dbgMarkerObjName );
}

void CommandList::bindVertexBuffer( const Buffer* buffer, const unsigned int bindIndex )
{
    constexpr VkDeviceSize OFFSETS = 0ull;

    vkCmdBindVertexBuffers( CommandListObject->cmdBuffer, bindIndex, 1u, &buffer->bufferObject, &OFFSETS );
}

void CommandList::bindIndiceBuffer( const Buffer* buffer )
{
    vkCmdBindIndexBuffer( CommandListObject->cmdBuffer, buffer->bufferObject, 0, ( buffer->stride == 4 ) ? VkIndexType::VK_INDEX_TYPE_UINT32 : VkIndexType::VK_INDEX_TYPE_UINT16 );
}

void CommandList::updateBuffer( Buffer* buffer, const void* data, const size_t dataSize )
{
    // TODO Implement me!
}

void CommandList::copyStructureCount( Buffer* srcBuffer, Buffer* dstBuffer, const unsigned int offset )
{
    // TODO Implement me!
}

void CommandList::drawInstancedIndirect( const Buffer* drawArgsBuffer, const unsigned int bufferDataOffset )
{
    vkCmdDrawIndirect( CommandListObject->cmdBuffer, drawArgsBuffer->bufferObject, bufferDataOffset, 1u, 0u );
}
#endif
