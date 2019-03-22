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

#if NYA_NULL_RENDERER
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

QueryPool* RenderDevice::createQueryPool( const eQueryType type, const unsigned int poolCapacity )
{
    return nullptr;
}

bool RenderDevice::getQueryResult( QueryPool* queryPool, const unsigned int queryIndex, uint64_t& queryResult )
{
    return true;
}

double RenderDevice::convertTimestampToMs( const QueryPool* queryPool, const double timestamp )
{
    return 0.0;
}

void RenderDevice::destroyQueryPool( QueryPool* queryPool )
{

}

unsigned int CommandList::allocateQuery( QueryPool* queryPool )
{
    return 0u;
}

void CommandList::beginQuery( QueryPool* queryPool, const unsigned int queryIndex )
{

}

void CommandList::endQuery( QueryPool* queryPool, const unsigned int queryIndex )
{

}

void CommandList::writeTimestamp( QueryPool* queryPool, const unsigned int queryIndex )
{

}
#endif
