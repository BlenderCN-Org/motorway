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

#if NYA_D3D11
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#include "QueryPool.h"

#include <Maths/Helpers.h>

#include "RenderDevice.h"
#include "CommandList.h"

#include <d3d11.h>

using namespace nya::rendering;

static constexpr D3D11_QUERY D3D11_QUERY_TYPE[eQueryType::QUERY_TYPE_COUNT] =
{
    D3D11_QUERY_EVENT,
    D3D11_QUERY_TIMESTAMP,
};

QueryPool* RenderDevice::createQueryPool( const eQueryType type, const unsigned int poolCapacity )
{
    QueryPool* queryPool = nya::core::allocate<QueryPool>( memoryAllocator );
    queryPool->targetType = D3D11_QUERY_TYPE[type];
    queryPool->queryHandles = nya::core::allocateArray<ID3D11Query*>( memoryAllocator, poolCapacity );

    queryPool->capacity = poolCapacity;
    queryPool->currentAllocableIndex = 0;

    // Create a pool of disjoint queries if the main pool is a timestamp one
    if ( queryPool->targetType == D3D11_QUERY_TIMESTAMP ) {
        queryPool->currentDisjointAllocableIndex = 0;
        queryPool->isRecordingDisjointQuery = false;
        queryPool->disjointQueryHandles = nya::core::allocateArray<ID3D11Query*>( memoryAllocator, poolCapacity );
        queryPool->disjointQueriesResult.resize( poolCapacity );
        queryPool->queryDisjointTable.resize( poolCapacity );

        std::fill_n( queryPool->disjointQueriesResult.begin(), poolCapacity, 0ull );

        D3D11_QUERY_DESC disjointQueryDesc;
        disjointQueryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        disjointQueryDesc.MiscFlags = 0;

        for ( unsigned int queryIdx = 0; queryIdx < poolCapacity; queryIdx++ ) {
            renderContext->nativeDevice->CreateQuery( &disjointQueryDesc, &queryPool->disjointQueryHandles[queryIdx] );
        }
    }

    D3D11_QUERY_DESC queryDesc;
    queryDesc.Query = queryPool->targetType;
    queryDesc.MiscFlags = 0;

    for ( unsigned int queryIdx = 0; queryIdx < poolCapacity; queryIdx++ ) {
        renderContext->nativeDevice->CreateQuery( &queryDesc, &queryPool->queryHandles[queryIdx] );
    }

    return queryPool;
}

bool RenderDevice::getQueryResult( QueryPool* queryPool, const unsigned int queryIndex, uint64_t& queryResult )
{
    auto deviceContext = renderContext->nativeDeviceContext;
    const auto queryHandle = queryPool->queryHandles[queryIndex];

    auto dataRetrieveResult = deviceContext->GetData( queryHandle, &queryResult, sizeof( uint64_t ), 0 );

    if ( queryPool->targetType == D3D11_QUERY_TIMESTAMP ) {
        auto disjointQueryIndex = queryPool->queryDisjointTable[queryIndex];

        if ( queryPool->disjointQueriesResult[disjointQueryIndex] == 0 ) {
            const auto disjointQueryHandle = queryPool->disjointQueryHandles[queryPool->queryDisjointTable[queryIndex]];

            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointQueryResult = {};
            auto dataDisjointRetrieveResult = deviceContext->GetData( disjointQueryHandle, &disjointQueryResult, sizeof( D3D11_QUERY_DATA_TIMESTAMP_DISJOINT ), 0 );
            if ( FAILED( dataDisjointRetrieveResult ) || disjointQueryResult.Disjoint ) {
                return false;
            }

            queryPool->deviceClockFrequency = nya::maths::max( queryPool->deviceClockFrequency, disjointQueryResult.Frequency );
        }
    }

    return SUCCEEDED( dataRetrieveResult );
}

double RenderDevice::convertTimestampToMs( const QueryPool* queryPool, const double timestamp )
{
    return timestamp / queryPool->deviceClockFrequency * 1000.0;
}

void RenderDevice::destroyQueryPool( QueryPool* queryPool )
{
    for ( unsigned int queryIdx = 0; queryIdx < queryPool->capacity; queryIdx++ ) {
        queryPool->queryHandles[queryIdx]->Release();
    }
    nya::core::free( memoryAllocator, queryPool->queryHandles );

    if ( queryPool->targetType == D3D11_QUERY_TIMESTAMP ) {
        for ( unsigned int queryIdx = 0; queryIdx < queryPool->capacity; queryIdx++ ) {
            queryPool->disjointQueryHandles[queryIdx]->Release();
        }

        nya::core::free( memoryAllocator, queryPool->disjointQueryHandles );
    }
}

unsigned int CommandList::allocateQuery( QueryPool* queryPool )
{
    queryPool->currentAllocableIndex = ( ++queryPool->currentAllocableIndex % queryPool->capacity );

    return queryPool->currentAllocableIndex;
}

void CommandList::beginQuery( QueryPool* queryPool, const unsigned int queryIndex )
{
    NativeCommandList->deferredContext->Begin( queryPool->queryHandles[queryIndex] );
}

void CommandList::endQuery( QueryPool* queryPool, const unsigned int queryIndex )
{
    NativeCommandList->deferredContext->End( queryPool->queryHandles[queryIndex] );
}

void CommandList::writeTimestamp( QueryPool* queryPool, const unsigned int queryIndex )
{
    // Update disjoint queries
    if ( !queryPool->isRecordingDisjointQuery ) {
        NativeCommandList->deferredContext->Begin( queryPool->disjointQueryHandles[queryIndex] );
        queryPool->disjointQueriesResult[queryIndex] = 0;
        queryPool->currentDisjointAllocableIndex = queryIndex;
    } else {
        NativeCommandList->deferredContext->End( queryPool->disjointQueryHandles[queryPool->currentDisjointAllocableIndex] );
    }

    queryPool->queryDisjointTable[queryIndex] = queryPool->currentDisjointAllocableIndex;
    queryPool->isRecordingDisjointQuery = !queryPool->isRecordingDisjointQuery;

    // Write timestamp
    NativeCommandList->deferredContext->End( queryPool->queryHandles[queryIndex] );
}
#endif
