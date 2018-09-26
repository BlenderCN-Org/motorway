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
#include "QueryPool.h"

#include "RenderDevice.h"
#include "CommandList.h"

#if FLAN_GL460
#include "OpenGL460/RenderContext.h"
#include "OpenGL460/QueryPool.h"
#elif FLAN_D3D11
#include "Direct3D11/RenderContext.h"
#include "Direct3D11/CommandList.h"
#include "Direct3D11/QueryPool.h"
#elif FLAN_VULKAN
#include "Vulkan/RenderContext.h"
#include "Vulkan/CommandList.h"
#include "Vulkan/QueryPool.h"
#endif

QueryPool::QueryPool()
    : nativeQueryPool( nullptr )
    , poolCapacity( 0 )
    , poolType( eQueryType::QUERY_TYPE_UNKNOWN )
{

}

QueryPool::~QueryPool()
{
    poolCapacity = 0;
    poolType = eQueryType::QUERY_TYPE_UNKNOWN;
}

void QueryPool::create( RenderDevice* renderDevice, const eQueryType queryType, const unsigned int capacity )
{
    poolType = queryType;
    poolCapacity = capacity;

    nativeQueryPool.reset( flan::rendering::CreateQueryPoolImpl( renderDevice->getNativeRenderContext(), poolType, poolCapacity ) );
}

void QueryPool::destroy( RenderDevice* renderDevice )
{
    flan::rendering::DestroyQueryPoolImpl( renderDevice->getNativeRenderContext(), nativeQueryPool.get() );
}

uint32_t QueryPool::allocateQueryHandle( CommandList* cmdList )
{
    return flan::rendering::AllocateQueryFromPoolCmdImpl( cmdList->getNativeCommandList(), nativeQueryPool.get() );
}

void QueryPool::beginQuery( CommandList* cmdList, const uint32_t queryHandle )
{
    flan::rendering::BeginQueryCmdImpl( cmdList->getNativeCommandList(), nativeQueryPool.get(), queryHandle );
}

void QueryPool::endQuery( CommandList* cmdList, const uint32_t queryHandle )
{
    flan::rendering::EndQueryCmdImpl( cmdList->getNativeCommandList(), nativeQueryPool.get(), queryHandle );
}

void QueryPool::writeTimestamp( CommandList* cmdList, const uint32_t queryHandle )
{
    flan::rendering::WriteTimestampQueryCmdImpl( cmdList->getNativeCommandList(), nativeQueryPool.get(), queryHandle );
}

bool QueryPool::retrieveQueryResult( RenderDevice* renderDevice, const uint32_t queryHandle, uint64_t& queryResult )
{
    return flan::rendering::GetQueryResultSynchronousImpl( renderDevice->getNativeRenderContext(), nativeQueryPool.get(), queryHandle, queryResult );
}

double flan::rendering::ConvertTimestampToMs( const double timestamp )
{
    return flan::rendering::NativeTimestampUnitToMsImpl( timestamp );
}
