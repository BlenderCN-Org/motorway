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

#include "ImageHelpers.h"

#include "RenderDevice.h"
#include "CommandList.h"

#include "Texture.h"
#include <Maths/Helpers.h>

#include <vulkan/vulkan.h>
#include <string.h>

VkImageCreateFlags GetTextureCreateFlags( const TextureDescription& description )
{
    VkImageCreateFlags flagset = 0;

    if ( description.flags.isCubeMap == 1 ) {
        flagset |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    } else if ( description.arraySize > 1 ) {
        // NOTE Do not set VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT when VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT is set (not allowed by the specs.)
        flagset |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    }

    return flagset;
}

uint32_t FindMemoryType( RenderContext* renderContext, const uint32_t typeFilter, const VkMemoryPropertyFlags properties )
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

bool IsUsingCompressedFormat( const eImageFormat format )
{
    return format == IMAGE_FORMAT_BC1_TYPELESS
        || format == IMAGE_FORMAT_BC1_UNORM
        || format == IMAGE_FORMAT_BC1_UNORM_SRGB
        || format == IMAGE_FORMAT_BC2_TYPELESS
        || format == IMAGE_FORMAT_BC2_UNORM
        || format == IMAGE_FORMAT_BC2_UNORM_SRGB
        || format == IMAGE_FORMAT_BC3_TYPELESS
        || format == IMAGE_FORMAT_BC3_UNORM
        || format == IMAGE_FORMAT_BC3_UNORM_SRGB
        || format == IMAGE_FORMAT_BC4_TYPELESS
        || format == IMAGE_FORMAT_BC4_UNORM
        || format == IMAGE_FORMAT_BC4_SNORM
        || format == IMAGE_FORMAT_BC5_TYPELESS
        || format == IMAGE_FORMAT_BC5_UNORM
        || format == IMAGE_FORMAT_BC5_SNORM;
}

VkSampleCountFlagBits GetVkSampleCount( const uint32_t sampleCount )
{
    switch ( sampleCount ) {
    case 1:
        return VK_SAMPLE_COUNT_1_BIT;
    case 2:
        return VK_SAMPLE_COUNT_2_BIT;
    case 4:
        return VK_SAMPLE_COUNT_4_BIT;
    case 8:
        return VK_SAMPLE_COUNT_8_BIT;
    case 16:
        return VK_SAMPLE_COUNT_16_BIT;
    case 32:
        return VK_SAMPLE_COUNT_32_BIT;
    case 64:
        return VK_SAMPLE_COUNT_64_BIT;
    default:
        return VK_SAMPLE_COUNT_1_BIT;
    }
}

void createBuffer(RenderContext* renderContext, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
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
    allocInfo.memoryTypeIndex = FindMemoryType(renderContext, memRequirements.memoryTypeBits, properties);

    vkAllocateMemory(renderContext->device, &allocInfo, nullptr, &bufferMemory);
    vkBindBufferMemory(renderContext->device, buffer, bufferMemory, 0);
}

