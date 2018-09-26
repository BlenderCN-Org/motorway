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

struct NativeQueryPool;
class RenderDevice;
class CommandList;

enum eQueryType
{
    QUERY_TYPE_UNKNOWN = 0,
    QUERY_TYPE_TIMESTAMP,

    QUERY_TYPE_COUNT
};

class QueryPool
{
public:
                QueryPool();
                QueryPool( QueryPool& ) = default;
                QueryPool& operator = ( QueryPool& ) = default;
                ~QueryPool();

    void        create( RenderDevice* renderDevice, const eQueryType queryType, const unsigned int capacity  );
    void        destroy( RenderDevice* renderDevice );

    uint32_t    allocateQueryHandle( CommandList* cmdList );

    void        beginQuery( CommandList* cmdList, const uint32_t queryHandle );
    void        endQuery( CommandList* cmdList, const uint32_t queryHandle );

    void        writeTimestamp( CommandList* cmdList, const uint32_t queryHandle );

    bool        retrieveQueryResult( RenderDevice* renderDevice, const uint32_t queryHandle, uint64_t& queryResult );

private:
    std::unique_ptr<NativeQueryPool>    nativeQueryPool;
    unsigned int                        poolCapacity;
    eQueryType                          poolType;
};

namespace flan
{
    namespace rendering
    {
        // Convert timestamp from gfx backend unit to milliseconds
        double ConvertTimestampToMs( const double timestamp );
    }
}
