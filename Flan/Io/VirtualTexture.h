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

#include <vector>
#include <Graphics/VirtualTexturing/PageIndex.h>

class FileSystemObject;

struct MipLevelHeader
{
    uint32_t width;
    uint32_t height;
    uint16_t pageCountX;
    uint16_t pageCountY;
};

struct PageEntry
{
    uint64_t pageRelativeOffset;
    uint32_t pageSizeInBytes;
};

struct VirtualTexture
{
    void setPage( const int x, const int y, const int level, const uint64_t fileOffsetInBytes, const uint32_t pageSize )
    {
        auto levelPage = levels[level];
        const auto pageIndex = x + y * pageCountX[level];

        levelPage[pageIndex].pageRelativeOffset = fileOffsetInBytes;
        levelPage[pageIndex].pageSizeInBytes = pageSize;
    }

    PageEntry& getPage( const fnPageId_t pageIndex )
    {
        // Extract useful bytes to retrieve page entry
        const int x = ( ( pageIndex & 0x000000FF ) >> 0 );
        const int y = ( ( pageIndex & 0x0000FF00 ) >> 8 );
        const int level = ( ( pageIndex & 0x00FF0000 ) >> 16 );

        auto levelPage = levels[level];
        const auto levelPageIndex = x + y * pageCountX[level];

        return levelPage[levelPageIndex];
    }

    void loadPage( const fnPageId_t pageIndex, void* pageData )
    {
        const auto& pageInfos = getPage( pageIndex );

        stream->seek( pageInfos.pageRelativeOffset, flan::core::eFileReadDirection::FILE_READ_DIRECTION_BEGIN );
        stream->read( (uint8_t*)pageData, pageInfos.pageSizeInBytes );
    }

    fnStringHash_t hashcode;
    uint32_t pageCountX[11];
    uint32_t pageCountY[11];
    std::vector<PageEntry*> levels;
    std::vector<PageEntry> pageEntries;
    FileSystemObject* stream;
};

namespace flan
{
    namespace core
    {
        void LoadVirtualTextureFile( FileSystemObject* stream, VirtualTexture& quadTree );
    }
}
