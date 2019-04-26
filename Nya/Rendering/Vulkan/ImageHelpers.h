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

#include <Rendering/ImageFormat.h>

#if NYA_VULKAN
#include <vulkan/vulkan.h>
#include <Rendering/RenderDevice.h>

#include "RenderDevice.h"

// Channel * PerChannelSize
static constexpr size_t VK_IMAGE_FORMAT_SIZE[IMAGE_FORMAT_COUNT] =
{
    0ull,
    0ull,
    16ull,
    16ull,
    16ull,
    0ull,
    12ull,
    12ull,
    12ull,
    0ull,
    8ull,
    8ull,
    8ull,
    8ull,
    8ull,
    0ull,
    8ull,
    8ull,
    8ull,
    0ull,
    8ull,
    0ull,
    0ull,
    0ull,
    4ull,
    4ull,
    4ull,
    0ull,
    4ull,
    4ull,
    4ull,
    4ull,
    4ull,
    0ull,
    4ull,
    4ull,
    4ull,
    4ull,
    4ull,
    4ull,
    4ull,
    4ull,
    4ull,
    4ull,

    4ull,
    0ull,
    0ull,
    0ull,

    0ull,
    2ull,
    2ull,
    2ull,
    2ull,

    0ull,
    2ull,

    2ull,
    2ull,
    2ull,
    2ull,
    2ull,

    0ull,
    1ull,
    1ull,
    1ull,
    1ull,

    0ull,
    0ull,
    0ull,
    0ull,
    0ull,

    8ull,
    8ull,
    8ull,

    16ull,
    16ull,
    16ull,

    16ull,
    16ull,
    16ull,

    8ull,
    8ull,
    8ull,

    16ull,
    16ull,
    16ull,

    2ull,
    2ull,

    4ull,
    4ull,

    0ull,
    0ull,

    4ull,
    0ull,
    0ull,

    0ull,
    16ull,
    16ull,

    0ull,
    16ull,
    16ull
};

static constexpr VkFormat VK_IMAGE_FORMAT[IMAGE_FORMAT_COUNT] =
{
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_R32G32B32A32_SFLOAT,
    VK_FORMAT_R32G32B32A32_UINT,
    VK_FORMAT_R32G32B32A32_SINT,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32B32_UINT,
    VK_FORMAT_R32G32B32_SINT,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_R16G16B16A16_SFLOAT,
    VK_FORMAT_R16G16B16A16_UNORM,
    VK_FORMAT_R16G16B16A16_UINT,
    VK_FORMAT_R16G16B16A16_SNORM,
    VK_FORMAT_R16G16B16A16_SINT,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_R32G32_SFLOAT,
    VK_FORMAT_R32G32_UINT,
    VK_FORMAT_R32G32_SINT,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_A2R10G10B10_UNORM_PACK32,
    VK_FORMAT_A2R10G10B10_UINT_PACK32,
    VK_FORMAT_B10G11R11_UFLOAT_PACK32,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_R8G8B8A8_SRGB,
    VK_FORMAT_R8G8B8A8_UINT,
    VK_FORMAT_R8G8B8A8_SNORM,
    VK_FORMAT_R8G8B8A8_SINT,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_R16G16_SFLOAT,
    VK_FORMAT_R16G16_UNORM,
    VK_FORMAT_R16G16_UINT,
    VK_FORMAT_R16G16_SNORM,
    VK_FORMAT_R16G16_SINT,
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_R32_SFLOAT,
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_R32_UINT,
    VK_FORMAT_R32_SINT,

    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_UNDEFINED,

    VK_FORMAT_UNDEFINED,
    VK_FORMAT_R8G8_UNORM,
    VK_FORMAT_R8G8_UINT,
    VK_FORMAT_R8G8_SNORM,
    VK_FORMAT_R8G8_SINT,

    VK_FORMAT_UNDEFINED,
    VK_FORMAT_R16_SFLOAT,

    VK_FORMAT_D16_UNORM,
    VK_FORMAT_R16_UNORM,
    VK_FORMAT_R16_UINT,
    VK_FORMAT_R16_SNORM,
    VK_FORMAT_R16_SINT,

    VK_FORMAT_UNDEFINED,
    VK_FORMAT_R8_UNORM,
    VK_FORMAT_R8_UINT,
    VK_FORMAT_R8_SNORM,
    VK_FORMAT_R8_SINT,

    VK_FORMAT_UNDEFINED,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_UNDEFINED,

    VK_FORMAT_BC1_RGB_UNORM_BLOCK,
    VK_FORMAT_BC1_RGB_UNORM_BLOCK,
    VK_FORMAT_BC1_RGB_SRGB_BLOCK,

    VK_FORMAT_BC2_UNORM_BLOCK,
    VK_FORMAT_BC2_UNORM_BLOCK,
    VK_FORMAT_BC2_SRGB_BLOCK,

    VK_FORMAT_BC3_UNORM_BLOCK,
    VK_FORMAT_BC3_UNORM_BLOCK,
    VK_FORMAT_BC3_SRGB_BLOCK,

    VK_FORMAT_BC4_UNORM_BLOCK,
    VK_FORMAT_BC4_UNORM_BLOCK,
    VK_FORMAT_BC4_SNORM_BLOCK,

    VK_FORMAT_BC5_UNORM_BLOCK,
    VK_FORMAT_BC5_UNORM_BLOCK,
    VK_FORMAT_BC5_SNORM_BLOCK,

    VK_FORMAT_B5G6R5_UNORM_PACK16,
    VK_FORMAT_B5G5R5A1_UNORM_PACK16,

    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_B8G8R8A8_UNORM,
        
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_UNDEFINED,

    VK_FORMAT_B8G8R8A8_SRGB,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_UNDEFINED,

    VK_FORMAT_UNDEFINED,
    VK_FORMAT_BC6H_UFLOAT_BLOCK,
    VK_FORMAT_BC6H_SFLOAT_BLOCK,

    VK_FORMAT_UNDEFINED,
    VK_FORMAT_BC7_UNORM_BLOCK,
    VK_FORMAT_BC7_SRGB_BLOCK
};