VkCommandBuffer beginSingleTimeCommands(RenderContext* renderContext) {
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

void endSingleTimeCommands(RenderContext* renderContext, VkCommandBuffer commandBuffer) {
   vkEndCommandBuffer(commandBuffer);

   VkSubmitInfo submitInfo = {};
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &commandBuffer;

   vkQueueSubmit(renderContext->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
   vkQueueWaitIdle(renderContext->graphicsQueue);

   vkFreeCommandBuffers(renderContext->device, renderContext->graphicsCommandPool, 1, &commandBuffer);
}

void transitionImageLayout(RenderContext* renderContext, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands( renderContext );

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(renderContext, commandBuffer);
}

void copyBufferToImage(RenderContext* renderContext, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
   VkCommandBuffer commandBuffer = beginSingleTimeCommands( renderContext );

   VkBufferImageCopy region = {};
   region.bufferOffset = 0;
   region.bufferRowLength = 0;
   region.bufferImageHeight = 0;
   region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   region.imageSubresource.mipLevel = 0;
   region.imageSubresource.baseArrayLayer = 0;
   region.imageSubresource.layerCount = 1;
   region.imageOffset = {0, 0, 0};
   region.imageExtent = {
       width,
       height,
       1
   };

   vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

   endSingleTimeCommands(renderContext, commandBuffer);
}

Texture* CreateTexture( RenderContext* renderContext, Texture* preallocatedTexture, const VkImageType imageType, const TextureDescription& description, const void* initialData, const size_t initialDataSize )
{
    preallocatedTexture->imageFormat = VK_IMAGE_FORMAT[description.format];

    // Create image descriptor object
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.flags = GetTextureCreateFlags( description );

    if ( imageType == VK_IMAGE_TYPE_2D && description.arraySize > 1 && description.flags.isCubeMap == 0 ) {
        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        imageInfo.arrayLayers = description.arraySize * description.depth;
        imageInfo.extent.depth = 1u;
    } else {
        imageInfo.imageType = imageType;

        if ( description.flags.isCubeMap == 0)
            imageInfo.arrayLayers = ( imageType == VK_IMAGE_TYPE_3D ) ? 1u : description.arraySize; // if pCreateInfo->imageType is VK_IMAGE_TYPE_3D, pCreateInfo->arrayLayers must be 1.
        else
            imageInfo.arrayLayers = description.arraySize * description.depth; // if pCreateInfo->imageType is VK_IMAGE_TYPE_3D, pCreateInfo->arrayLayers must be 1.

        imageInfo.extent.depth = ( imageType == VK_IMAGE_TYPE_2D ) ? 1u : description.depth; // if pCreateInfo->imageType is VK_IMAGE_TYPE_2D, pCreateInfo->extent.depth must be 1.
    }

    imageInfo.extent.width = static_cast<uint32_t>( description.width );
    imageInfo.extent.height = static_cast<uint32_t>( description.height );
    imageInfo.mipLevels = description.mipCount;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = GetVkSampleCount( description.samplerCount );
    imageInfo.format = preallocatedTexture->imageFormat;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    // TODO This is quite lazy and not efficient?
    if ( !IsUsingCompressedFormat( description.format ) ) {
        if ( description.flags.isDepthResource == 1 ) {
            imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        } else {
            imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }

    VkImage imageObject = nullptr;

    VkResult operationResult = vkCreateImage( renderContext->device, &imageInfo, nullptr, &imageObject );
    if ( operationResult != VK_SUCCESS ) {
        NYA_CERR << "Failed to create VkImage (error code: " << operationResult << ")" << std::endl;
        return nullptr;
    }

    preallocatedTexture->image = imageObject;

    // Allocate memory
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements( renderContext->device, imageObject, &memRequirements );

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType( renderContext, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    VkResult allocationResult = vkAllocateMemory( renderContext->device, &allocInfo, nullptr, &preallocatedTexture->imageMemory );
    if ( allocationResult != VK_SUCCESS ) {
        NYA_CERR << "Failed to allocate VkImage memory (error code: " << allocationResult << ")" << std::endl;
        return nullptr;
    }

    // Bind allocated memory to descriptor
    vkBindImageMemory( renderContext->device, preallocatedTexture->image, preallocatedTexture->imageMemory, 0ull );

    // Upload inital data (if provided)
    if ( initialData != nullptr ) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        // initialDataSize is the scanline size; not the actual buffer size
        size_t bufferSize = initialDataSize * description.height;
        createBuffer(renderContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(renderContext->device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, initialData, bufferSize);
        vkUnmapMemory(renderContext->device, stagingBufferMemory);

        // Create resource view
        preallocatedTexture->imageView = CreateImageView( renderContext->device, description, preallocatedTexture->image );

        transitionImageLayout(renderContext, imageObject, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copyBufferToImage(renderContext, stagingBuffer, imageObject, description.width, description.height);
        transitionImageLayout(renderContext, imageObject, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(renderContext->device, stagingBuffer, nullptr);
        vkFreeMemory(renderContext->device, stagingBufferMemory, nullptr);
    }

    return preallocatedTexture;
}

Texture* RenderDevice::createTexture1D( const TextureDescription& description, const void* initialData, const size_t initialDataSize )
{
    return CreateTexture( renderContext, nya::core::allocate<Texture>( memoryAllocator ), VK_IMAGE_TYPE_1D, description, initialData, initialDataSize );
}

Texture* RenderDevice::createTexture2D( const TextureDescription& description, const void* initialData, const size_t initialDataSize )
{
    return CreateTexture( renderContext, nya::core::allocate<Texture>( memoryAllocator ), VK_IMAGE_TYPE_2D, description, initialData, initialDataSize );
}

Texture* RenderDevice::createTexture3D( const TextureDescription& description, const void* initialData, const size_t initialDataSize )
{
    return CreateTexture( renderContext, nya::core::allocate<Texture>( memoryAllocator ), VK_IMAGE_TYPE_3D, description, initialData, initialDataSize );
}

void RenderDevice::destroyTexture( Texture* texture )
{
    vkDestroyImage( renderContext->device, texture->image, nullptr );
    vkDestroyImageView( renderContext->device, texture->imageView, nullptr );
    vkFreeMemory( renderContext->device, texture->imageMemory, nullptr );

    nya::core::free( memoryAllocator, texture );
}

void RenderDevice::setDebugMarker( Texture* texture, const char* objectName )
{
    VkDebugMarkerObjectNameInfoEXT dbgMarkerObjName = {};
    dbgMarkerObjName.sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
    dbgMarkerObjName.pNext = nullptr;
    dbgMarkerObjName.objectType = VkDebugReportObjectTypeEXT::VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT;
    dbgMarkerObjName.object = reinterpret_cast<uint64_t>( texture->image );
    dbgMarkerObjName.pObjectName = objectName;

    //vkDebugMarkerSetObjectNameEXT( renderContext->device, &dbgMarkerObjName );
}
#endif
