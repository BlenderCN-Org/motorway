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

#include "QueryPool.h"

#include <Maths/Helpers.h>

#include "RenderDevice.h"
#include "CommandList.h"

#include <vulkan/vulkan.h>

using namespace nya::rendering;

QueryPool* RenderDevice::createQueryPool( const eQueryType type, const unsigned int poolCapacity )
{
    QueryPool* queryPool = nya::core::allocate<QueryPool>( memoryAllocator );

    VkQueryPoolCreateInfo queryPoolCreateInfos = {};
    queryPoolCreateInfos.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolCreateInfos.pNext = nullptr;
    queryPoolCreateInfos.flags = 0;

    if ( type == eQueryType::QUERY_TYPE_TIMESTAMP ) {
        queryPoolCreateInfos.queryType = VkQueryType::VK_QUERY_TYPE_TIMESTAMP;
    }

    queryPoolCreateInfos.queryCount = poolCapacity;
    queryPoolCreateInfos.pipelineStatistics = 0;

    VkQueryPool nativeQueryPool = nullptr;
    vkCreateQueryPool( renderContext->device, &queryPoolCreateInfos, nullptr, &nativeQueryPool );

    queryPool->queryPool = nativeQueryPool;
    queryPool->currentAllocableIndex = 0;
    queryPool->capacity = poolCapacity;

    return queryPool;
}

bool RenderDevice::getQueryResult( QueryPool* queryPool, const unsigned int queryIndex, uint64_t& queryResult )
{
    VkResult operationResult = vkGetQueryPoolResults( renderContext->device, queryPool->queryPool, queryIndex, 1, sizeof( uint64_t ), &queryResult, sizeof( uint64_t ), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_PARTIAL_BIT );

    return ( operationResult == VK_SUCCESS );
}

double RenderDevice::convertTimestampToMs( const QueryPool* queryPool, const double timestamp )
{
    return timestamp / 1000000.0;
}

void RenderDevice::destroyQueryPool( QueryPool* queryPool )
{
    vkDestroyQueryPool( renderContext->device, queryPool->queryPool, nullptr );
}

unsigned int CommandList::allocateQuery( QueryPool* queryPool )
{
    queryPool->currentAllocableIndex = ++queryPool->currentAllocableIndex % queryPool->capacity;

    return queryPool->currentAllocableIndex;
}

void CommandList::beginQuery( QueryPool* queryPool, const unsigned int queryIndex )
{
    vkCmdBeginQuery( CommandListObject->cmdBuffer, queryPool->queryPool, queryIndex, 0 );
}

void CommandList::endQuery( QueryPool* queryPool, const unsigned int queryIndex )
{
    vkCmdEndQuery( CommandListObject->cmdBuffer, queryPool->queryPool, queryIndex );
}

void CommandList::writeTimestamp( QueryPool* queryPool, const unsigned int queryIndex )
{
    vkCmdWriteTimestamp( CommandListObject->cmdBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, queryPool->queryPool, queryIndex );
}
#endif
