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
#include "QueryPool.h"

#include <vulkan/vulkan.hpp>

#include "CommandList.h"
#include "RenderContext.h"

NativeQueryPool* flan::rendering::CreateQueryPoolImpl( NativeRenderContext* nativeRenderContext, const eQueryType queryType, const unsigned int capacity )
{
    NativeQueryPool* queryPool = new NativeQueryPool();

    VkQueryPoolCreateInfo queryPoolCreateInfos = {};
    queryPoolCreateInfos.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolCreateInfos.pNext = nullptr;
    queryPoolCreateInfos.flags = 0;

    if ( queryType == eQueryType::QUERY_TYPE_TIMESTAMP ) {
        queryPoolCreateInfos.queryType = VkQueryType::VK_QUERY_TYPE_TIMESTAMP;
    }

    queryPoolCreateInfos.queryCount = capacity;
    queryPoolCreateInfos.pipelineStatistics = 0;

    VkQueryPool nativeQueryPool = nullptr;
    vkCreateQueryPool( nativeRenderContext->device, &queryPoolCreateInfos, nullptr, &nativeQueryPool );

    queryPool->queryPool = nativeQueryPool;
    queryPool->currentAllocableIndex = 0;
    queryPool->capacity = capacity;
    
    return queryPool;
}

void flan::rendering::DestroyQueryPoolImpl( NativeRenderContext* nativeRenderContext, NativeQueryPool* nativeQueryPool )
{
    vkDestroyQueryPool( nativeRenderContext->device, nativeQueryPool->queryPool, nullptr );
}

bool flan::rendering::GetQueryResultSynchronousImpl( NativeRenderContext* nativeRenderContext, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex, uint64_t& queryResult )
{
    VkResult operationResult = vkGetQueryPoolResults( nativeRenderContext->device, nativeQueryPool->queryPool, queryIndex, 1, sizeof( uint64_t ), &queryResult, sizeof( uint64_t ), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_PARTIAL_BIT );
    
    return ( operationResult == VK_SUCCESS );
}

uint32_t flan::rendering::AllocateQueryFromPoolCmdImpl( NativeCommandList* nativeCmdList, NativeQueryPool* nativeQueryPool )
{
    if ( nativeQueryPool->currentAllocableIndex >= nativeQueryPool->capacity ) {
        nativeQueryPool->currentAllocableIndex = 0;
    }

    return nativeQueryPool->currentAllocableIndex++;
}

void flan::rendering::BeginQueryCmdImpl( NativeCommandList* nativeCmdList, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex )
{
    vkCmdBeginQuery( nativeCmdList->cmdBuffer, nativeQueryPool->queryPool, queryIndex, 0 );
}

void flan::rendering::EndQueryCmdImpl( NativeCommandList* nativeCmdList, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex )
{
    vkCmdEndQuery( nativeCmdList->cmdBuffer, nativeQueryPool->queryPool, queryIndex );
}

void flan::rendering::WriteTimestampQueryCmdImpl( NativeCommandList* nativeCmdList, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex )
{
    vkCmdWriteTimestamp( nativeCmdList->cmdBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, nativeQueryPool->queryPool, queryIndex );
}

double flan::rendering::NativeTimestampUnitToMsImpl( const double nativeTimestampResult )
{
    return nativeTimestampResult / 1000000.0;
}
#endif
