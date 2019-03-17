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

#include <vulkan/vulkan.h>

VkImageCreateFlags GetTextureCreateFlags( const TextureDescription& description )
{
    VkImageCreateFlags flagset = 0;

    if ( description.flags.isCubeMap == 1 ) {
        flagset |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    if ( description.arraySize > 1 ) {
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

    return ~0;
}

Texture* CreateTexture( RenderContext* renderContext, Texture* preallocatedTexture, const VkImageType imageType, const TextureDescription& description, const void* initialData, const size_t initialDataSize )
{
    preallocatedTexture->imageFormat = VK_IMAGE_FORMAT[description.format];

    // Create image descriptor object
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.flags = GetTextureCreateFlags( description );
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>( description.width );
    imageInfo.extent.height = static_cast<uint32_t>( description.height );
    imageInfo.extent.depth = description.depth;
    imageInfo.mipLevels = description.mipCount;
    imageInfo.arrayLayers = description.arraySize;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.format = preallocatedTexture->imageFormat;

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

    // Create resource view
    preallocatedTexture->imageView = CreateImageView( renderContext->device, description, preallocatedTexture->image );

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
}

void RenderDevice::setDebugMarker( Texture* texture, const char* objectName )
{
    VkDebugMarkerObjectNameInfoEXT dbgMarkerObjName = {};
    dbgMarkerObjName.sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
    dbgMarkerObjName.pNext = nullptr;
    dbgMarkerObjName.objectType = VkDebugReportObjectTypeEXT::VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT;
    dbgMarkerObjName.object = reinterpret_cast<uint64_t>( texture->image );
    dbgMarkerObjName.pObjectName = objectName;

    vkDebugMarkerSetObjectNameEXT( renderContext->device, &dbgMarkerObjName );
}
#endif
