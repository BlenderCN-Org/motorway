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

#if FLAN_GL460
#include "QueryPool.h"

static constexpr GLenum GL_QUERY_TYPE[eQueryType::QUERY_TYPE_COUNT] =
{
    0,
    GL_TIMESTAMP,
};

NativeQueryPool* flan::rendering::CreateQueryPoolImpl( NativeRenderContext* nativeRenderContext, const eQueryType queryType, const unsigned int capacity )
{
    NativeQueryPool* queryPool = new NativeQueryPool();
    queryPool->queryHandles = new GLuint[capacity]{ 0 };
    queryPool->targetType = GL_QUERY_TYPE[queryType];
    queryPool->capacity = capacity;

    glCreateQueries( queryPool->targetType, static_cast<GLsizei>( capacity ), queryPool->queryHandles );

    return queryPool;
}

void flan::rendering::DestroyQueryPoolImpl( NativeRenderContext* nativeRenderContext, NativeQueryPool* nativeQueryPool )
{
    glDeleteQueries( nativeQueryPool->capacity, nativeQueryPool->queryHandles );
}

bool flan::rendering::GetQueryResultSynchronousImpl( NativeRenderContext* nativeRenderContext, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex, uint64_t& queryResult )
{
    const auto queryHandle = nativeQueryPool->queryHandles[queryIndex];

    GLuint isAvailable = 0;
    glGetQueryObjectuiv( queryHandle, GL_QUERY_RESULT_AVAILABLE, &isAvailable );

    if ( isAvailable == 0 ) {
        return false;
    }

    glGetQueryObjectui64v( queryHandle, GL_QUERY_RESULT, &queryResult );

    return true;
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
    glBeginQuery( nativeQueryPool->targetType, nativeQueryPool->queryHandles[queryIndex] );
}

void flan::rendering::EndQueryCmdImpl( NativeCommandList* nativeCmdList, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex )
{
    glEndQuery( nativeQueryPool->targetType );
}

void flan::rendering::WriteTimestampQueryCmdImpl( NativeCommandList* nativeCmdList, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex )
{
    glQueryCounter( nativeQueryPool->queryHandles[queryIndex], GL_TIMESTAMP );
}

double flan::rendering::NativeTimestampUnitToMsImpl( const double nativeTimestampResult )
{
    // Nanoseconds to milliseconds
    return nativeTimestampResult / 1000000.0;
}
#endif
