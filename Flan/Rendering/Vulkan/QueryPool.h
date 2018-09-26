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
#pragma once

#if FLAN_VULKAN
#include <Rendering/QueryPool.h>

#include <vulkan/vulkan.hpp>
#include <vector>

struct NativeRenderContext;
struct NativeCommandList;

struct NativeQueryPool
{
    VkQueryPool  queryPool;
    unsigned int currentAllocableIndex;
    unsigned int capacity;
};

namespace flan
{
    namespace rendering
    {
        NativeQueryPool* CreateQueryPoolImpl( NativeRenderContext* nativeRenderContext, const eQueryType queryType, const unsigned int capacity );
        void DestroyQueryPoolImpl( NativeRenderContext* nativeRenderContext, NativeQueryPool* nativeQueryPool );
        bool GetQueryResultSynchronousImpl( NativeRenderContext* nativeRenderContext, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex, uint64_t& queryResult );
      
        uint32_t AllocateQueryFromPoolCmdImpl( NativeCommandList* nativeCmdList, NativeQueryPool* nativeQueryPool );

        void BeginQueryCmdImpl( NativeCommandList* nativeCmdList, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex );
        void EndQueryCmdImpl( NativeCommandList* nativeCmdList, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex );

        void WriteTimestampQueryCmdImpl( NativeCommandList* nativeCmdList, NativeQueryPool* nativeQueryPool, const uint32_t queryIndex );

        double NativeTimestampUnitToMsImpl( const double nativeTimestampResult );
    }
}
#endif
