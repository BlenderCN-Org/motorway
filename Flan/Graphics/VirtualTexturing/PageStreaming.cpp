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
#include "PageStreaming.h"

#include <Rendering/Texture.h>

PageStreaming::PageStreaming()
{

}

PageStreaming::~PageStreaming()
{

}

void PageStreaming::destroy( RenderDevice* renderDevice )
{
    for ( auto& table : allocatedPageTables ) {
        table.destroy( renderDevice );
    }
}
/*
PageTable& PageStreaming::allocatePageTable( RenderDevice* renderDevice )
{   
    allocatedPageTables.push_back( {} );

    PageTable& pageTable = allocatedPageTables.back();
    pageTable.create( renderDevice );

    return pageTable;
}
*/
void PageStreaming::addPageRequest( const fnPageId_t pageIndex )
{

}

void PageStreaming::addAsynchronousPageLoading( const fnPageId_t pageIndex )
{

}
