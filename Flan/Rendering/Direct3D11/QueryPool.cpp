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

#if FLAN_D3D11
#include "QueryPool.h"

#include "CommandList.h"
#include "RenderContext.h"

static constexpr D3D11_QUERY D3D11_QUERY_TYPE[eQueryType::QUERY_TYPE_COUNT] =
{
    D3D11_QUERY_EVENT,
    D3D11_QUERY_TIMESTAMP,
};

NativeQueryPool* flan::rendering::CreateQueryPoolImpl( NativeRenderContext* nativeRenderContext, const eQueryType queryType, const unsigned int capacity )
{
    NativeQueryPool* queryPool = new NativeQueryPool();
    queryPool->targetType = D3D11_QUERY_TYPE[queryType];
    queryPool->queryHandles = new ID3D11Query*[capacity]{ nullptr };
    
    queryPool->capacity = capacity;
    queryPool->currentAllocableIndex = 0;

    // Create a pool of disjoint queries if the main pool is a timestamp one
    if ( queryPool->targetType == D3D11_QUERY_TIMESTAMP ) {
        queryPool->currentDisjointAllocableIndex = 0;
        queryPool->isRecordingDisjointQuery = false;
        queryPool->disjointQueryHandles = new ID3D11Query*[capacity]{ nullptr };
        queryPool->disjointQueriesResult.resize( capacity );
        queryPool->queryDisjointTable.resize( capacity );

        std::fill_n( queryPool->disjointQueriesResult.begin(), capacity, 0ull );

        D3D11_QUERY_DESC disjointQueryDesc;
        disjointQueryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        disjointQueryDesc.MiscFlags = 0;

        for ( unsigned int queryIdx = 0; queryIdx < capacity; queryIdx++ ) {
            nativeRenderContext->nativeDevice->CreateQuery( &disjointQueryDesc, &queryPool->disjointQueryHandles[queryIdx] );
        }
    }

    D3D11_QUERY_DESC queryDesc;
    queryDesc.Query = queryPool->targetType;
    queryDesc.MiscFlags = 0;

    for ( unsigned int queryIdx = 0; queryIdx < capacity; queryIdx++ ) {
        nativeRenderContext->nativeDevice->CreateQuery( &queryDesc, &queryPool->queryHandles[queryIdx] );
    }

    return queryPool;
}

void flan::rendering::DestroyQueryPoolImpl( NativeRenderContext* nativeRenderContext, NativeQueryPool* nativeQueryPool )
{
    for ( unsigned int queryIdx = 0; queryIdx < nativeQueryPool->capacity; queryIdx++ ) {
        nativeQueryPool->queryHandles[queryIdx]->Release();
    }
    delete[] nativeQueryPool->queryHandles;

    if ( nativeQueryPool->targetType == D3D11_QUERY_TIMESTAMP ) {
        for ( unsigned int queryIdx = 0; queryIdx < nativeQueryPool->capacity; queryIdx++ ) {
            nativeQueryPool->disjointQueryHandles[queryIdx]->Release();
        }

        delete[] nativeQueryPool->disjointQueryHandles;
    }
}

// TODO This sucks; find a better way to store clock frequency to convert ticks to ms
static uint64_t DEVICE_CLOCK_FREQUENCY = 0;

bool flan::rendering::GetQueryResultSynchronousImpl( NativeRenderContext* nativeRenderContext, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex, uint64_t& queryResult )
{
    auto deviceContext = nativeRenderContext->nativeDeviceContext;
    const auto queryHandle = nativeQueryPool->queryHandles[queryIndex];

    auto dataRetrieveResult = deviceContext->GetData( queryHandle, &queryResult, sizeof( uint64_t ), 0 );
  
    if ( nativeQueryPool->targetType == D3D11_QUERY_TIMESTAMP ) {
        auto disjointQueryIndex = nativeQueryPool->queryDisjointTable[queryIndex];

        if ( nativeQueryPool->disjointQueriesResult[disjointQueryIndex] == 0 ) {
            const auto disjointQueryHandle = nativeQueryPool->disjointQueryHandles[nativeQueryPool->queryDisjointTable[queryIndex]];

            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointQueryResult = {};
            auto dataDisjointRetrieveResult = deviceContext->GetData( disjointQueryHandle, &disjointQueryResult, sizeof( D3D11_QUERY_DATA_TIMESTAMP_DISJOINT ), 0 );
            if ( FAILED( dataDisjointRetrieveResult ) || disjointQueryResult.Disjoint ) {
                return false;
            }

            DEVICE_CLOCK_FREQUENCY = std::max( DEVICE_CLOCK_FREQUENCY, disjointQueryResult.Frequency );
        }
    }

    return SUCCEEDED( dataRetrieveResult );
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
    nativeCmdList->deferredContext->Begin( nativeQueryPool->queryHandles[queryIndex] );
}

void flan::rendering::EndQueryCmdImpl( NativeCommandList* nativeCmdList, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex )
{
    nativeCmdList->deferredContext->End( nativeQueryPool->queryHandles[queryIndex] );
}

void flan::rendering::WriteTimestampQueryCmdImpl( NativeCommandList* nativeCmdList, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex )
{
    // Update disjoint queries
    if ( !nativeQueryPool->isRecordingDisjointQuery ) {
        nativeCmdList->deferredContext->Begin( nativeQueryPool->disjointQueryHandles[queryIndex] );
        nativeQueryPool->disjointQueriesResult[queryIndex] = 0;
        nativeQueryPool->currentDisjointAllocableIndex = queryIndex;
    } else {
        nativeCmdList->deferredContext->End( nativeQueryPool->disjointQueryHandles[nativeQueryPool->currentDisjointAllocableIndex] );
    }

    nativeQueryPool->queryDisjointTable[queryIndex] = nativeQueryPool->currentDisjointAllocableIndex;
    nativeQueryPool->isRecordingDisjointQuery = !nativeQueryPool->isRecordingDisjointQuery;

    // Write timestamp
    nativeCmdList->deferredContext->End( nativeQueryPool->queryHandles[queryIndex] );
}

double flan::rendering::NativeTimestampUnitToMsImpl( const double nativeTimestampResult )
{
    // Ticks to milliseconds
    return nativeTimestampResult / DEVICE_CLOCK_FREQUENCY * 1000.0;
}
#endif