static VkImageViewType GetViewType( const TextureDescription& description, const bool perSliceView = false )
{
    switch ( description.dimension ) {
    case TextureDescription::DIMENSION_TEXTURE_1D:
        return ( ( description.arraySize > 1 && !perSliceView ) ? VkImageViewType::VK_IMAGE_VIEW_TYPE_1D_ARRAY : VkImageViewType::VK_IMAGE_VIEW_TYPE_1D );

    case TextureDescription::DIMENSION_TEXTURE_2D:
    {
        if ( description.flags.isCubeMap == 1 && !perSliceView ) {
            return ( ( description.arraySize > 1 ) ? VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE );
        } else {
            return ( ( description.arraySize > 1 && !perSliceView ) ? VkImageViewType::VK_IMAGE_VIEW_TYPE_2D_ARRAY : VkImageViewType::VK_IMAGE_VIEW_TYPE_2D );
        }
    } break;

    case TextureDescription::DIMENSION_TEXTURE_3D:
    {
        return  VkImageViewType::VK_IMAGE_VIEW_TYPE_3D;
    }

    default:
        return VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
    }
}

static VkSampleCountFlagBits GetVkSampleCount( const uint32_t sampleCount )
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

static VkImageView CreateImageView( VkDevice device, const TextureDescription& description, const VkImage image, const uint32_t sliceIndex = 0u, const uint32_t sliceCount = ~0u, const uint32_t mipIndex = 0u, const uint32_t mipCount = ~0u )
{
    const bool isPerSliceView = ( sliceIndex != 0 );

    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0u;
    createInfo.image = image;
    createInfo.viewType = GetViewType( description );
    createInfo.format = VK_IMAGE_FORMAT[description.format];
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = ( ( description.flags.isDepthResource ) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT );
    createInfo.subresourceRange.baseMipLevel = mipIndex;
    createInfo.subresourceRange.levelCount = ( ( mipCount == ~0u ) ? description.mipCount : mipCount );
    createInfo.subresourceRange.baseArrayLayer = sliceIndex;
    createInfo.subresourceRange.layerCount = ( ( sliceCount == ~0u ) ? description.arraySize : sliceCount );

    VkImageView imageView;
    vkCreateImageView( device, &createInfo, nullptr, &imageView );

    return imageView;
}

namespace
{
    static VkCommandBuffer BeginSingleTimeCommands( RenderContext* renderContext )
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = renderContext->graphicsCommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers( renderContext->device, &allocInfo, &commandBuffer );

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer( commandBuffer, &beginInfo );

        return commandBuffer;
    }

    void EndSingleTimeCommands( RenderContext* renderContext, VkCommandBuffer commandBuffer )
    {
        vkEndCommandBuffer( commandBuffer );

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit( renderContext->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
        vkQueueWaitIdle( renderContext->graphicsQueue );

        vkFreeCommandBuffers( renderContext->device, renderContext->graphicsCommandPool, 1, &commandBuffer );
    }
}

static void TransitionImageLayout( RenderContext* renderContext, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout )
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands( renderContext );

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

    if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {   
        NYA_DEV_ASSERT( false, "Invalid layout transition! (oldLayout: %X, nextLayout: %X)", oldLayout, newLayout );
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    EndSingleTimeCommands( renderContext, commandBuffer );
}

static void SetImageLayout( VkCommandBuffer cmdBuffer, VkImage image,
                            VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
                            VkPipelineStageFlags srcStages,
                            VkPipelineStageFlags destStages )
{
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.oldLayout = oldImageLayout;
    barrier.newLayout = newImageLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    switch ( oldImageLayout ) {
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;

    default:
        break;
    }

    switch ( newImageLayout ) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    default:
        break;
    }

    vkCmdPipelineBarrier( cmdBuffer, srcStages, destStages, 0, 0, nullptr, 0, nullptr, 1, &barrier );
}
#endif
