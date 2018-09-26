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

#if FLAN_VULKAN
#include "Texture.h"
#include "RenderContext.h"
#include "CommandList.h"

#include "ImageFormat.h"

#include <vector>

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

NativeTextureObject* flan::rendering::CreateTexture1DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
{
    NativeTextureObject* textureObject = new NativeTextureObject();
    return textureObject;
}

uint32_t findMemoryType( NativeRenderContext* nativeRenderContext, uint32_t typeFilter, VkMemoryPropertyFlags properties )
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties( nativeRenderContext->physicalDevice, &memProperties );

    for ( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ ) {
        if ( ( typeFilter & ( 1 << i ) ) && ( memProperties.memoryTypes[i].propertyFlags & properties ) == properties ) {
            return i;
        }
    }
}

NativeTextureObject* flan::rendering::CreateTexture2DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
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
    imageInfo.format = VK_IMAGE_FORMAT[description.format];


    VkImage imageObject = {};

    auto operationResult = vkCreateImage( nativeRenderContext->device, &imageInfo, nullptr, &imageObject );   
    if ( operationResult != VK_SUCCESS ) {
        FLAN_CERR << "Failed to create VkImage (error code: " << operationResult << ")" << std::endl;
        return nullptr;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements( nativeRenderContext->device, imageObject, &memRequirements );

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType( nativeRenderContext, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    NativeTextureObject* nativeTextureObject = new NativeTextureObject();
    return nativeTextureObject;
}

NativeTextureObject* flan::rendering::CreateTexture3DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
{
    NativeTextureObject* nativeTextureObject = new NativeTextureObject();
    return nativeTextureObject;
}

void flan::rendering::DestroyTextureImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject )
{

}

void flan::rendering::BindTextureImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo )
{
}

void flan::rendering::UnbindTextureImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo )
{
}

void flan::rendering::BindTextureCmdImpl( NativeCommandList* nativeCmdList, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo )
{
}

void flan::rendering::UnbindTextureCmdImpl( NativeCommandList* nativeCmdList, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo )
{
}

void flan::rendering::SetTextureDebugNameImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const std::string& debugName )
{
}

void flan::rendering::RetrieveTextureTexelsLDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, std::vector<uint8_t>& texels )
{
}

void flan::rendering::RetrieveTextureLayerTexelsLDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, const unsigned int layerIndex, const unsigned int mipLevel, std::vector<uint8_t>& texels )
{
}

void flan::rendering::RetrieveTextureTexelsHDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, std::vector<float>& texels )
{
}
#endif
