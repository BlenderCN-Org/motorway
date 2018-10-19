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
#include "VirtualTexture.h"

#include "PageTable.h"

VirtualTexture::VirtualTexture()
{

}

VirtualTexture::~VirtualTexture()
{

}

void VirtualTexture::createFromStream( const VirtualTextureStream& stream )
{
    virtualTextureStream = std::move( stream );
}

void VirtualTexture::update( PageTable* pageTable )
{
    
}

void VirtualTexture::setPage( const int x, const int y, const int level, const uint64_t fileOffsetInBytes, const uint32_t pageSize )
{
    auto levelPage = virtualTextureStream.levels[level];
    const auto pageIndex = x + y * virtualTextureStream.pageCountX[level];

    levelPage[pageIndex].pageRelativeOffset = fileOffsetInBytes;
    levelPage[pageIndex].pageSizeInBytes = pageSize;
}

PageEntry& VirtualTexture::getPage( const fnPageId_t pageIndex )
{
    // Extract useful bytes to retrieve page entry
    const int x = ( ( pageIndex & 0x000000FF ) >> 0 );
    const int y = ( ( pageIndex & 0x0000FF00 ) >> 8 );
    const int level = ( ( pageIndex & 0x00FF0000 ) >> 16 );

    auto levelPage = virtualTextureStream.levels[level];
    const auto levelPageIndex = x + y * virtualTextureStream.pageCountX[level];

    return levelPage[levelPageIndex];
}

void VirtualTexture::loadPage( const fnPageId_t pageIndex, void* pageData )
{
    const auto& pageInfos = getPage( pageIndex );

    virtualTextureStream.stream->seek( pageInfos.pageRelativeOffset, flan::core::eFileReadDirection::FILE_READ_DIRECTION_BEGIN );
    virtualTextureStream.stream->read( ( uint8_t* )pageData, pageInfos.pageSizeInBytes );
}
