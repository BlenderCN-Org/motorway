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

#include <FileSystem/FileSystemObject.h>
#include <Rendering/ImageFormat.h>

static constexpr uint32_t MAGIC = 0x00FF00FF;

void flan::core::LoadVirtualTextureFile( FileSystemObject* stream, VirtualTextureFileQuadTree& quadTree )
{
    const auto contentLength = stream->getSize();

    struct Header {
        uint32_t magic;
        uint32_t version;
        eImageFormat format;
        uint16_t mipCount;
        uint16_t borderSize;
        uint32_t dimensionBorderless;
        uint32_t dimension;
    };

    Header fileHeader;
    stream->read( fileHeader );
    if ( fileHeader.magic != MAGIC ) {
        FLAN_CERR << stream->getFilename() << " : invalid header magic!" << std::endl;
        return;
    }

    const auto dataBeginOffset = stream->tell();
    
    for ( uint32_t mipIdx = 0; mipIdx < fileHeader.mipCount; mipIdx++ ) {
        MipLevelHeader mipHeader;
        stream->read( mipHeader );
        stream->seek( mipHeader.pageCountY * mipHeader.pageCountX, eFileReadDirection::FILE_READ_DIRECTION_CURRENT );

        quadTree.pageCountX[mipIdx] = mipHeader.pageCountX;
        quadTree.pageCountY[mipIdx] = mipHeader.pageCountY;
    }
    
    uint32_t totalEntries = 0;
    for ( uint32_t l = 0; l < fileHeader.mipCount; ++l ) {
        totalEntries += quadTree.pageCountX[l] * quadTree.pageCountY[l];
    }

    quadTree.levels.resize( fileHeader.mipCount, nullptr );
    quadTree.pageEntries.resize( totalEntries );

    totalEntries = 0;
    for ( uint32_t l = 0; l < fileHeader.mipCount; ++l ) {
        quadTree.levels[l] = quadTree.pageEntries.data() + totalEntries;
        totalEntries += quadTree.pageCountX[l] * quadTree.pageCountY[l];
    }

    stream->seek( dataBeginOffset, eFileReadDirection::FILE_READ_DIRECTION_BEGIN );
    for ( uint32_t l = 0; l < fileHeader.mipCount; ++l ) {
        MipLevelHeader mipHeader;
        stream->read( mipHeader );

        for ( uint16_t y = 0; y < mipHeader.pageCountY; ++y ) {
            for ( uint16_t x = 0; x < mipHeader.pageCountX; ++x ) {
                PageEntry pageEntry;
                stream->read( pageEntry );

                quadTree.setPage( x, y, l, pageEntry.pageRelativeOffset, pageEntry.pageSizeInBytes );
            }
        }
    }
}
