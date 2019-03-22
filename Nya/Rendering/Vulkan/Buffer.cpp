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

#include <vulkan/vulkan.h>

Buffer* RenderDevice::createBuffer( const BufferDesc& description, const void* initialData )
{
    Buffer* buffer = nya::core::allocate<Buffer>( memoryAllocator );

    VkBuffer nativeBuffer = nullptr;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.size = description.size;
    bufferInfo.usage = 0;

    if ( description.type == BufferDesc::CONSTANT_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    } else if ( description.type == BufferDesc::DYNAMIC_VERTEX_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    } else if ( description.type == BufferDesc::VERTEX_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    } else if ( description.type == BufferDesc::DYNAMIC_INDICE_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    } else if ( description.type == BufferDesc::INDICE_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    } else if ( description.type == BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    } else if ( description.type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_1D
        || description.type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D
        || description.type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_3D ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }

    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto bufferCreationResult = vkCreateBuffer( renderContext->device, &bufferInfo, nullptr, &nativeBuffer );
    if ( bufferCreationResult != VK_SUCCESS ) {
        NYA_CERR << "Failed to create buffer description! (error code: " << bufferCreationResult << ")" << std::endl;
        return nullptr;
    }

    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;

    buffer->bufferObject = nativeBuffer;
    buffer->stride = description.stride;

    return buffer;
}

void RenderDevice::destroyBuffer( Buffer* buffer )
{
    vkDestroyBuffer( renderContext->device, buffer->bufferObject, nullptr );
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
