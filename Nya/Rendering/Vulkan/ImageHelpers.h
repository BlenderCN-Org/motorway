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
    VK_FORMAT_R32G32B32_UINT,
    VK_FORMAT_R32G32B32_SINT,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_A2R10G10B10_UNORM_PACK32,
    VK_FORMAT_A2R10G10B10_UINT_PACK32,
    VK_FORMAT_B10G11R11_UFLOAT_PACK32,

    VK_FORMAT_UNDEFINED,
    VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
    VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
    VK_FORMAT_R8G8B8A8_UINT,
    VK_FORMAT_R8G8B8A8_SNORM,
    VK_FORMAT_R8G8B8A8_SINT,

    VK_FORMAT_UNDEFINED,
    VK_FORMAT_R16G16_SFLOAT,
    VK_FORMAT_R16G16_UNORM,
    VK_FORMAT_R16G16_UINT,
    VK_FORMAT_R16G16_SNORM,
    VK_FORMAT_R16G16_SINT,

    VK_FORMAT_UNDEFINED,
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

static VkImageViewType GetViewType( const TextureDescription& description )
{
    switch ( description.dimension ) {
    case TextureDescription::DIMENSION_TEXTURE_1D:
        return ( ( description.arraySize > 1 ) ? VkImageViewType::VK_IMAGE_VIEW_TYPE_1D_ARRAY : VkImageViewType::VK_IMAGE_VIEW_TYPE_1D );

    case TextureDescription::DIMENSION_TEXTURE_2D:
    {
        if ( description.flags.isCubeMap == 1 ) {
            return ( ( description.arraySize > 1 ) ? VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE );
        } else {
            return ( ( description.arraySize > 1 ) ? VkImageViewType::VK_IMAGE_VIEW_TYPE_2D_ARRAY : VkImageViewType::VK_IMAGE_VIEW_TYPE_2D );
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

static VkImageView CreateImageView( VkDevice device, const TextureDescription& description, const VkImage image, const uint32_t sliceIndex = 0u, const uint32_t sliceCount = ~0u, const uint32_t mipIndex = 0u, const uint32_t mipCount = ~0u )
{
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
#endif
