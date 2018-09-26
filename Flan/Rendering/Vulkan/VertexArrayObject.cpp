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
#include "VertexArrayObject.h"

#include "RenderContext.h"

VkFormat GetVertexInputEntryFormat( const VertexLayoutEntry& entry )
{
    switch ( entry.format ) {
    case VertexLayoutEntry::FORMAT_BYTE:
        if ( entry.dimension == VertexLayoutEntry::DIMENSION_X ) {
            return VkFormat::VK_FORMAT_R8_SINT;
        } else if ( entry.dimension == VertexLayoutEntry::DIMENSION_XY ) {
            return VkFormat::VK_FORMAT_R8G8_SINT;
        } else if ( entry.dimension == VertexLayoutEntry::DIMENSION_XYZ ) {
            return VkFormat::VK_FORMAT_R8G8B8_SINT;
        } else {
            return VkFormat::VK_FORMAT_R8G8B8A8_SINT;
        }
        break;

    case VertexLayoutEntry::FORMAT_SHORT:
        if ( entry.dimension == VertexLayoutEntry::DIMENSION_X ) {
            return VkFormat::VK_FORMAT_R16_SINT;
        } else if ( entry.dimension == VertexLayoutEntry::DIMENSION_XY ) {
            return VkFormat::VK_FORMAT_R16G16_SINT;
        } else if ( entry.dimension == VertexLayoutEntry::DIMENSION_XYZ ) {
            return VkFormat::VK_FORMAT_R16G16B16_SINT;
        } else {
            return VkFormat::VK_FORMAT_R16G16B16A16_SINT;
        }
        break;

    case VertexLayoutEntry::FORMAT_INT:
        if ( entry.dimension == VertexLayoutEntry::DIMENSION_X ) {
            return VkFormat::VK_FORMAT_R32_SINT;
        } else if ( entry.dimension == VertexLayoutEntry::DIMENSION_XY ) {
            return VkFormat::VK_FORMAT_R32G32_SINT;
        } else if ( entry.dimension == VertexLayoutEntry::DIMENSION_XYZ ) {
            return VkFormat::VK_FORMAT_R32G32B32_SINT;
        } else {
            return VkFormat::VK_FORMAT_R32G32B32A32_SINT;
        }
        break;

    case VertexLayoutEntry::FORMAT_HALF_FLOAT:
        if ( entry.dimension == VertexLayoutEntry::DIMENSION_X ) {
            return VkFormat::VK_FORMAT_R16_SFLOAT;
        } else if ( entry.dimension == VertexLayoutEntry::DIMENSION_XY ) {
            return VkFormat::VK_FORMAT_R16G16_SFLOAT;
        } else if ( entry.dimension == VertexLayoutEntry::DIMENSION_XYZ ) {
            return VkFormat::VK_FORMAT_R16G16B16_SFLOAT;
        } else {
            return VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
        }
        break;

    case VertexLayoutEntry::FORMAT_FLOAT:
        if ( entry.dimension == VertexLayoutEntry::DIMENSION_X ) {
            return VkFormat::VK_FORMAT_R32_SFLOAT;
        } else if ( entry.dimension == VertexLayoutEntry::DIMENSION_XY ) {
            return VkFormat::VK_FORMAT_R32G32_SFLOAT;
        } else if ( entry.dimension == VertexLayoutEntry::DIMENSION_XYZ ) {
            return VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
        } else {
            return VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
        }
        break;

    case VertexLayoutEntry::FORMAT_DOUBLE:
        if ( entry.dimension == VertexLayoutEntry::DIMENSION_X ) {
            return VkFormat::VK_FORMAT_R64_SFLOAT;
        } else if ( entry.dimension == VertexLayoutEntry::DIMENSION_XY ) {
            return VkFormat::VK_FORMAT_R64G64_SFLOAT;
        } else if ( entry.dimension == VertexLayoutEntry::DIMENSION_XYZ ) {
            return VkFormat::VK_FORMAT_R64G64B64_SFLOAT;
        } else {
            return VkFormat::VK_FORMAT_R64G64B64A64_SFLOAT;
        }
        break;

    default:
        return VkFormat::VK_FORMAT_UNDEFINED;
    }

    return VkFormat::VK_FORMAT_UNDEFINED;
}

NativeVertexArrayObject* flan::rendering::CreateVertexArrayImpl( NativeRenderContext* nativeRenderContext, Buffer* vbo, Buffer* ibo )
{
    VkPipelineVertexInputStateCreateInfo vertexInputInfos;
    vertexInputInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfos.pNext = VK_NULL_HANDLE;
    vertexInputInfos.flags = 0;

    return new NativeVertexArrayObject();
}

void flan::rendering::DestroyVertexArrayImpl( NativeRenderContext* nativeRenderContext, NativeVertexArrayObject* vertexArrayObject )
{

}

void flan::rendering::BindVertexArrayCmdImpl( NativeCommandList* nativeCmdList, NativeVertexArrayObject* vertexArrayObject )
{

}

void flan::rendering::SetVertexArrayVertexLayoutImpl( NativeRenderContext* nativeRenderContext, NativeVertexArrayObject* vertexArrayObject, const VertexLayout_t& vertexLayout )
{
    for ( auto& entry : vertexLayout ) {
        VkVertexInputAttributeDescription vertexInputAttrib;
        vertexInputAttrib.binding = 0;
        vertexInputAttrib.location = entry.index;
        vertexInputAttrib.format = GetVertexInputEntryFormat( entry );
        vertexInputAttrib.offset = entry.offset;

        vertexArrayObject->vertexLayout.push_back( vertexInputAttrib );
    }

    vertexArrayObject->nativeVertexInput.vertexAttributeDescriptionCount = vertexArrayObject->vertexLayout.size();
    vertexArrayObject->nativeVertexInput.pVertexAttributeDescriptions = vertexArrayObject->vertexLayout.data();
}
#endif
