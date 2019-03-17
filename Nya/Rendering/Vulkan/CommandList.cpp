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
#include <Rendering/CommandList.h>
#include "CommandList.h"

#include <vulkan/vulkan.h>

CommandList::~CommandList()
{

}

void CommandList::begin()
{
    VkCommandBufferBeginInfo cmdBufferInfos = {};
    cmdBufferInfos.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferInfos.pNext = nullptr;
    cmdBufferInfos.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBufferInfos.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer( NativeCommandList->cmdBuffer, &cmdBufferInfos );
}

void CommandList::end()
{
    vkEndCommandBuffer( NativeCommandList->cmdBuffer );
}

void CommandList::draw( const unsigned int vertexCount, const unsigned int vertexOffset )
{
    vkCmdDraw( NativeCommandList->cmdBuffer, vertexCount, 1, vertexOffset, 0 );
}

void CommandList::drawIndexed( const unsigned int indiceCount, const unsigned int indiceOffset, const size_t indiceType, const unsigned int vertexOffset )
{
    vkCmdDrawIndexed( NativeCommandList->cmdBuffer, indiceCount, 1, indiceOffset, vertexOffset, 0 );
}

void CommandList::drawInstancedIndexed( const unsigned int indiceCount, const unsigned int instanceCount, const unsigned int indiceOffset, const unsigned int vertexOffset, const unsigned int instanceOffset )
{
    vkCmdDrawIndexed( NativeCommandList->cmdBuffer, indiceCount, instanceCount, indiceOffset, vertexOffset, instanceOffset );
}

void CommandList::dispatchCompute( const unsigned int threadCountX, const unsigned int threadCountY, const unsigned int threadCountZ )
{
    vkCmdDispatch( NativeCommandList->cmdBuffer, threadCountX, threadCountY, threadCountZ );
}

void CommandList::setViewport( const Viewport& viewport )
{
    NativeCommandList->currentViewport = viewport;

    VkViewport vkViewport;
    vkViewport.x = static_cast<float>( viewport.X );
    vkViewport.y = static_cast<float>( viewport.Y );
    vkViewport.width = static_cast<float>( viewport.Width );
    vkViewport.height = static_cast<float>( viewport.Height );
    vkViewport.minDepth = viewport.MinDepth;
    vkViewport.maxDepth = viewport.MaxDepth;

    vkCmdSetViewport( NativeCommandList->cmdBuffer, 0, 1, &vkViewport );
}

void CommandList::getViewport( Viewport& viewport )
{
    viewport = NativeCommandList->currentViewport;
}
#endif
